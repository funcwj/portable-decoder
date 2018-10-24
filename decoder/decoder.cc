// wujian@2018

// From Kaldi's faster-decoder.{h,cc}

#include "decoder/decoder.h"

Bool debug_decoder = false;

void FasterDecoder::Reset() {
  num_frames_decoded_ = 0;
  if (reset_) return;
  ClearToks(toks_.Clear());
  StateId start_state = fst_.Start();
  ASSERT(start_state != NoStateId);
  Arc dummy_arc(0, 0, 0, start_state);
  toks_.Insert(start_state, new Token(dummy_arc, NULL));
  ProcessNonemitting(std::numeric_limits<Float64>::max());
  reset_ = true;
}

void FasterDecoder::DecodeFrame(Float32 *loglikes, Int32 num_pdfs) {
  if (num_pdfs != table_.NumPdfs()) {
    LOG_FAIL << "It seems that dimention of loglikes do not equal to number of "
                "pdfs, "
             << num_pdfs << " vs " << table_.NumPdfs();
  }
  if (!reset_) LOG_FAIL << "Need call Reset() first to initialize decoder";
  Float64 weight_cutoff = ProcessEmitting(loglikes, num_pdfs);
  ProcessNonemitting(weight_cutoff);
}

void FasterDecoder::Decode(Float32 *loglikes, Int32 num_frames, Int32 stride,
                           Int32 num_pdfs) {
  // check memory
  ASSERT(num_pdfs <= stride);
  for (Int32 t = 0; t < num_frames; t++) {
    // LOG_INFO << "Decode frame " << t;
    DecodeFrame(loglikes + t * stride, num_pdfs);
  }
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
    cost_active_.clear();
    for (Elem *e = list_head; e != NULL; e = e->tail, count++) {
      Float64 w = e->val->cost_;
      cost_active_.push_back(w);
      if (w < best_cost) {
        best_cost = w;
        if (best_elem) *best_elem = e;
      }
    }
    if (tok_count != NULL) *tok_count = count;

    Float64 beam_cutoff = best_cost + beam_, min_active_cutoff = FLOAT64_INF,
            max_active_cutoff = FLOAT64_INF;

    if (cost_active_.size() > static_cast<UInt64>(max_active_)) {
      std::nth_element(cost_active_.begin(), cost_active_.begin() + max_active_,
                       cost_active_.end());
      max_active_cutoff = cost_active_[max_active_];
    }
    if (max_active_cutoff < beam_cutoff) {  // max_active is tighter than beam.
      if (adaptive_beam) *adaptive_beam = max_active_cutoff - best_cost + 0.5;
      return max_active_cutoff;
    }
    if (cost_active_.size() > static_cast<UInt64>(min_active_)) {
      if (min_active_ == 0)
        min_active_cutoff = best_cost;
      else {
        std::nth_element(cost_active_.begin(),
                         cost_active_.begin() + min_active_,
                         cost_active_.size() > static_cast<UInt64>(max_active_)
                             ? cost_active_.begin() + max_active_
                             : cost_active_.end());
        min_active_cutoff = cost_active_[min_active_];
      }
    }
    if (min_active_cutoff > beam_cutoff) {  // min_active is looser than beam.
      if (adaptive_beam) *adaptive_beam = min_active_cutoff - best_cost + 0.5;
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
  Float64 weight_cutoff =
      GetCutoff(last_toks, &tok_cnt, &adaptive_beam, &best_elem);

  UInt64 new_sz = static_cast<UInt64>(static_cast<Float32>(tok_cnt) * 2);
  if (new_sz > toks_.Size()) toks_.SetSize(new_sz);

  Float64 next_weight_cutoff = FLOAT64_INF;

  if (best_elem) {
    StateId state = best_elem->key;
    Token *tok = best_elem->val;
    for (ArcIterator aiter(fst_, state); !aiter.Done(); aiter.Next()) {
      const Arc &arc = aiter.Value();
      if (arc.ilabel != 0) {
        // -loglikes[table_.TransitionIdToPdf(arc.ilabel)] * acoustic_scale_;
        Float32 ac_cost = NegativeLoglikelihood(loglikes, arc.ilabel);
        Float64 new_weight = arc.weight + tok->cost_ + ac_cost;
        if (new_weight + adaptive_beam < next_weight_cutoff)
          next_weight_cutoff = new_weight + adaptive_beam;
      }
    }
  }

  for (Elem *e = last_toks, *e_tail; e != NULL; e = e_tail) {
    StateId state = e->key;
    Token *tok = e->val;
    if (tok->cost_ < weight_cutoff) {
      ASSERT(state == tok->arc_.nextstate);
      for (ArcIterator aiter(fst_, state); !aiter.Done(); aiter.Next()) {
        const Arc &arc = aiter.Value();
        if (arc.ilabel != 0) {
          // minimum
          // -loglikes[table_.TransitionIdToPdf(arc.ilabel)] * acoustic_scale_;
          Float32 ac_cost = NegativeLoglikelihood(loglikes, arc.ilabel);
          Float64 new_weight = arc.weight + tok->cost_ + ac_cost;
          if (new_weight < next_weight_cutoff) {  // not pruned..
            Token *new_tok = new Token(arc, tok, ac_cost);
            Elem *e_found = toks_.Find(arc.nextstate);
            if (new_weight + adaptive_beam < next_weight_cutoff)
              next_weight_cutoff = new_weight + adaptive_beam;
            if (e_found == NULL) {
              toks_.Insert(arc.nextstate, new_tok);
              if (debug_decoder)
                std::cerr << "insert token(" << arc.nextstate << ", "
                          << new_tok->cost_ << "=" << arc.weight << "+"
                          << tok->cost_ << "+" << ac_cost << "[" << arc.ilabel
                          << "->" << table_.TransitionIdToPdf(arc.ilabel)
                          << "])\n";
            } else {
              if (e_found->val->cost_ > new_tok->cost_) {
                FreeToken(e_found->val);
                e_found->val = new_tok;
                if (debug_decoder)
                  std::cerr << "replace token(" << e_found->key << ", "
                            << e_found->val->cost_ << ") with "
                            << "token(" << e_found->key << ", "
                            << new_tok->cost_ << ")\n";
              } else {
                FreeToken(new_tok);
              }
            }
          }
        }
      }
    }
    e_tail = e->tail;
    FreeToken(e->val);
    toks_.Delete(e);
  }
  num_frames_decoded_++;

  if (debug_decoder)
    LOG_INFO << "NumFrames/ActiveTokens: " << num_frames_decoded_ << "/"
             << tok_cnt << "(" << weight_cutoff << "|" << next_weight_cutoff
             << ")";

  return next_weight_cutoff;
}

void FasterDecoder::ProcessNonemitting(Float64 cutoff) {
  // Processes nonemitting arcs for one frame.
  ASSERT(queue_.empty());
  for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
    queue_.push_back(e->key);

  Int32 num_iter = 0;
  while (!queue_.empty()) {
    StateId state = queue_.back();
    queue_.pop_back();
    Token *tok = toks_.Find(state)->val;
    if (tok->cost_ > cutoff) continue;

    ASSERT(tok != NULL && state == tok->arc_.nextstate);
    // std::cerr << "Go: pop state(" << state << "), push state(";
    for (ArcIterator aiter(fst_, state); !aiter.Done(); aiter.Next()) {
      const Arc &arc = aiter.Value();
      if (arc.ilabel == 0) {
        Token *new_tok = new Token(arc, tok);
        if (new_tok->cost_ > cutoff) {
          FreeToken(new_tok);
        } else {
          Elem *e_found = toks_.Find(arc.nextstate);
          if (e_found == NULL) {
            toks_.Insert(arc.nextstate, new_tok);
            queue_.push_back(arc.nextstate);
          } else {
            if (e_found->val->cost_ > new_tok->cost_) {
              FreeToken(e_found->val);
              e_found->val = new_tok;
              queue_.push_back(arc.nextstate);
              // std::cerr << arc.nextstate << " ";
            } else {
              FreeToken(new_tok);
            }
          }
        }
      }
    }
    // std::cerr << ")" << std::endl;
    num_iter++;
  }
  if (debug_decoder) LOG_INFO << "Go " << num_iter << " iterations";
}

inline Float32 FasterDecoder::NegativeLoglikelihood(Float32 *loglikes,
                                                    Label tid) {
  Int32 pdf_id = table_.TransitionIdToPdf(tid);
  return -loglikes[pdf_id] * acoustic_scale_ + word_penalty_;
}

Bool FasterDecoder::ReachedFinal() {
  for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail) {
    if (e->val->cost_ != FLOAT64_INF && fst_.Final(e->key) != 0) return true;
  }
  return false;
}

Bool FasterDecoder::GetBestPath(std::vector<Int32> *word_sequence) {
  // do not clear
  // word_sequence->clear();
  // std::vector<Int32>::iterator end_iter = word_sequence->end();
  Token *best_tok = NULL;
  reset_ = false;

  Bool is_final = ReachedFinal();
  if (!is_final) {
    for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail)
      if (best_tok == NULL || best_tok->cost_ > e->val->cost_)
        best_tok = e->val;
    if (debug_decoder)
      LOG_INFO << "Log-like per frame is "
               << -best_tok->cost_ / num_frames_decoded_ << " over "
               << num_frames_decoded_ << " frames[PARTIAL]";
  } else {
    Float64 best_cost = FLOAT64_INF;
    for (const Elem *e = toks_.GetList(); e != NULL; e = e->tail) {
      Float64 this_cost = e->val->cost_ + fst_.Final(e->key);
      if (this_cost < best_cost && this_cost != FLOAT64_INF) {
        best_cost = this_cost;
        best_tok = e->val;
      }
    }
    if (debug_decoder)
      LOG_INFO << "Log-like per frame is " << -best_cost / num_frames_decoded_
               << " over " << num_frames_decoded_ << " frames";
  }
  if (best_tok == NULL) return false;

  Int32 num_labels = 0;
  for (Token *tok = best_tok; tok != NULL; tok = tok->prev_)
    if (tok->arc_.olabel) {
      word_sequence->push_back(tok->arc_.olabel);
      num_labels++;
    }
  if (num_labels)
    std::reverse(word_sequence->end() - num_labels, word_sequence->end());
  return true;
}

void FasterDecoder::ClearToks(Elem *list) {
  for (Elem *e = list, *e_tail; e != NULL; e = e_tail) {
    FreeToken(e->val);
    e_tail = e->tail;
    // delete Elem
    toks_.Delete(e);
  }
}

void FasterDecoder::FreeToken(Token *tok) {
  // traceback
  while (--tok->ref_count_ == 0) {
    Token *prev = tok->prev_;
    delete tok;  // delete or other free function
    if (prev == NULL)
      return;
    else
      tok = prev;
  }
}