// decoder/online.cc

// wujian@2018

#include "decoder/online.h"


FeatureType StringToFeatureType(const std::string &type) {
    if (type == "spectrogram")
        return kSpectrogram;
    else if (type == "fbank")
        return kFbank;
    else if (type == "mfcc")
        return kMfcc;
    else
        return kUnkown;
}

FeatureExtractor::FeatureExtractor(const std::string& conf, const std::string &type):
        computer_(NULL) { 
    type_ = StringToFeatureType(type);
    ConfigureParser parser(conf);
    // initialize
    SpectrogramOpts spectrogram_opts;
    FbankOpts fbank_opts;
    MfccOpts mfcc_opts;
    switch (type_) {
        case kSpectrogram:
            LOG_INFO << "Create FeatureExtractor(Spectrogram)";
            spectrogram_opts.ParseConfigure(&parser);
            computer_ = new SpectrogramComputer(spectrogram_opts);
            break;
        case kFbank:
            LOG_INFO << "Create FeatureExtractor(Fbank)";
            fbank_opts.ParseConfigure(&parser);
            computer_ = new FbankComputer(fbank_opts);
            break;
        case kMfcc:
            LOG_INFO << "Create FeatureExtractor(Mfcc)";
            mfcc_opts.ParseConfigure(&parser);
            computer_ = new MfccComputer(mfcc_opts);
            break;
        case kUnkown:
            LOG_FAIL << "Unknown feature type: " << type;
            break;
    }
}

Int32 FeatureExtractor::Compute(Float32 *signal, Int32 num_samps, Float32 *addr, Int32 stride) {
    Int32 num_frames = -1;
    switch (type_) {
        case kSpectrogram:
        case kFbank:
        case kMfcc:
            num_frames = ComputeFeature(computer_, signal, num_samps, addr, stride);
            break;
        case kUnkown:
            LOG_FAIL << "Init FeatureExtractor with unknown feature type, stop compute";
            break;
    }
    return num_frames;
}

/*
Bool EnergyVadWrapper::Run(Float32 energy) { 
    Float32 db = ToDB(energy);
    LOG_INFO << "Energy: " << db << " dB";
    Bool active = db > vad_opts_.threshold_in_db;
    ASSERT(inner_steps_ >= 0 && 
        "inner_steps_ < 0, bugs existed in function EnergyVadWrapper::Run(Float32 energy)");
    switch (inner_status_) {
        case kSilence:
            if (active) {
                inner_steps_++;
                if (inner_steps_ == 
                    vad_opts_.transition_context) {
                    inner_status_ = kActive;
                    LOG_INFO << "ACTIVE";
                }
            }
            break;
        case kActive:
            if (!active) {
                inner_steps_--;
                if (inner_steps_ == 0) {
                    inner_status_ = kSilence;
                    LOG_INFO << "SILENCE";
                }
            }
            break;
    }
    return (inner_status_ == kActive);
}


Bool EnergyVadWrapper::Run(Float32 *signal, Int32 num_samps, std::vector<Bool> *vad_status) {
    Int32 num_frames = splitter_->NumFrames(num_samps);
    vad_status->resize(num_frames);
    Float32 energy;
    for (Int32 t = 0; t < num_frames; t++) {
        splitter_->FrameForIndex(signal, num_samps, t, frame_cache_, &energy);
        Bool status = Run(energy / vad_opts_.frame_length);
        (*vad_status)[t] = status;
    }
    return inner_status_ == kActive;
}

void OnlineExtractor::Compute(Float32 *feats, Int32 stride) {
    Int32 frame_index = 0;
    for (Int32 t = 0; t < status_buffer_.size(); t++) {
        Int32 traceback = 0;
        // Compute feature for active frame
        if (status_buffer_[t]) { 
            // traceback if needed
            if (t >= 1 && !status_buffer_[t - 1])
                traceback = std::min(t, vad_.Context()) - 1;
            // Compute features
            for (Int32 i = t - traceback; i <= t; i++) {
                extractor_.Compute(speech_buffer_.data(), 
                    speech_buffer_.size(), feats + frame_index * stride, stride);
                frame_index++;
            }
        }
    }
    ASSERT(frame_index == NumFramesReady());
    // Clear buffer
    speech_buffer_.clear();
    status_buffer_.clear();
}

Int32 OnlineExtractor::NumFramesReady() { 
    Int32 check_num_frames = extractor_.NumFrames(speech_buffer_.size());
    ASSERT(status_buffer_.size() == check_num_frames);
    Int32 num_frames_ready = 0;
    for (Int32 t = 0; t < status_buffer_.size(); t++) {
        if (status_buffer_[t]) { 
            num_frames_ready++;
            if (t >= 1 && !status_buffer_[t - 1]) {
                num_frames_ready += (std::min(t, vad_.Context()) - 1);
            }
        }
    }
    LOG_INFO << "Active/Total Frames: " << num_frames_ready << "/" << check_num_frames;
    return num_frames_ready;
}

void OnlineExtractor::Receive(Float32 *wav, Int32 num_samps) {
    std::vector<Bool> vad_status;
    // status_buffer_ resize here
    vad_.Run(wav, num_samps, &vad_status);
    speech_buffer_.insert(speech_buffer_.end(), wav, wav + num_samps);
    status_buffer_.insert(status_buffer_.end(), vad_status.begin(), vad_status.end());
}
*/