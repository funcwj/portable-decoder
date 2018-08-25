// wujian@2018

// From Kaldi's faster-decoder.{h,cc}


#include "decoder/common.h"
#include "decoder/transition-table.h"
#include "decoder/hash-list.h"
#include "decoder/simple-fst.h"
#include "decoder/config.h"

struct DecodeOpts {
    Int32 min_active, max_active;
    Float32 beam, acwt, penalty;

    DecodeOpts(Int32 min_active = 200, Int32 max_active = 7000, Float32 beam = 15.0, Float32 acwt = 0.1, Float32 penalty = 0.0):
            min_active(min_active), max_active(max_active), beam(beam),
            acwt(acwt), penalty(penalty) { }
    
    DecodeOpts(const std::string &conf) {
        ConfigureParser parser(conf);
        ParseConfigure(&parser);
    }
    
    void ParseConfigure(ConfigureParser *parser) {
        parser->AddOptions("DecodeOpts", "max_active", &max_active);
        parser->AddOptions("DecodeOpts", "min_active", &min_active);
        parser->AddOptions("DecodeOpts", "beam", &beam);
        parser->AddOptions("DecodeOpts", "acwt", &acwt);
        parser->AddOptions("DecodeOpts", "penalty", &penalty);
    }

    std::string Configure() {
        std::ostringstream oss;
        oss << "--DecodeOpts.min_active=" << min_active << std::endl;  
        oss << "--DecodeOpts.max_active=" << max_active << std::endl;  
        oss << "--DecodeOpts.beam=" << beam << std::endl;  
        oss << "--DecodeOpts.acwt=" << acwt << std::endl;  
        oss << "--DecodeOpts.penalty=" << penalty << std::endl; 
        return oss.str(); 
    }


};

class FasterDecoder {
public:
    FasterDecoder(const SimpleFst &fst, const TransitionTable &table, 
                    Int32 min_active = 200, Int32 max_active = 7000, 
                    Float32 beam = 15.0, Float32 acwt = 0.1, Float32 penalty = 0.0):
                    fst_(fst), table_(table), min_active_(min_active), 
                    max_active_(max_active), beam_(beam), 
                    acoustic_scale_(acwt),  word_penalty_(penalty) {
        toks_.SetSize(1000);
        Check();
        reset_ = false;
    }

    FasterDecoder(const SimpleFst &fst, const TransitionTable &table, const DecodeOpts &opts): 
                    fst_(fst), table_(table), min_active_(opts.min_active), 
                    max_active_(opts.max_active), beam_(opts.beam), 
                    acoustic_scale_(opts.acwt), word_penalty_(opts.penalty) {
        toks_.SetSize(1000);
        Check();
        reset_ = false;
    }

    FasterDecoder(const std::string &str_fst, const std::string &str_table, const std::string &conf):
        fst_(str_fst), table_(str_table) {
        DecodeOpts opts(conf);
        min_active_ = opts.min_active, max_active_ = opts.max_active;
        beam_ = opts.beam, acoustic_scale_ = opts.acwt, word_penalty_ = opts.penalty;
        toks_.SetSize(1000);
        Check();
        reset_ = false;
    }

    ~FasterDecoder() { ClearToks(toks_.Clear()); }

    void Reset();

    void DecodeFrame(Float32 *loglikes, Int32 num_pdfs);

    void Decode(Float32 *loglikes, Int32 num_frames, Int32 stride, Int32 num_pdfs);

    Int32 NumDecodedFrames() { return num_frames_decoded_; }

    Bool ReachedFinal();

    Bool GetBestPath(std::vector<Int32> *word_sequence);

private:

    void Check() {
        ASSERT(min_active_ < max_active_);
        ASSERT(min_active_ > 0 && max_active_ > 1);
        ASSERT(word_penalty_ >= 0 && word_penalty_ <= 1);
    }

    class Token {
    public:
        Arc arc_;
        Token *prev_;
        Int32 ref_count_;
        Float64 cost_;  // negative-log

        inline Token(const Arc &arc, Token *prev, Float32 ac_cost = 0.0):
            arc_(arc), prev_(prev), ref_count_(1) {
            if (prev) {
                prev->ref_count_++;
                cost_ = prev->cost_ + arc.weight + ac_cost;
            } else {
                cost_ = arc.weight + ac_cost;
            }
        }
    };

    typedef HashList<StateId, Token*>::Elem Elem;

    Float64 GetCutoff(Elem *list_head, UInt64 *tok_count, Float32 *adaptive_beam, Elem **best_elem);

    Float64 ProcessEmitting(Float32 *loglikes, Int32 num_pdfs);

    inline Float32 NegativeLoglikelihood(Float32 *loglikes, Label tid);

    void ProcessNonemitting(Float64 cutoff);

    HashList<StateId, Token*> toks_;

    void ClearToks(Elem *list);

    inline void FreeToken(Token *tok);

    std::vector<StateId> queue_;
    std::vector<Float32> cost_active_;

    SimpleFst fst_;
    TransitionTable table_;

    Int32 min_active_, max_active_;
    Float32 beam_;
    Float32 acoustic_scale_, word_penalty_; // acwt and word penalty

    Int32 num_frames_decoded_;
    Bool reset_;
};