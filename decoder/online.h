// decoder/online.h

// wujian@2018
// TODO: improve vad or use py-webrtcvad in python scripts
// py-webrtcvad: https://github.com/wiseman/py-webrtcvad

#ifndef ONLINE_H
#define ONLINE_H

#include <queue>

#include "decoder/common.h"
#include "decoder/config.h"
#include "decoder/signal.h"

enum VadStatus { kSilence, kActive };

enum FeatureType { 
    kSpectrogram,
    kFbank,
    kMfcc,
    kUnkown
};

FeatureType StringToFeatureType(const std::string &type);

// Simple FeatureExtractor(mfcc/spectrogram/fbank)
class FeatureExtractor {
public:
    FeatureExtractor(const std::string& conf, const std::string &type);

    Int32 Compute(Float32 *signal, Int32 num_samps, Float32 *addr, Int32 stride);

    void Reset() { computer_->Reset(); }

    Int32 FeatureDim() { return computer_->FeatureDim(); }

    Int32 NumFrames(Int32 num_samps) { return computer_->NumFrames(num_samps); }

    ~FeatureExtractor() {
        if (computer_ != NULL)
            delete computer_;
    }

private:
    FeatureType type_;
    Computer *computer_;
};

// EnergyVad: performed bad
/*
struct EnergyVadOpts {
    Float32 threshold_in_db;
    Int32 transition_context;
    Int32 frame_length, frame_shift;

    EnergyVadOpts(Float32 thres, Int32 context = 10, Int32 frame_length = 400, 
                  Int32 frame_shift = 160): threshold_in_db(thres), transition_context(context),
                                            frame_length(frame_length), 
                                            frame_shift(frame_shift) { }

    EnergyVadOpts(const EnergyVadOpts &opts): threshold_in_db(opts.threshold_in_db),
                                            transition_context(opts.transition_context),
                                            frame_length(opts.frame_length),
                                            frame_shift(opts.frame_shift) { }
    void Check() { 
        ASSERT(transition_context >= 1);
        ASSERT(frame_length > frame_shift);
        ASSERT(frame_length > 0 && frame_shift > 0);
    }

};

class EnergyVadWrapper {
public:
    EnergyVadWrapper(const EnergyVadOpts vad_opts): vad_opts_(vad_opts) { 
        // remove_dc=false
        FrameOpts frame_opts(vad_opts.frame_length, vad_opts.frame_shift, 8000, 0.0, kNone, false);
        frame_opts.Check();
        vad_opts_.Check();
        splitter_ = new FrameSplitter(frame_opts);
        frame_cache_ = new Float32[vad_opts.frame_length];
        // Initialize state
        inner_steps_ = 0; inner_status_ = kSilence;
    }
    
    ~EnergyVadWrapper() {
        if (splitter_)
            delete splitter_;
        if (frame_cache_)
            delete[] frame_cache_;
    }

    Int32 Context() { return vad_opts_.transition_context; }

    Bool Run(Float32 *signal, Int32 num_samps, std::vector<Bool> *vad_status);

private:
    // Run one step
    Bool Run(Float32 energy);
    // To compute raw energy
    FrameSplitter *splitter_;
    EnergyVadOpts vad_opts_;

    VadStatus inner_status_;
    Int32 inner_steps_;
    Float32 *frame_cache_;
};

FrameOpts frame_opts_for_v2(400, 160, 8000, 0.0, kNone, false);

class EnergyVadWrapperV2 {
public:
    EnergyVadWrapperV2(Int32 context, Float32 threshold): vad_context_(context), threshold_in_db_(threshold),
                                                        speech_frames_(0), splitter_(frame_opts_for_v2) {
        frame_ = new Float32[frame_opts_for_v2.frame_length];
    }

    ~EnergyVadWrapperV2() { 
        if (frame_)
            delete[] frame_;
    }

    Int32 Context() { return vad_context_ / 2; }

    void Run(Float32 *signal, Int32 num_samps, std::vector<Bool> *vad_status) {
        Int32 num_frames = splitter_.NumFrames(num_samps);
        vad_status->resize(num_frames);
        Float32 energy;
        for (Int32 t = 0; t < num_frames; t++) {
            splitter_.FrameForIndex(signal, num_samps, t, frame_, &energy);
            Bool status = Run(energy / 400);
            (*vad_status)[t] = status;
        }
    }

private:

    Bool Run(Float32 energy) {
        ASSERT(speech_frames_ <= vad_context_ && speech_frames_ >= 0);
        Float32 db = ToDB(energy);
        Bool is_speech = db > threshold_in_db_;
        Int32 context_frames = 0;
        if (context_status_.size() < vad_context_) {
            context_status_.push(is_speech);
            speech_frames_ += (is_speech ? 1: 0);
            context_frames = context_status_.size();
        } else {
            Bool pop_status = context_status_.front();
            context_status_.pop();
            // is speech
            if (pop_status && !is_speech)
                speech_frames_--;
            if (!pop_status && is_speech)
                speech_frames_++;
            context_status_.push(is_speech);
            context_frames = vad_context_;
        }
        Bool status = (static_cast<Float32>(speech_frames_) / context_frames >= 0.5);
        LOG_INFO << "Energy: " << db << ", status: " << status << " speech frames: " << speech_frames_;
        return status;
    }

    Int32 vad_context_;
    Float32 threshold_in_db_;
    // Number of speech frames in context window
    Int32 speech_frames_;
    FrameSplitter splitter_;
    Float32 *frame_;

    std::queue<Bool> context_status_;
};
*/

// Using vad, not well enough
/*
class OnlineExtractor {
public:
    OnlineExtractor(const std::string &conf, const std::string &type, 
                    EnergyVadOpts vad_opts): extractor_(conf, type), vad_(vad_opts) {
        speech_buffer_.reserve(buffer_size);
        status_buffer_.reserve(buffer_size);
    }

    Int32 NumFramesReady();

    Int32 Dim() { return extractor_.FeatureDim(); }

    void Receive(Float32 *wav, Int32 num_samps);

    void Compute(Float32 *feats, Int32 stride);
    
private:
    const Int32 buffer_size = 160 * 500;

    FeatureExtractor extractor_;
    EnergyVadWrapper vad_;
    
    std::vector<Float32> speech_buffer_;
    std::vector<Bool> status_buffer_;
};
*/

#endif