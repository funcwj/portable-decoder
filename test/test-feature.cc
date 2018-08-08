// wujian@2018

#include <Eigen/Dense>
#include "decoder/wave.h"
#include "decoder/signal.h"

using namespace Eigen;

using Mat = Matrix<Float32, Dynamic, Dynamic, RowMajor>;

// compared with command:
// echo "egs egs.wav" | compute-fbank-feats --window-type=hamming --dither=0.0 --num-mel-bins=40 scp:-  ark,t:- 

void TestFBank() {
    FbankOpts fbank_opts;
    fbank_opts.num_bins = 40;
    FbankComputer computer(fbank_opts);

    Wave egs;
    ReadWave("egs.wav", &egs);

    Int32 num_samples = egs.NumSamples();
    Int32 num_frames = computer.NumFrames(num_samples), dim = computer.FeatureDim();
    LOG_INFO << "Compute filter-bank for " << num_samples << " samples";
    Mat spectrogram = Mat::Zero(num_frames, dim);
    LOG_INFO << spectrogram.rows() << " x " << spectrogram.cols() << "(" << spectrogram.stride() << ")";
    ComputeFeature(computer, egs.Data(), num_samples, spectrogram.data(), spectrogram.stride());
    LOG_INFO << "Shape of filter-bank: " << num_frames << " x " << dim;
    std::cout << spectrogram << std::endl;
}

// compared with command:
// echo "egs egs.wav" | compute-spectrogram-feats --window-type=hamming --dither=0.0 scp:-  ark,t:- 

void TestSpectrogram() {
    SpectrogramOpts spectrogram_opts;
    SpectrogramComputer computer(spectrogram_opts);

    Wave egs;
    ReadWave("egs.wav", &egs);

    Int32 num_samples = egs.NumSamples();
    Int32 num_frames = computer.NumFrames(num_samples), dim = computer.FeatureDim();
    LOG_INFO << "Compute spectrogram for " << num_samples << " samples";
    Mat spectrogram = Mat::Zero(num_frames, dim);
    LOG_INFO << spectrogram.rows() << " x " << spectrogram.cols() << "(" << spectrogram.stride() << ")";
    ComputeFeature(computer, egs.Data(), num_samples, spectrogram.data(), spectrogram.stride());
    LOG_INFO << "Shape of spectrogram: " << num_frames << " x " << dim;
    std::cout << spectrogram << std::endl;
}

void TestFrameSplitter() {
    FrameOpts frame_opts;
    FrameSplitter splitter(frame_opts);
    Wave egs;
    ReadWave("egs.wav", &egs);

    Int32 num_samples = egs.NumSamples();
    const Int32 num_frames = splitter.NumFrames(num_samples), dim = splitter.FrameLength();
    LOG_INFO << "Compute frame for " << num_samples << " samples";
    LOG_INFO << "Shape of frames: " << num_frames << " x " << dim;
    Mat frames = Mat::Zero(num_frames, dim);
    LOG_INFO << frames.rows() << " x " << frames.cols() << "(" << frames.stride() << ")";
    splitter.Frame(egs.Data(), num_samples, frames.data(), frames.stride());
    std::cout << frames << std::endl;
}

int main(int argc, char const *argv[]) {
    // TestFrameSplitter();
    // TestSpectrogram();
    TestFBank();
    return 0;
}
