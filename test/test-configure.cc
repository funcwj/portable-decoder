// wujian@2018

#include "decoder/config.h"
#include "decoder/signal.h"

/*
--MfccOpts.num_ceps=40
--MfccOpts.cepstral_lifter=40.0
--FrameOpts.remove_dc=false
--FrameOpts.window=hanning
*/
int main(int argc, char const *argv[]) {
    ConfigureParser parser("mfcc.conf");
    std::cout << ">> Parse configures:\n" << parser.Configure();
    MfccOpts mfcc_opts;
    mfcc_opts.ParseConfigure(&parser);
    std::cout << ">> Final configures:\n" << mfcc_opts.Configure();
    return 0;
}

