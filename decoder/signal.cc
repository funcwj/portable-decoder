// decoder/signal.cc
// wujian@2018


#include "signal.h"

Bool debug_mel = false;

std::string WindowToString(WindowType window) {
    switch (window) {
        case kBlackMan:
            return "blackman";
        case kHamm:
            return "hamming";
        case kHann:
            return "hanning";
        case kRect:
            return "rectangle";
        case kNone:
            return "none";
    }
}

WindowType StringToWindow(std::string window) {
    if (window == "blackman") return kBlackMan;
    else if (window == "hamming") return kHamm;
    else if (window == "hanning") return kHann;
    else if (window == "rectangle") return kRect;
    else if (window == "none") return kNone;
    else {
        LOG_FAIL << "Unknown type of window: " << window;
        return kNone;
    }
}

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
                    Int32 lower_bound, Int32 upper_bound, std::vector<std::vector<Float32> > *weights) {
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

void ComputeDctMatrix(Float32 *dct_matrix_, Int32 num_rows, Int32 num_cols) {
    ASSERT(num_rows && num_cols);

    Float32 normalizer = std::sqrt(1.0 / static_cast<Float32>(num_cols));

    for (Int32 j = 0; j < num_cols; j++) {
        dct_matrix_[j] = normalizer;
        // std::cerr << dct_matrix_[j] << (j == num_cols - 1 ? "\n" : " ");
    }
    normalizer = std::sqrt(2.0 / static_cast<Float32>(num_cols));

    for (Int32 k = 1; k < num_rows; k++)
        for (Int32 n = 0; n < num_cols; n++) {
            dct_matrix_[k * num_cols + n] = normalizer * 
                std::cos(static_cast<Float64>(PI) / num_cols * (n + 0.5) * k);
            // std::cerr << dct_matrix_[k * num_cols + n] << (n == num_cols - 1 ? "\n": " ");
        }
}

// template<class Computer>
Int32 ComputeFeature(Computer *computer, Float32 *signal, Int32 num_samps, Float32 *addr, Int32 stride) {
    ASSERT(computer->FeatureDim() <= stride);
    Int32 num_frames = computer->NumFrames(num_samps);
    for (Int32 t = 0; t < num_frames; t++)
        computer->ComputeFrame(signal, num_samps, t, addr + t * stride);
    return num_frames;
}


// template
// Int32 ComputeFeature(SpectrogramComputer *computer, Float32 *signal, Int32 num_samps, Float32 *addr, Int32 stride);// 

// template
// Int32 ComputeFeature(FbankComputer *computer, Float32 *signal, Int32 num_samps, Float32 *addr, Int32 stride);// 

// template
// Int32 ComputeFeature(MfccComputer *computer, Float32 *signal, Int32 num_samps, Float32 *addr, Int32 stride);

// Fix frame on time t
void FrameSplitter::FixFrame(Float32 *signal, Int32 t, Float32 *frame_addr) {
    Int32 frame_length = frame_opts_.frame_length;
    Int32 num_bytes = frame_length * sizeof(Float32);
    Int32 offset_in_signal = t * frame_opts_.frame_shift - prev_discard_size_;
    // if zero or positive
    if (offset_in_signal >= 0)
        memcpy(frame_addr, signal + offset_in_signal, num_bytes);
    else {
        ASSERT(frame_length + offset_in_signal);
        // pad to online_use_[prev_discard_size_: ]
        memcpy(online_use_ + prev_discard_size_, signal, 
            (frame_length + offset_in_signal) * sizeof(Float32));
        memcpy(frame_addr, online_use_ + prev_discard_size_ + offset_in_signal, num_bytes);
    }
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
    FixFrame(signal, index, frame_addr);
    // If last frame, cache discarded samples
    if (index == num_frames - 1) {
        // update prev_discard_size_
        prev_discard_size_ = num_samps + prev_discard_size_ - num_frames * frame_opts_.frame_shift;
        LOG_INFO << "Cache " << prev_discard_size_ << " discarded samples";
        if (frame_opts_.frame_length > frame_opts_.frame_shift) {
            ASSERT(prev_discard_size_);
            memcpy(online_use_, signal + num_samps - prev_discard_size_, 
                    sizeof(Float32) * prev_discard_size_);
        }
    }
    if (frame_opts_.remove_dc) {
        Float32 dc = 0;
        for (Int32 n = 0; n < frame_opts_.frame_length; n++)
            dc += frame_addr[n];
        // If use openblas
        // energy = VdotV(frame_addr, frame_addr, frame_length_);
        dc /= frame_opts_.frame_length;
        // Remove DC
        for (Int32 n = 0; n < frame_opts_.frame_length; n++)
            frame_addr[n] -= dc;
    }
    // Compute raw energy after removing DC
    if (raw_energy) {
        Float32 energy = 0.0;
        for (Int32 n = 0; n < frame_opts_.frame_length; n++)
            energy += frame_addr[n] * frame_addr[n];
        *raw_energy = energy;
    }
    // Preemphasize
    if (frame_opts_.preemph_coeff != 0.0)
        Preemphasize(frame_addr, frame_opts_.frame_length, frame_opts_.preemph_coeff);
    // Windowing
    if (frame_opts_.window_type != kNone)
        for (Int32 n = 0; n < frame_opts_.frame_length; n++)
            frame_addr[n] = frame_addr[n] * window_[n];
}

Float32 SpectrogramComputer::ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *spectrum_addr) {
    Int32 num_frames = splitter.NumFrames(num_samps);
    Float32 raw_energy;
    ASSERT(t < num_frames);
    // SetZero for padding windows
    memset(realfft_cache_, 0, sizeof(Float32) * padding_length_);
    memset(spectrum_addr, 0, sizeof(Float32) * FeatureDim());
    // Load current frame into cache
    splitter.FrameForIndex(signal, num_samps, t, realfft_cache_, &raw_energy);
    // Run RealFFT
    fft_computer->RealFFT(realfft_cache_, padding_length_);
    // Compute (Log)(Power/Magnitude) spectrum
    ComputeSpectrum(realfft_cache_, padding_length_, spectrum_addr, apply_pow_, apply_log_);
    // Using log-energy or not
    if (use_log_raw_energy_)
        spectrum_addr[0] = LogFloat32(raw_energy);
    return raw_energy;
}

Float32 FbankComputer::ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *fbank_addr) {
    Int32 num_fft_bins = spectrogram_computer_.FeatureDim();
    ASSERT(t < spectrogram_computer_.NumFrames(num_samps));
    memset(fbank_addr, 0, sizeof(Float32) * FeatureDim());
    // Compute linear-spectrogram, no energy
    Float32 raw_energy = spectrogram_computer_.ComputeFrame(signal, num_samps, t, spectrum_cache_);
    // Weight spectrogram with mel coefficients
    for (Int32 f = 0; f < num_bins_; f++) {
        ASSERT(mel_coeff_[f].size() == num_fft_bins);
        const Float32 *weights = mel_coeff_[f].data();
        Float32 mel_energy = 0;
        // if use openblas
        // Float32 mel_energy = VdotV(weights, spectrum_cache_, num_fft_bins);
        for (Int32 i = 0; i < num_fft_bins; i++)
            mel_energy += spectrum_cache_[i] * weights[i];
        // log mel-fbank or linear mel-fbank
        fbank_addr[f] = apply_log_ ? LogFloat32(mel_energy): mel_energy;
    }
    return raw_energy;
}


Float32 MfccComputer::ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *mfcc_addr) {
    ASSERT(t < fbank_computer.NumFrames(num_samps));
    // Zero mfcc_addr
    memset(mfcc_addr, 0, sizeof(Float32) * FeatureDim());
    Float32 raw_energy = fbank_computer.ComputeFrame(signal, num_samps, t, mel_energy_cache_);
    Int32 num_mel_bins = fbank_computer.FeatureDim();
    // mfcc = dct_matrix_ * mel_energy
    // dct_matrix_: only use first num_ceps rows
    for (Int32 i = 0; i < num_ceps_; i++) {
        for (Int32 j = 0; j < num_mel_bins; j++)
            mfcc_addr[i] += (dct_matrix_[i * num_mel_bins + j] * mel_energy_cache_[j]);
    }
    // Scale
    if (cepstral_lifter_ != 0.0) {
        for (Int32 i = 0; i < num_ceps_; i++)
            mfcc_addr[i] = mfcc_addr[i] * lifter_coeffs_[i];
    }
    if (use_energy_)
        mfcc_addr[0] = LogFloat32(raw_energy);
    return raw_energy;
}
