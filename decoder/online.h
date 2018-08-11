// decoder/online.h

// wujian@2018

#ifndef ONLINE_H
#define ONLINE_H

#include "decoder/common.h"
#include "decoder/signal.h"

enum VadStatus { kSilence, kActive };

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
        // Initialize state
        inner_steps_ = 0; inner_status_ = kSilence; active_trigger_ = false;
        // remove_dc=false
        FrameOpts frame_opts(vad_opts.frame_length, vad_opts.frame_shift, 8000, 0.0, kNone, false);
        vad_opts_.Check(); frame_opts.Check();
        splitter_ = new FrameSplitter(frame_opts);
        frame_cache_ = new Float32[vad_opts.frame_length];
    }
    
    ~EnergyVadWrapper() {
        if (splitter_)
            delete splitter_;
        if (frame_cache_)
            delete[] frame_cache_;
    }

    // Run one step
    Int32 Run(Float32 energy);

    void Run(Float32 *signal, Int32 num_samps, Int32 *vad_status, Int32 num_frames);

private:
    FrameSplitter *splitter_;
    EnergyVadOpts vad_opts_;
    VadStatus inner_status_;
    Int32 inner_steps_;
    Bool active_trigger_;
    Float32 *frame_cache_;
};



#endif