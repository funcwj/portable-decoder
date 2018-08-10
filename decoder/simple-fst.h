// wujian@2018

// Simple FST

#ifndef SIMPLE_FST_H
#define SIMPLE_FST_H

#include "decoder/common.h"

struct Arc {
    Label ilabel, olabel;
    Weight weight;
    StateId nextstate;

    Arc() {}

    Arc(Label ilabel, Label olabel, Weight weight, StateId nextstate): 
        ilabel(ilabel), olabel(olabel), 
        weight(weight), nextstate(nextstate) { }

    std::string ToString() const {
        std::ostringstream oss;
        oss << "(" << ilabel << "/" << olabel << "/" << weight << "->" << nextstate << ")";
        return oss.str();
    }
};


class State {
public:
    State(): final_(TROPICAL_ZERO32), niepsilons_(0), noepsilons_(0) { }

    Weight Final() const { return final_; }

    const Arc& GetArc(Int32 index) { return arcs_[index]; }
    
    const Arc *Arcs() const { return !arcs_.empty() ? &arcs_[0] : nullptr; }

    UInt64 NumArcs() const { return arcs_.size(); }

    UInt64 NumInputEps() const { return niepsilons_; }

    UInt64 NumOutputEps() const { return noepsilons_; }

    void SetFinal(Weight weight) { final_ = weight; } 

    void ReserveArcs(Int32 n) { arcs_.reserve(n); }

    void SetNumInputEps(Int32 number) { niepsilons_ = number; }

    void SetNumOutputEps(Int32 number) { noepsilons_ = number; }

    void DeleteArcs() {
        niepsilons_ = noepsilons_ = 0;
        arcs_.clear();
    }

    void Reset() {
        final_ = 0.0;
        DeleteArcs();
    }

    void AddArc(const Arc& arc) {
        if (arc.ilabel == 0) niepsilons_++;
        if (arc.olabel == 0) noepsilons_++;
        arcs_.push_back(arc);
    }

    void SetArc(const Arc &arc, Int32 n) {
        if (arcs_[n].ilabel == 0) --niepsilons_;
        if (arcs_[n].olabel == 0) --noepsilons_;
        if (arc.ilabel == 0) ++niepsilons_;
        if (arc.olabel == 0) ++noepsilons_;
        arcs_[n] = arc;
    }

private:
    Weight final_;
    UInt64 niepsilons_, noepsilons_;
    std::vector<Arc> arcs_;
};

class SimpleFst {

public:
    SimpleFst(): start_(NoStateId) {}

    SimpleFst(const SimpleFst &fst);
    
    ~SimpleFst() {
        for (State *state: states_) {
            delete state;
        }
    }

    Bool Equal(const SimpleFst &fst);

    void Read(std::istream &is);

    void Write(std::ostream &os);

    StateId Start() const { return start_; }

    Weight Final(StateId state) const { return states_[state]->Final(); }

    UInt64 NumStates() const { return states_.size(); }

    UInt64 NumArcs(StateId state) const { return states_[state]->NumArcs(); }

    void SetStart(StateId state) { start_ = state; }

    void SetFinal(StateId state, Weight weight) { states_[state]->SetFinal(weight); }

    void SetState(StateId state, State *vstate) { states_[state] = vstate; }

    State *GetState(StateId state) { return states_[state]; }

    const State *GetState(StateId state) const { return states_[state]; }

    void AddArc(StateId state, const Arc &arc) { states_[state]->AddArc(arc); }

    void ReserveArcs(StateId state, Int32 n) { states_[state]->ReserveArcs(n); }

    StateId AddState() {
        State *new_state = new State();
        states_.push_back(new_state);
        return states_.size() - 1;
    }

    StateId AddState(State *state) {
        states_.push_back(state);
        return states_.size() - 1;
    }

    UInt64 NumInputEpsilons(StateId state) const { return GetState(state)->NumInputEps(); }

    UInt64 NumOutputEpsilons(StateId state) const { return GetState(state)->NumOutputEps(); }

private:
    StateId start_;
    std::vector<State*> states_;
};

/*
for (StateIterator siter(fst);
    !siter.Done();
    siter.Next()) {
    StateId s = siter.Value();
    ...
}
*/
class StateIterator {
public:
    StateIterator(const SimpleFst& fst): cur_state_(0), num_states_(fst.NumStates()) { }

    bool Done() { return cur_state_ >= num_states_; }

    void Next() { cur_state_++; }

    void Reset() { cur_state_ = 0; }

    StateId Value() { return cur_state_; }

private:
    StateId cur_state_, num_states_;
};

/*
for(ArcIterator aiter(fst, s); 
    !aiter.Done(); 
    aiter.Next()) {
    Arc &arc = aiter.Value();
    ...
}
*/
class ArcIterator {
public:
    ArcIterator(const SimpleFst& fst, StateId state): cur_arc_(0), 
                num_arcs_(fst.NumArcs(state)), arc_ptr_(fst.GetState(state)->Arcs()) { }

    bool Done() { return cur_arc_ >= num_arcs_; }

    void Next() { cur_arc_++; }

    void Reset() { cur_arc_ = 0; }

    const Arc& Value() { return arc_ptr_[cur_arc_]; }
    
private:
    UInt64 cur_arc_, num_arcs_;
    const Arc *arc_ptr_;
};


void ReadSimpleFst(const std::string &filename, SimpleFst *fst);


#endif