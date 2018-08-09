// wujian@2018

// From Kaldi's faster-decoder.{h,cc}

#include "decoder/decoder.h"


void FasterDecoder::InitDecoding() {
    ClearToks(toks_.Clear());
    StateId start_state = fst_.Start();
    ASSERT(start_state != NoStateId);
    Arc dummy_arc(0, 0, 0, start_state);
    toks_.Insert(start_state, new Token(dummy_arc, NULL));
    ProcessNonemitting(std::numeric_limits<Float32>::max());
    num_frames_decoded_ = 0;
}


void FasterDecoder::DecodeFrame(Float32 *loglikes, Int32 num_pdfs) {
    if (num_pdfs != table_.NumPdfs()) {
        LOG_FAIL << "It seems that dimention of loglikes do not equal to number of pdfs, "
                 << num_pdfs << " vs " << table_.NumPdfs();
    }
    Float64 weight_cutoff = ProcessEmitting(loglikes, num_pdfs);
    ProcessNonemitting(weight_cutoff);
}

// Gets the weight cutoff.  Also counts the active tokens.
Float64 FasterDecoder::GetCutoff(Elem *list_head, UInt64 *tok_count,
                                Float32 *adaptive_beam, Elem **best_elem) {
    Float64 best_cost = FLOAT64_INF;
    UInt64 count = 0;
    if (max_active_ == std::numeric_limits<Int32>::max() && min_active_ == 0) {
        for (Elem *e = list_head; e != NULL; e = e->tail, count++) {
            Float64 w = e->val->cost_;
            if (w < best_cost) {
                best_cost = w;
                if (best_elem) *best_elem = e;
            }
        }
        if (tok_count != NULL) *tok_count = count;
        if (adaptive_beam != NULL) *adaptive_beam = beam_;
        return best_cost + beam_;
    } else {
        tmp_array_.clear();
        for (Elem *e = list_head; e != NULL; e = e->tail, count++) {
            Float64 w = e->val->cost_;
            tmp_array_.push_back(w);
            if (w < best_cost) {
                best_cost = w;
                if (best_elem) *best_elem = e;
            }
        }
        if (tok_count != NULL) 
            *tok_count = count;

        Float64 beam_cutoff = best_cost + beam_, min_active_cutoff = FLOAT64_INF,
            max_active_cutoff = FLOAT64_INF;

        if (tmp_array_.size() > static_cast<UInt64>(max_active_)) {
            std::nth_element(tmp_array_.begin(), tmp_array_.begin() + max_active_, tmp_array_.end());
            max_active_cutoff = tmp_array_[max_active_];
        }
        if (max_active_cutoff < beam_cutoff) { // max_active is tighter than beam.
            if (adaptive_beam)
                *adaptive_beam = max_active_cutoff - best_cost + 0.5;
            return max_active_cutoff;
        }
        if (tmp_array_.size() > static_cast<UInt64>(min_active_)) {
            if (min_active_ == 0) min_active_cutoff = best_cost;
            else {
                std::nth_element(tmp_array_.begin(), tmp_array_.begin() + min_active_,
                                tmp_array_.size() > static_cast<UInt64>(max_active_) ?
                                tmp_array_.begin() + max_active_: tmp_array_.end());
                min_active_cutoff = tmp_array_[min_active_];
            }
        }
        if (min_active_cutoff > beam_cutoff) { // min_active is looser than beam.
            if (adaptive_beam)
                *adaptive_beam = min_active_cutoff - best_cost + 0.5;
            return min_active_cutoff;
        } else {
            *adaptive_beam = beam_;
            return beam_cutoff;
        }
    }
}


Float64 FasterDecoder::ProcessEmitting(Float32 *loglikes, Int32 num_pdfs) {
    Elem *last_toks = toks_.Clear();
    UInt64 tok_cnt;
    Float32 adaptive_beam;
    Elem *best_elem = NULL;
    Float64 weight_cutoff = GetCutoff(last_toks, &tok_cnt, &adaptive_beam, &best_elem);
    LOG_INFO << tok_cnt << " tokens active.";
    // This makes sure the hash is always big enough.
    // PossiblyResizeHash(tok_cnt);

    UInt64 new_sz = static_cast<UInt64>(static_cast<Float32>(tok_cnt) * 2);
    if (new_sz > toks_.Size())
        toks_.SetSize(new_sz);

    // This is the cutoff we use after adding in the log-likes (i.e.
    // for the next frame).  This is a bound on the cutoff we will use
    // on the next frame.
    Float64 next_weight_cutoff = FLOAT64_INF;

    // First process the best token to get a hopefully
    // reasonably tight bound on the next cutoff.
    if (best_elem) {
        StateId state = best_elem->key;
        Token *tok = best_elem->val;
        for (ArcIterator aiter(fst_, state); !aiter.Done(); aiter.Next()) {
            const Arc &arc = aiter.Value();
            if (arc.ilabel != 0) {  // we'd propagate..
                Float32 ac_cost = -loglikes[table_.TransitionIdToPdf(arc.ilabel)];
                Float64 new_weight = arc.weight + tok->cost_ + ac_cost;
                if (new_weight + adaptive_beam < next_weight_cutoff)
                    next_weight_cutoff = new_weight + adaptive_beam;
            }
        }
    }

    // the tokens are now owned here, in last_toks, and the hash is empty.
    // 'owned' is a complex thing here; the point is we need to call TokenDelete
    // on each elem 'e' to let toks_ know we're done with them.
    for (Elem *e = last_toks, *e_tail; e != NULL; e = e_tail) {  // loop this way
        // because we delete "e" as we go.
        StateId state = e->key;
        Token *tok = e->val;
        if (tok->cost_ < weight_cutoff) {  // not pruned.
            // np++;
            ASSERT(state == tok->arc_.nextstate);
            for (ArcIterator aiter(fst_, state); !aiter.Done(); aiter.Next()) {
                Arc arc = aiter.Value();
                if (arc.ilabel != 0) {  // propagate..
                    // minimum
                    Float32 ac_cost = -loglikes[table_.TransitionIdToPdf(arc.ilabel)];
                    double new_weight = arc.weight + tok->cost_ + ac_cost;
                    if (new_weight < next_weight_cutoff) {  // not pruned..
                        Token *new_tok = new Token(arc, tok, ac_cost);
                        Elem *e_found = toks_.Find(arc.nextstate);
                        if (new_weight + adaptive_beam < next_weight_cutoff)
                            next_weight_cutoff = new_weight + adaptive_beam;
                        if (e_found == NULL) {
                            toks_.Insert(arc.nextstate, new_tok);
                        } else {
                            if ( e_found->val->cost_ > new_tok->cost_ ) {
                                Token::TokenDelete(e_found->val);
                                e_found->val = new_tok;
                            } else {
                                Token::TokenDelete(new_tok);
                            }
                        }
                    }
                }
            }
        }
        e_tail = e->tail;
        Token::TokenDelete(e->val);
        toks_.Delete(e);
    }
    num_frames_decoded_++;
    return next_weight_cutoff;
}

void FasterDecoder::ProcessNonemitting(Float64 cutoff) {
    // Processes nonemitting arcs for one frame. 
    ASSERT(queue_.empty());
    for (const Elem *e = toks_.GetList(); e != NULL;  e = e->tail)
        queue_.push_back(e->key);

    while (!queue_.empty()) {
        StateId state = queue_.back();
        queue_.pop_back();
        Token *tok = toks_.Find(state)->val;  // would segfault if state not
        // in toks_ but this can't happen.
        if (tok->cost_ > cutoff) // Don't bother processing successors.
            continue;

        ASSERT(tok != NULL && state == tok->arc_.nextstate);
        for (ArcIterator aiter(fst_, state); !aiter.Done(); aiter.Next()) {
            const Arc &arc = aiter.Value();
            if (arc.ilabel == 0) {  // propagate nonemitting only...
                Token *new_tok = new Token(arc, tok);
                if (new_tok->cost_ > cutoff) {  // prune
                    Token::TokenDelete(new_tok);
                } else {
                    Elem *e_found = toks_.Find(arc.nextstate);
                    if (e_found == NULL) {
                        toks_.Insert(arc.nextstate, new_tok);
                        queue_.push_back(arc.nextstate);
                    } else {
                        if ( e_found->val->cost_ > new_tok->cost_ ) {
                            Token::TokenDelete(e_found->val);
                            e_found->val = new_tok;
                            queue_.push_back(arc.nextstate);
                        } else {
                            Token::TokenDelete(new_tok);
                        }
                    }
                }
            }
        }
    }
}

Bool FasterDecoder::ReachedFinal() {
    for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail) {
        if (e->val->cost_ != FLOAT64_INF && fst_.Final(e->key) != 0)
        return true;
    }
    return false;
}

Bool FasterDecoder::GetBestPath(std::vector<Int32> *word_sequence) {
    word_sequence->clear();
    Token *best_tok = NULL;
    Bool is_final = ReachedFinal();
    if (!is_final) {
        for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
            if (best_tok == NULL || best_tok->cost_ > e->val->cost_ )
                best_tok = e->val;
    } else {
        Float64 best_cost = FLOAT64_INF;
        for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail) {
            Float64 this_cost = e->val->cost_ + fst_.Final(e->key);
            if (this_cost < best_cost && this_cost != FLOAT64_INF) {
                best_cost = this_cost;
                best_tok = e->val;
            }
        }
    }
    if (best_tok == NULL) 
        return false;  // No output.

    for (Token *tok = best_tok; tok != NULL; tok = tok->prev_)
        if (tok->arc_.olabel)
            word_sequence->push_back(tok->arc_.olabel);
    std::reverse(word_sequence->begin(), word_sequence->end());
    return true;
}

void FasterDecoder::ClearToks(Elem *list) {
    for (Elem *e = list, *e_tail; e != NULL; e = e_tail) {
        Token::TokenDelete(e->val);
        e_tail = e->tail;
        toks_.Delete(e);
    }
}