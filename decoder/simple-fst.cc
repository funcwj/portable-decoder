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

// slow, need to optimize
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
            // ReadBinary(is, reinterpret_cast<char*>(&cur_arc), sizeof(Arc));
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
            // WriteBinary(os, reinterpret_cast<const char*>(&arc), sizeof(Arc));
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

void ReadSimpleFst(const std::string &filename, SimpleFst *fst) {
    BinaryInput bi(filename);
    ASSERT(fst);
    fst->Read(bi.Stream());
}