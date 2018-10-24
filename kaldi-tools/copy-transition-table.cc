// wujian@2018

#include "hmm/transition-model.h"
#include "util/common-utils.h"

int main(int argc, char const *argv[]) {
  try {
    using namespace kaldi;

    const char *usage =
        "Copy TransitionTable(mapping from tid to pdf) from Kaldi's Transition "
        "models, which will be used in simple-decoder\n"
        "\n"
        "Usage: copy-transition-table <transition-model> <tid-map>\n";

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
    // Output ko(transition_mapped_wxfilename, true);
    std::ofstream os(transition_mapped_wxfilename.c_str(),
                     std::ios::out | std::ios::binary);

    int32 num_tids = trans_model.NumTransitionIds(),
          num_pdfs = trans_model.NumPdfs();
    WriteBasicType(os, true, num_tids);
    WriteBasicType(os, true, num_pdfs);
    int32 *table = new int32[num_tids];
    for (int32 tid = 0; tid < num_tids; tid++)
      // WriteBasicType(ko.Stream(), true, trans_model.TransitionIdToPdf(tid));
      // NOTE: tid + 1 important, transition-id start from 1
      table[tid] = trans_model.TransitionIdToPdf(tid + 1);
    os.write(reinterpret_cast<const char *>(table), sizeof(int32) * num_tids);
    KALDI_LOG << "Copy from transition model with " << num_tids << " tids and "
              << num_pdfs << " pdfs done";
    delete[] table;
  } catch (const std::exception &e) {
    std::cerr << e.what();
    return -1;
  }
}
