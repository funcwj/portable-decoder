// wujian@2018


#include "signal.h"

Bool debug_mel = false;

void ComputeWindow(Int32 window_size, Float32 *window, WindowType window_type) {
    Float32 a = PI2 / (window_size - 1);
    for (Int32 i = 0; i < window_size; i++) {
        switch (window_type) {
            case kBlackMan:
                window[i] = 0.42 - 0.5 * cos(a * i) + 0.08 * cos(2 * a * i);
                break;
            case kHamm:
                window[i] = 0.54 - 0.46 * cos(a * i);
                break;
            case kHann:
                window[i] = 0.50 - 0.50 * cos(a * i);
                break;
            case kRect:
                window[i] = 1.0;
                break;
            default:
                break;
        }
    }
}

void ComputeSpectrum(Float32 *realfft, Int32 dim, Float32 *spectrum, bool apply_pow, bool apply_log) {
    Int32 mid = dim / 2;
    for (Int32 d = 1; d < mid; d++) {
        spectrum[d] = REAL_PART(realfft, d) * REAL_PART(realfft, d) + \
            IMAG_PART(realfft, d) * IMAG_PART(realfft, d);
    }
    spectrum[0] = REAL_PART(realfft, 0) * REAL_PART(realfft, 0);
    spectrum[mid] = IMAG_PART(realfft, 0) * IMAG_PART(realfft, 0);

    for (Int32 d = 0; d <= mid; d++) {
        spectrum[d] = apply_pow ? spectrum[d]: sqrtf(spectrum[d]);
        if (apply_log)
            spectrum[d] = LogFloat32(spectrum[d]);
    }
}

void Preemphasize(Float32 *frame, Int32 frame_length, Float32 preemph_coeff) {
    ASSERT(preemph_coeff >= 0);
    if (preemph_coeff == 0)
        return;
    for (Int32 n = frame_length - 1; n > 0; n--)
        frame[n] -= preemph_coeff * frame[n - 1];
    frame[0] -= preemph_coeff * frame[0];
}

// Compute mel-filter coefficients
void ComputeMelFilters(Int32 num_fft_bins, Int32 num_mel_bins, Int32 center_freq, 
                        Int32 lower_bound, Int32 upper_bound,
                        std::vector<std::vector<Float32> > *weights) {
    if (lower_bound < 0 || lower_bound >= center_freq || upper_bound < 0 
        || upper_bound > center_freq || upper_bound <= lower_bound)
        LOG_FAIL << "Bad frequency range: [" << lower_bound << ", " 
                 << upper_bound << "] with center frequency = " << center_freq;

    ASSERT(num_mel_bins >= 3);
    // egs: 257
    ASSERT(RoundUpToNearestPowerOfTwo(num_fft_bins - 1) == num_fft_bins - 1);
    weights->resize(num_mel_bins);
    // Bound in melscale
    Float32 mel_upper_bound = ToMelScale(upper_bound), 
            mel_lower_bound = ToMelScale(lower_bound);
    // Linear/Mel band width
    Float32 linear_bw = center_freq / static_cast<Float32>(num_fft_bins - 1), 
            mel_band_width = (mel_upper_bound - mel_lower_bound) / (num_mel_bins + 1);
    
    for (Int32 bin = 0; bin < num_mel_bins; bin++) {
        Float32 center_mel = mel_lower_bound + (bin + 1) * mel_band_width;
        // Fill with zero
        std::vector<Float32> &filter = (*weights)[bin];
        filter.resize(num_fft_bins, 0.0);
        if (debug_mel) {
            LOG_INFO << center_mel - mel_band_width << "/" << center_mel << "/" 
                     << center_mel + mel_band_width;
            std::cerr << "For bin " << bin << ": ";
        }
        // Compute coefficient for each bin
        for (Int32 f = 0; f < num_fft_bins; f++) {
            Float32 mel = ToMelScale(linear_bw * f);
            if (mel > center_mel - mel_band_width && mel < center_mel + mel_band_width) {
                filter[f] = mel <= center_mel ? 
                    (mel - center_mel) / mel_band_width + 1:
                    (center_mel - mel) / mel_band_width + 1;
                if (debug_mel)
                    std::cerr << filter[f] << " ";
            }
        }
        if (debug_mel)
            std::cerr << std::endl;
    }
}

template<class Computer>
Int32 ComputeFeature(Computer &computer, Float32 *signal, Int32 num_samps, Float32 *addr, Int32 stride) {
    ASSERT(computer.FeatureDim() <= stride);
    Int32 num_frames = computer.NumFrames(num_samps);
    for (Int32 t = 0; t < num_frames; t++)
        computer.ComputeFrame(signal, num_samps, t, addr + t * stride);
    return num_frames;
}


template
Int32 ComputeFeature(SpectrogramComputer &computer, Float32 *signal, Int32 num_samps, 
                        Float32 *addr, Int32 stride);

template
Int32 ComputeFeature(FbankComputer &computer, Float32 *signal, Int32 num_samps, 
                        Float32 *addr, Int32 stride);

/*
    1) Remove DC
    2) Preemphasize
    3) Apply window
*/
void FrameSplitter::FrameForIndex(Float32 *signal, Int32 num_samps, Int32 index, 
                                  Float32 *frame_addr, Float32 *raw_energy) {
    Int32 num_frames = NumFrames(num_samps);
    ASSERT(num_frames > index);
    // Copy to dest addr
    memcpy(frame_addr, signal + index * frame_shift_, sizeof(Float32) * frame_length_);
    Float32 dc = 0;
    for (Int32 n = 0; n < frame_length_; n++)
        dc += frame_addr[n];

    if (raw_energy) {
        Float32 energy = 0.0;
        for (Int32 n = 0; n < frame_length_; n++)
            energy += frame_addr[n] * frame_addr[n];
        *raw_energy = energy;
    }
    // If use openblas
    // energy = VdotV(frame_addr, frame_addr, frame_length_);
    dc /= frame_length_;
    // Remove DC
    for (Int32 n = 0; n < frame_length_; n++)
        frame_addr[n] -= dc;
    // Preemphasize
    if (preemph_coeff_ != 0)
        Preemphasize(frame_addr, frame_length_, preemph_coeff_);
    // Windowing
    for (Int32 n = 0; n < frame_length_; n++)
        frame_addr[n] = frame_addr[n] * window_[n];
}

void SpectrogramComputer::ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *spectrum_addr) {
    Int32 num_frames = splitter->NumFrames(num_samps);
    Float32 raw_energy;
    ASSERT(t < num_frames);
    // SetZero for padding windows
    memset(realfft_cache_, 0, sizeof(Float32) * padding_length_);
    // Load current frame into cache
    splitter->FrameForIndex(signal, num_samps, t, realfft_cache_, &raw_energy);
    // Run RealFFT
    fft_computer->RealFFT(realfft_cache_, padding_length_);
    // Compute (Log)(Power/Magnitude) spectrum
    ComputeSpectrum(realfft_cache_, padding_length_, spectrum_addr, apply_pow_, apply_log_);
    // Using log-energy or not
    if (use_log_raw_energy_)
        spectrum_addr[0] = LogFloat32(raw_energy);
}

void FbankComputer::ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *fbank_addr) {
    Int32 num_fft_bins = spectrogram_computer_->FeatureDim();
    ASSERT(t < spectrogram_computer_->NumFrames(num_samps));
    // Compute linear-spectrogram, no energy
    spectrogram_computer_->ComputeFrame(signal, num_samps, t, spectrum_cache_);
    // Weight spectrogram with mel coefficients
    for (Int32 f = 0; f < num_bins_; f++) {
        ASSERT(mel_coeff_[f].size() == num_fft_bins);
        const Float32 *weights = mel_coeff_[f].data();
        Float32 mel_energy = 0;
        // if use openblas
        // Float32 mel_energy = VdotV(weights, spectrum_cache_, num_fft_bins);
        for (Int32 i = 0; i < num_fft_bins; i++)
            mel_energy += weights[i] * spectrum_cache_[i];
        // log mel-fbank or linear mel-fbank
        fbank_addr[f] = apply_log_ ? LogFloat32(mel_energy): mel_energy;
    }
}


