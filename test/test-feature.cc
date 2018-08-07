// wujian@2018

#include <Eigen/Dense>
#include "decoder/wave.h"
#include "decoder/signal.h"

using namespace Eigen;


void TestSpectrogram() {
    SpectrogramOpts spectrogram_opts;
    SpectrogramComputer computer(spectrogram_opts);

    Wave egs;
    ReadWave("egs.wav", &egs);

    Int32 num_samples = egs.NumSamples();
    Int32 num_frames = computer.NumFrames(num_samples), dim = computer.FeatureDim();
    LOG_INFO << "Compute spectrogram for " << num_samples << " samples";
    Matrix<Float32, Dynamic, Dynamic, RowMajor> spectrogram = 
        Matrix<Float32, Dynamic, Dynamic, RowMajor>::Zero(num_frames, dim);
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
    Matrix<Float32, Dynamic, Dynamic, RowMajor> frames = 
        Matrix<Float32, Dynamic, Dynamic, RowMajor>::Zero(num_frames, dim);
    LOG_INFO << frames.rows() << " x " << frames.cols() << "(" << frames.stride() << ")";
    splitter.Frame(egs.Data(), num_samples, frames.data(), frames.stride());
    std::cout << frames << std::endl;
}

int main(int argc, char const *argv[]) {
    // TestFrameSplitter();
    TestSpectrogram();
    return 0;
}
