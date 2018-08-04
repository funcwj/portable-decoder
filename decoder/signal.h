// wujian@2018

#ifndef SIGNAL_H
#define SIGNAL_H

#include "decoder/common.h"
#include "decoder/fft-computer.h"

void Preemphasize(Float32 *frame, Int32 frame_length, Float32 preemph_coeff);

// Compute window values
void ComputeWindow(Int32 window_size, Float32 *window, WindowType window_type);

// Compute power/magnitude spectrum from Realfft results
void ComputeSpectrum(Float32 *realfft, Int32 dim, Float32 *spectrum, 
                        bool apply_pow, bool apply_log);

// Compute mel-filter coefficients
void ComputeMelFilters(Int32 num_fft_bins, Int32 num_mel_bins,
                       Float32 sample_freq, Float32 low_freq, Float32 high_freq,
                       std::vector<std::vector<Float32>> *weights);


struct FrameOpts {
    Int32 frame_length, frame_shift, sample_freq;
    WindowType window_type;
    Float32 preemph_coeff;

    FrameOpts(): preemph_coeff(0.97), frame_length(400), 
                frame_shift(160), window_type(kHamm) { }

    FrameOpts(Int32 length, Int32 shift, Int32 frequency, Float32 coeff, 
            WindowType window): preemph_coeff(coeff), frame_length(length), 
            frame_shift(shift), window_type(window) { }

    FrameOpts(const FrameOpts& opts): preemph_coeff(opts.preemph_coeff), frame_length(opts.frame_length), 
                frame_shift(opts.frame_shift), window_type(opts.window_type), 
                sample_freq(opts.sample_freq) { }
};


// Using for feature extraction
class FrameSplitter {
public:
    FrameSplitter(const FrameOpts &opts): 
            frame_length_(opts.frame_length),
            frame_shift_(opts.frame_shift), 
            preemph_coeff_(opts.preemph_coeff),
            window_type_(opts.window_type), 
            sample_freq_(opts.sample_freq) {
        window_ = new Float32[frame_length_];
        ComputeWindow(frame_length_, window_, window_type_);
    }

    // Framing whole signal at a time into assigned memory address
    Int32 Frame(Float32 *signal, Int32 num_samps, Float32 *frames, Int32 stride);

    // Copy Frame at time 'index' into assigned memory address
    void FrameForIndex(Float32 *signal, Int32 num_samps, Int32 index,
                       Float32 *frame, Float32 *raw_energy);

    // Compute number of frames given number of samples
    Int32 NumFrames(Int32 num_samps) {
        return static_cast<Int32>((num_samps - frame_length_) / frame_shift_) + 1;
    }

    Int32 FrameLength() { return frame_length_; }

    Int32 FrameShift() { return frame_shift_; }

    Int32 SampleFreq() { return sample_freq_; }

    Int32 PaddingLength() { return RoundUpToNearestPowerOfTwo(frame_length_); }

    ~FrameSplitter() {
        if (window_)
            delete[] window_;
    }

private:
    Int32 frame_length_, frame_shift_, sample_freq_;
    Float32 preemph_coeff_;
    Float32 *window_;
    WindowType window_type_;
};

struct SpectrogramOpts {
    bool apply_log, apply_pow, use_log_raw_energy;
    FrameOpts frame_opts;

    SpectrogramOpts(): apply_log(true), apply_pow(true), 
        use_log_raw_energy(true) { }

    SpectrogramOpts(const FrameOpts& opts): apply_log(true), apply_pow(true), 
        use_log_raw_energy(true), frame_opts(opts) { }

    SpectrogramOpts(bool power, bool log, bool use_energy): apply_log(log), 
            apply_pow(power), use_log_raw_energy(use_energy) { }
    
    SpectrogramOpts(const SpectrogramOpts &opts): apply_log(opts.apply_log), 
            apply_pow(opts.apply_pow), use_log_raw_energy(opts.use_log_raw_energy), 
            frame_opts(opts.frame_opts) { }
};


// SpectrogramComputer for ASR
class SpectrogramComputer {
public:
    SpectrogramComputer(const SpectrogramOpts &spectrogram_opts) : 
                        apply_pow_(spectrogram_opts.apply_pow), 
                        use_log_raw_energy_(spectrogram_opts.use_log_raw_energy),
                        apply_log_(spectrogram_opts.apply_log) {
        splitter = new FrameSplitter(spectrogram_opts.frame_opts);
        padding_length_ = splitter->PaddingLength();
        fft_computer = new FFTComputer(padding_length_);
        realfft_cache_ = new Float32[padding_length_];
    }

    // Compute spectrogram for signal chunk
    Int32 Compute(Float32 *signal, Int32 num_samps, Float32 *spectrogram, Int32 stride);

    // Compute spectrum for frame t
    void ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *spectrum_addr);

    ~SpectrogramComputer() {
        if (splitter)
            delete splitter;
        if (fft_computer)
            delete fft_computer;
        if (realfft_cache_)
            delete[] realfft_cache_;
    }

    Int32 PaddingLength() { return padding_length_; }

    Int32 FeatureDim() { return padding_length_ / 2 + 1; }

    Int32 NumFrames(Int32 num_samps) { return splitter->NumFrames(num_samps); }

protected:
    Int32 padding_length_;
    FrameSplitter *splitter;
    FFTComputer *fft_computer;
    Float32 *realfft_cache_;
    bool apply_pow_, apply_log_, use_log_raw_energy_;
};

struct FbankOpts {
    Int32 num_bins;
    Int32 low_freq, high_freq;
    bool apply_pow;
    SpectrogramOpts spectrogram_opts;

    FbankOpts(): num_bins(23), low_freq(20), high_freq(0), apply_pow(true) { }

    FbankOpts(const SpectrogramOpts &opts): num_bins(23), low_freq(20), 
        high_freq(0), apply_pow(true), spectrogram_opts(opts) { }

    FbankOpts(Int32 num_bins, Int32 low, Int32 high, bool power): num_bins(num_bins), 
        low_freq(low), high_freq(high), apply_pow(power) { }

};

class FbankComputer {
public:
    FbankComputer(const FbankOpts &fbank_opts) : 
                num_bins_(fbank_opts.num_bins), apply_pow_(fbank_opts.apply_pow), 
                low_freq_(fbank_opts.low_freq) {
        Int32 samp_freq = fbank_opts.spectrogram_opts.frame_opts.sample_freq;
        high_freq_ = fbank_opts.high_freq > 0.0 ? fbank_opts.high_freq : 
                    samp_freq / 2 + fbank_opts.high_freq;
        spectrogram_computer_ = new SpectrogramComputer(fbank_opts.spectrogram_opts);
        spectrum_cache_ = new Float32[spectrogram_computer_->FeatureDim()];
        ComputeMelFilters(spectrogram_computer_->FeatureDim(), num_bins_, samp_freq >> 1, 
                                    low_freq_, high_freq_, &mel_coeff_);
    }

    ~FbankComputer() {
        if (spectrogram_computer_)
            delete spectrogram_computer_;
        if (spectrum_cache_)
            delete[] spectrum_cache_;
    }

    Int32 Compute(Float32 *signal, Int32 num_samps, Float32 *fbank, Int32 stride);
    
    void ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *fbank_addr);

    Int32 FeatureDim() { return num_bins_; }

    Int32 NumFrames(Int32 num_samps) { return spectrogram_computer_->NumFrames(num_samps); }

protected:
    SpectrogramComputer *spectrogram_computer_;
    Float32 *spectrum_cache_;
    Int32 num_bins_, low_freq_, high_freq_;
    bool apply_pow_;
    std::vector<std::vector<Float32> > mel_coeff_;
};


#endif