// wujian@2018

#include <iomanip>

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "nnet3/nnet-utils.h"
#include "nnet3/nnet-simple-component.h"
#include "nnet3/nnet-normalize-component.h"

int main(int argc, char *argv[]) {
    try {
        using namespace kaldi;
        using namespace kaldi::nnet3;

        const char *usage = 
            "Copy TDNN parameters from Kaldi's raw nnet3 model, "
            "which aims for propagating through pytorch.\n"
            "\n"
            "Usage: copy-nnet3-linear <nnet3-mdl> <nnet3-params>\n"
            "\n";

        ParseOptions po(usage);
        po.Read(argc, argv);

        if (po.NumArgs() != 2) {
            po.PrintUsage();
            exit(1);
        }
        
        std::string nnet_rxfilename = po.GetArg(1), param_wxfilename = po.GetArg(2);
        std::ofstream os(param_wxfilename, std::ios::binary | std::ios::out);

        Nnet raw_nnet;
        ReadKaldiObject(nnet_rxfilename, &raw_nnet);
        // compute batchnorm statstics
        SetBatchnormTestMode(true, &raw_nnet);

        int32 num_components = raw_nnet.NumComponents();
        std::cout << "Number Components: " << num_components << std::endl;

        for (int32 i = 0; i < num_components; i++) {
            Component *c = raw_nnet.GetComponent(i);
            const std::string &type = c->Type();
            std::cout << std::setw(3) << i << ": " << type << std::endl;
            if (type == "FixedAffineComponent") {
                WriteToken(os, true, "Linear");
                const FixedAffineComponent *fa = dynamic_cast<const FixedAffineComponent*>(c);
                const Matrix<BaseFloat> mat(fa->LinearParams());
                const Vector<BaseFloat> vec(fa->BiasParams());
                mat.Write(os, true);
                vec.Write(os, true);
            } else if (type == "NaturalGradientAffineComponent") {
                WriteToken(os, true, "Linear");
                const NaturalGradientAffineComponent *ng = dynamic_cast<const NaturalGradientAffineComponent*>(c);
                const Matrix<BaseFloat> mat(ng->LinearParams());
                const Vector<BaseFloat> vec(ng->BiasParams());
                mat.Write(os, true);
                vec.Write(os, true);
            } else if (type == "BatchNormComponent") {
                WriteToken(os, true, "BatchNorm");
                const BatchNormComponent *bn = dynamic_cast<const BatchNormComponent*>(c);
                Vector<BaseFloat> scale(bn->Scale());
                Vector<BaseFloat> offset(bn->Offset());
                scale.Write(os, true);
                offset.Write(os, true);
            } else if (type == "RectifiedLinearComponent") {
                WriteToken(os, true, "ReLU");
            } else {
                KALDI_LOG << type << ": skipped, do nothing.";
            }
        }

        return 0;

    } catch (const std::exception &e) {
        std::cerr << e.what();
        return -1;
    }
}