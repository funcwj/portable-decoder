// decoder/online.cc

// wujian@2018

#include "decoder/online.h"


Int32 EnergyVadWrapper::Run(Float32 energy) { 
    Float32 db = ToDB(energy);
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
                    active_trigger_ = true;
                }
            }
            break;
        case kActive:
            if (!active) {
                inner_steps_--;
                if (inner_steps_ == 0) {
                    inner_status_ = kSilence;
                }
            }
            break;
    }
    return (inner_status_ == kActive ? 1: 0);
}


void EnergyVadWrapper::Run(Float32 *signal, Int32 num_samps, Int32 *vad_status, Int32 num_frames) {
    Int32 check_num_frames = splitter_->NumFrames(num_samps);
    if (check_num_frames != num_frames) 
        LOG_FAIL << "Check frame configure in EnergyVadOpts, seems different from feature extractor";
    for (Int32 t = 0; t < num_frames; t++) {
        Float32 energy;
        splitter_->FrameForIndex(signal, num_samps, t, frame_cache_, &energy);
        Int32 status = Run(energy / vad_opts_.frame_length);
        vad_status[t] = status;
        // Traceback and reset vad flags
        if (active_trigger_) {
            active_trigger_ = false;
            for (Int32 i = t + 1 - vad_opts_.transition_context; 
                i <= t; i++) {
                if (i >= 0) vad_status[t] = true;
            }
        }
    }
}