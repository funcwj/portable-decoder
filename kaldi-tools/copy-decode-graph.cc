// wujian@2018

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "fstext/fstext-lib.h"

using namespace kaldi;

struct SimpleArc {
    int32 ilabel, olabel;
    BaseFloat weight;
    int32 nextstate;

    SimpleArc(const fst::StdArc &arc): ilabel(arc.ilabel), olabel(arc.olabel),
                                weight(arc.weight.Value()), nextstate(arc.nextstate) { }
}; 

void WriteBinaryArc(std::ostream &os, const SimpleArc &arc) {
    char num_bytes = sizeof(SimpleArc);
    os.write(reinterpret_cast<const char*>(&num_bytes), 1);
    os.write(reinterpret_cast<const char*>(&arc), num_bytes);
}

int main(int argc, char const *argv[]) {
    try {
        using namespace fst;

        const char *usage =
            "Transform format of Kaldi's decode graph(HCLG.fst) which is supported by simple asr-decoder\n"
            "\n"
            "Usage: copy-decode-graph <kaldi-graph> <decoder-graph>\n";

        ParseOptions po(usage);
        po.Read(argc, argv);

        if (po.NumArgs() != 2) {
            po.PrintUsage();
            return 1;
        }

        std::string in_rxfilename = po.GetArg(1), out_rxfilename = po.GetArg(2);

        fst::Fst<fst::StdArc> *decode_fst = ReadFstKaldiGeneric(in_rxfilename);

        std::fstream os(out_rxfilename.c_str(), std::ios::binary|std::ios::out);


        int64 num_states = 0, num_arcs = 0;
        for (fst::StateIterator<fst::Fst<fst::StdArc> > siter(*decode_fst);
                !siter.Done();
                siter.Next()) {
            num_states++;
            fst::StdArc::StateId state = siter.Value();
            for (fst::ArcIterator<fst::Fst<fst::StdArc> > aiter(*decode_fst, state);
                !aiter.Done(); aiter.Next()) {
                num_arcs++;
            }
        }

        KALDI_LOG << "Read decoder graph(type:" << decode_fst->Type() <<") contains " << num_states << " states and "
                  << num_arcs << " arcs with start index " << decode_fst->Start();

        // StateId => int
        WriteBasicType(os, true, decode_fst->Start());
        // int64
        WriteBasicType(os, true, num_states);
        WriteBasicType(os, true, num_arcs);

        for (fst::StateIterator<fst::Fst<fst::StdArc> > siter(*decode_fst);
                !siter.Done();
                siter.Next()) {
            fst::StdArc::StateId state = siter.Value();
            // final->num_arcs
            BaseFloat final_cost = decode_fst->Final(state).Value();
            WriteBasicType(os, true, final_cost);
            // size_t
            WriteBasicType(os, true, decode_fst->NumArcs(state));
            for (fst::ArcIterator<fst::Fst<fst::StdArc> > aiter(*decode_fst, state);
                !aiter.Done(); aiter.Next()) {
                const fst::StdArc &arc = aiter.Value();
                SimpleArc simple_arc(arc);
                WriteBinaryArc(os, simple_arc);
            }
        }
        delete decode_fst;
        return 0;

    } catch (const std::exception &e) {
        std::cerr << e.what();
        return -1;
    }

}