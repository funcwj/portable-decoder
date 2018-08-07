// wujian@2018

#include "hmm/transition-model.h"
#include "util/common-utils.h"


int main(int argc, char const *argv[]) {
    try {
        using namespace kaldi;

        const char *usage = 
            "Copy mapping of tid & pdf from Kaldi's Transition models, which will be used in simple-decoder\n"
            "\n"
            "Usage: copy-tid-to-pdf <transition-model> <tid-map>\n";

        ParseOptions po(usage);
        po.Read(argc, argv);

        if (po.NumArgs() != 2) {
            po.PrintUsage();
            exit(1);
        }

        std::string transition_model_rxfilename = po.GetArg(1),
            transition_mapped_wxfilename = po.GetArg(2);

        TransitionModel trans_model;
        ReadKaldiObject(transition_model_rxfilename, &trans_model);
        Output ko(transition_mapped_wxfilename, true);
        
        int32 num_tids = trans_model.NumTransitionIds(), num_pdfs = trans_model.NumPdfs();
        WriteBasicType(ko.Stream(), true, num_tids);
        WriteBasicType(ko.Stream(), true, num_pdfs);

        for (int32 tid = 0; tid < num_tids; tid++)
            WriteBasicType(ko.Stream(), true, trans_model.TransitionIdToPdf(tid));
        KALDI_LOG << "Copy from transition model with " << num_tids << " tids and " << num_pdfs << " pdfs done";

    } catch (const std::exception &e) {
        std::cerr << e.what();
        return -1;
    }
}
