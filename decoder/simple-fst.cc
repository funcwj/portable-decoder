// wujian@2018

#include "io.h"
#include "simple-fst.h"


void ReadBinaryArc(std::istream &is, Arc *arc) {
    char num_bytes;
    ReadBinary(is, &num_bytes, 1);
    ASSERT(num_bytes == sizeof(*arc));
    ReadBinary(is, reinterpret_cast<char*>(arc), sizeof(Arc));
}

void WriteBinaryArc(std::ostream &os, Arc arc) {
    char num_bytes = sizeof(Arc);
    WriteBinary(os, reinterpret_cast<const char*>(&num_bytes), 1);
    WriteBinary(os, reinterpret_cast<const char*>(&arc), num_bytes);
}

SimpleFst::SimpleFst(const SimpleFst &fst) {
    this->SetStart(fst.Start());
    for (StateIterator siter(fst); !siter.Done(); siter.Next()) {
        const auto state_id = siter.Value();
        this->AddState();
        this->SetFinal(state_id, fst.Final(state_id));
        for (ArcIterator aiter(fst, state_id); !aiter.Done(); aiter.Next()) {
            const Arc &arc = aiter.Value();
            this->AddArc(state_id, arc);
        }
    }
    if (!this->Equal(fst))
        LOG_FAIL << "Check equality failed in copy constructor";
}

Bool SimpleFst::Equal(const SimpleFst &fst) {
    if (fst.Start() != this->Start() || fst.NumStates() != this->NumStates()) {
        LOG_WARN << "Check start/num_states failed: " << fst.Start() << "/" << fst.NumStates()
                 << " vs " << this->Start() << "/" << this->NumStates();
        return false;
    }
    for (StateIterator siter(fst); !siter.Done(); siter.Next()) {
        const auto state_id = siter.Value();
        if (fst.NumArcs(state_id) != this->NumArcs(state_id) ||
            fst.Final(state_id) != this->Final(state_id)) {
            LOG_WARN << "Check " << state_id << "-th state's " << " num_arcs/final failed: "
                     << fst.NumArcs(state_id) << "/" << fst.Final(state_id) << " vs "
                     << this->NumArcs(state_id) << "/" << this->Final(state_id);
            return false;
        }
        for (ArcIterator other_aiter(fst, state_id), this_aiter(*this, state_id); 
                !other_aiter.Done(); other_aiter.Next(), this_aiter.Next()) {
            const Arc &other_arc = other_aiter.Value(), &this_arc = this_aiter.Value();
            if (other_arc.ilabel != this_arc.ilabel || 
                other_arc.olabel != this_arc.olabel ||
                other_arc.weight != this_arc.weight ||
                other_arc.nextstate != this_arc.nextstate) {
                LOG_WARN << "Check " << state_id << "-th state's arc failed"
                         << other_arc.ToString() << " vs " << this_arc.ToString();
                return false;
            }
        }
    }
    return true;
}

void SimpleFst::Read(std::istream &is) {
    ReadBinaryBasicType(is, &start_);
    Int64 num_states = 0, num_arcs = 0;
    ReadBinaryBasicType(is, &num_states);
    ReadBinaryBasicType(is, &num_arcs);
    LOG_INFO << "Read decoder graph, contains " << num_states << " states and "
             << num_arcs << " arcs with start index " << start_;
    Int64 state_num_arcs, check_num_arcs = 0;
    Float32 state_final;
    for (Int32 state_id = 0; state_id < num_states; state_id++) {
        ReadBinaryBasicType(is, &state_final);
        ReadBinaryBasicType(is, &state_num_arcs);
        // State *state = this->GetState(state_id);
        // state->SetFinal(state_final);
        this->AddState();
        this->SetFinal(state_id, state_final);
        check_num_arcs += state_num_arcs;
        for (Int32 i = 0; i < state_num_arcs; i++) {
            Arc cur_arc;
            ReadBinaryArc(is, &cur_arc);
            // ReadBinary(is, reinterpret_cast<char*>(&cur_arc), sizeof(Arc));
            this->AddArc(state_id, cur_arc);
        }
    }
    if (check_num_arcs != num_arcs)
        LOG_FAIL << "Check number of arcs failed, " << check_num_arcs << " vs " << num_arcs;
}

// start->num_states->num_arcs
void SimpleFst::Write(std::ostream &os) {
    WriteBinaryBasicType(os, start_);
    WriteBinaryBasicType(os, NumStates());
    // count number of arcs
    Int64 num_arcs = 0;
    for (StateIterator siter(*this); !siter.Done(); siter.Next()) {
        StateId state = siter.Value();
        num_arcs += NumArcs(state);
    }
    WriteBinaryBasicType(os, num_arcs);
    Int64 check_num_states = 0;
    for (StateIterator siter(*this); !siter.Done(); siter.Next()) {
        check_num_states++;
        StateId state = siter.Value();
        // final->num_arcs
        WriteBinaryBasicType(os, Final(state));
        WriteBinaryBasicType(os, NumArcs(state));
        for (ArcIterator aiter(*this, state); 
            !aiter.Done(); aiter.Next()) {
            const Arc &arc = aiter.Value();
            WriteBinaryArc(os, arc);
            // WriteBinary(os, reinterpret_cast<const char*>(&arc), sizeof(Arc));
            num_arcs++;
        }
    }
    if (check_num_states != NumStates())
        LOG_FAIL << "Number of state in current FST do not equal with values return "
                 << "from NumStates(), " << check_num_states << " vs" << NumStates();
    LOG_INFO << "Write decoder graph, contains " << NumStates() << " states and "
             << num_arcs << " arcs with start index " << start_;
}

void ReadSimpleFst(const std::string &filename, SimpleFst *fst) {
    BinaryInput bi(filename);
    ASSERT(fst);
    fst->Read(bi.Stream());
}