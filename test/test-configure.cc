// wujian@2018

#include <Eigen/Dense>
#include "decoder/config.h"
#include "decoder/signal.h"
#include "decoder/wave.h"

using namespace Eigen;

using Mat = Matrix<Float32, Dynamic, Dynamic, RowMajor>;

void TestConfigure() {
  ConfigureParser parser("mfcc.conf");
  std::cout << ">> Parse configures:\n" << parser.Configure();
  MfccOpts mfcc_opts;
  mfcc_opts.ParseConfigure(&parser);
  std::cout << ">> Final configures:\n" << mfcc_opts.Configure();

  MfccComputer computer(mfcc_opts);

  Wave egs;
  ReadWave("egs.wav", &egs);

  Int32 num_samples = egs.NumSamples();
  Int32 num_frames = computer.NumFrames(num_samples),
        dim = computer.FeatureDim();
  LOG_INFO << "Compute mfcc for " << num_samples << " samples";
  Mat mfcc = Mat::Zero(num_frames, dim);
  LOG_INFO << mfcc.rows() << " x " << mfcc.cols() << "(" << mfcc.stride()
           << ")";
  ComputeFeature(&computer, egs.Data(), num_samples, mfcc.data(),
                 mfcc.stride());
  LOG_INFO << "Shape of mfcc: " << num_frames << " x " << dim;
  std::cout << mfcc << std::endl;
}
/*
--MfccOpts.num_ceps=40
--MfccOpts.cepstral_lifter=40.0
--FrameOpts.remove_dc=false
--FrameOpts.window=hanning
*/
int main(int argc, char const *argv[]) {
  TestConfigure();
  return 0;
}
