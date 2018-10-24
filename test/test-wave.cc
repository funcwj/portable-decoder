// wujian@2018

#include <Eigen/Dense>
#include "decoder/wave.h"

using namespace Eigen;

int main(int argc, char const *argv[]) {
  const Int32 N = 20000;
  VectorXf vector = VectorXf::Random(N);
  Wave in(vector.data(), N);
  WriteWave("egs.wav", in);
  Wave egs;
  ReadWave("egs.wav", &egs);
  LOG_INFO << "num channels: " << egs.NumChannels();
  LOG_INFO << "num samples: " << egs.NumSamples();
  LOG_INFO << "sample rate: " << egs.SampleRate();
  // Map<VectorXf> wav(egs.Data(), egs.NumSamples());
  // std::cout << wav;
  return 0;
}
