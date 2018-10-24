// decoder/online.h

// wujian@2018
// TODO: improve vad or use py-webrtcvad in python scripts
// py-webrtcvad: https://github.com/wiseman/py-webrtcvad

#ifndef ONLINE_H
#define ONLINE_H

#include <queue>

#include "decoder/common.h"
#include "decoder/config.h"
#include "decoder/signal.h"

enum VadStatus { kSilence, kActive };

enum FeatureType { kSpectrogram, kFbank, kMfcc, kUnkown };

FeatureType StringToFeatureType(const std::string &type);

// Simple FeatureExtractor(mfcc/spectrogram/fbank)
class FeatureExtractor {
 public:
  FeatureExtractor(const std::string &conf, const std::string &type);

  Int32 Compute(Float32 *signal, Int32 num_samps, Float32 *addr, Int32 stride);

  void Reset() { computer_->Reset(); }

  Int32 FeatureDim() { return computer_->FeatureDim(); }

  Int32 NumFrames(Int32 num_samps) { return computer_->NumFrames(num_samps); }

  ~FeatureExtractor() {
    if (computer_ != NULL) delete computer_;
  }

 private:
  FeatureType type_;
  Computer *computer_;
};


#endif