// decoder/online.cc

// wujian@2018

#include "decoder/online.h"

FeatureType StringToFeatureType(const std::string &type) {
  if (type == "spectrogram")
    return kSpectrogram;
  else if (type == "fbank")
    return kFbank;
  else if (type == "mfcc")
    return kMfcc;
  else
    return kUnkown;
}

FeatureExtractor::FeatureExtractor(const std::string &conf,
                                   const std::string &type)
    : computer_(NULL) {
  type_ = StringToFeatureType(type);
  ConfigureParser parser(conf);
  // initialize
  SpectrogramOpts spectrogram_opts;
  FbankOpts fbank_opts;
  MfccOpts mfcc_opts;
  switch (type_) {
    case kSpectrogram:
      LOG_INFO << "Create FeatureExtractor(Spectrogram)";
      spectrogram_opts.ParseConfigure(&parser);
      computer_ = new SpectrogramComputer(spectrogram_opts);
      break;
    case kFbank:
      LOG_INFO << "Create FeatureExtractor(Fbank)";
      fbank_opts.ParseConfigure(&parser);
      computer_ = new FbankComputer(fbank_opts);
      break;
    case kMfcc:
      LOG_INFO << "Create FeatureExtractor(Mfcc)";
      mfcc_opts.ParseConfigure(&parser);
      computer_ = new MfccComputer(mfcc_opts);
      break;
    case kUnkown:
      LOG_FAIL << "Unknown feature type: " << type;
      break;
  }
}

Int32 FeatureExtractor::Compute(Float32 *signal, Int32 num_samps, Float32 *addr,
                                Int32 stride) {
  Int32 num_frames = -1;
  switch (type_) {
    case kSpectrogram:
    case kFbank:
    case kMfcc:
      num_frames = ComputeFeature(computer_, signal, num_samps, addr, stride);
      break;
    case kUnkown:
      LOG_FAIL
          << "Init FeatureExtractor with unknown feature type, stop compute";
      break;
  }
  return num_frames;
}