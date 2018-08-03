// wujian@2018


#include "signal.h"

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
void ComputeMelFilters(Int32 num_fft_bins, Int32 num_mel_bins, 
                        Float32 center_freq, Float32 low_freq, Float32 high_freq,
                        std::vector<std::vector<Float32> > *weights) {
    if (low_freq < 0 || low_freq >= center_freq || high_freq < 0 
        || high_freq > center_freq || high_freq <= low_freq)
        LOG_ERR << "Bad frequency range: [" << low_freq << ", " 
                << high_freq << "] with center frequency = "
                << center_freq;

    ASSERT(num_mel_bins >= 3);
    // egs: 257
    ASSERT(RoundUpToNearestPowerOfTwo(num_fft_bins - 1) == num_fft_bins);
    weights->resize(num_mel_bins);
    // Bound in melscale
    Float32 mel_high_freq = ToMelScale(high_freq), 
            mel_low_freq = ToMelScale(low_freq);
    // Linear/Mel band width
    Float32 linear_bw = center_freq / (num_fft_bins - 1), 
            mel_bw = (mel_high_freq - mel_low_freq) / (num_mel_bins + 1);
    
    for (Int32 bin = 0; bin < num_mel_bins; bin++) {
        // Fill with zero
        std::vector<Float32> &filter = (*weights)[bin];
        filter.resize(num_fft_bins, 0.0);

        Float32 left_mel = mel_low_freq + bin * mel_bw,
            center_mel = mel_low_freq + (bin + 1) * mel_bw,
            right_mel = mel_low_freq + (bin + 2) * mel_bw;
        // Compute coefficient for each bin
        for (Int32 f = 0; f < num_fft_bins; f++) {
            Float32 mel = ToMelScale(linear_bw * f);
            if (mel > left_mel && mel < right_mel) {
                filter[f] = mel <= center_mel ? 
                    (mel - left_mel) / (center_mel - left_mel):
                    (right_mel - mel) / (right_mel - center_mel);
            }
        }
    }
}

// Frame chunk of samples
Int32 FrameSplitter::Frame(Float32 *signal, Int32 num_samps, Float32 *frames, Int32 stride) {
    ASSERT(stride >= frame_length_);
    Int32 num_frames = NumFrames(num_samps);
    for (Int32 t = 0; t < num_frames; t++) {
        FrameForIndex(signal, num_samps, t, frames + stride * t, NULL);
    }
    return num_frames;
}

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
    Float32 dc = 0, energy = 0;
    for (Int32 n = 0; n < frame_length_; n++) {
        dc += frame_addr[n];
        energy += frame_addr[n] * frame_addr[n];
    }
    // using Blas
    // energy = VdotV(frame_addr, frame_addr, frame_length_);
    dc /= frame_length_;
    for (Int32 n = 0; n < frame_length_; n++)
        frame_addr[n] -= dc;
    if (preemph_coeff_ != 0)
        Preemphasize(frame_addr, frame_length_, preemph_coeff_);
    for (Int32 n = 0; n < frame_length_; n++)
        frame_addr[n] = frame_addr[n] * window_[n];
    if (raw_energy)
        *raw_energy = energy;
}

void SpectrogramComputer::ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *spectrum_addr) {
    Int32 num_frames = splitter->NumFrames(num_frames);
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

Int32 SpectrogramComputer::Compute(Float32 *signal, Int32 num_samps, 
                                   Float32 *spectrogram, Int32 stride) {
    ASSERT(FeatureDim() <= stride);
    Int32 num_frames = splitter->NumFrames(num_frames);
    for (Int32 t = 0; t < num_frames; t++) {
        ComputeFrame(signal, num_samps, t, spectrogram + t * stride);
    }
    return num_frames;
}


void FbankComputer::ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *fbank_addr) {
    Int32 num_fft_bins = spectrogram_computer_->FeatureDim();
    ASSERT(t < spectrogram_computer_->NumFrames(num_samps));
    // Compute linear-spectrogram, no energy
    spectrogram_computer_->ComputeFrame(signal, num_samps, t, spectrum_cache_);
    // Weight spectrogram with mel coefficients
    for (Int32 f = 0; f < num_bins_; f++) {
        ASSERT(mel_coeff_[f].size() == num_fft_bins);
        Float32 *weights = mel_coeff_[f].data, mel_energy;
        // Blas
        // Float32 mel_energy = VdotV(weights, spectrum_cache_, num_fft_bins);
        for (Int32 i = 0; i < num_fft_bins; i++)
            mel_energy += weights[i] * spectrum_cache_[i];
        fbank_addr[f] = LogFloat32(mel_energy);
    }
}


Int32 FbankComputer::Compute(Float32 *signal, Int32 num_samps, Float32 *fbank, Int32 stride) {
    ASSERT(FeatureDim() <= stride);
    Int32 num_frames = spectrogram_computer_->NumFrames(num_frames);
    for (Int32 t = 0; t < num_frames; t++) {
        ComputeFrame(signal, num_frames, t, fbank + t * stride);
    }
    return num_frames;
}


