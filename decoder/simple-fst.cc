// wujian@2018

#include "io.h"
#include "simple-fst.h"


void ReadBinaryArc(std::istream &is, Arc *arc) {
    ReadBinaryBasicType(is, &(arc->ilabel));
    ReadBinaryBasicType(is, &(arc->olabel));
    ReadBinaryBasicType(is, &(arc->weight));
    ReadBinaryBasicType(is, &(arc->nextstate));
}

void WriteBinaryArc(std::ostream &os, Arc arc) {
    WriteBinaryBasicType(os, arc.ilabel);
    WriteBinaryBasicType(os, arc.olabel);
    WriteBinaryBasicType(os, arc.weight);
    WriteBinaryBasicType(os, arc.nextstate);
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
        this->AddState();
        ReadBinaryBasicType(is, &state_final);
        ReadBinaryBasicType(is, &state_num_arcs);
        State *state = this->GetState(state_id);
        state->SetFinal(state_final);
        // reserve memory
        this->ReserveArcs(state_id, num_arcs);
        for (Int32 i = 0; i < state_num_arcs; i++) {
            Arc cur_arc;
            ReadBinaryArc(is, &cur_arc);
            check_num_arcs++;
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
    Int64 num_arcs = 0, arc_shift = os.tellp();
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
            num_arcs++;
        }
    }
    Int32 end_shift = os.tellp();
    if (check_num_states != NumStates())
        LOG_FAIL << "Number of state in current FST do not equal with values return "
                 << "from NumStates(), " << check_num_states << " vs" << NumStates();
    os.seekp(arc_shift);
    WriteBinaryBasicType(os, num_arcs);
    os.seekp(end_shift);
    LOG_INFO << "Write decoder graph, contains " << NumStates() << " states and "
             << num_arcs << " arcs with start index " << start_;
}


