// wujian@2018

#include <Eigen/Dense>
#include "decoder/wave.h"
#include "decoder/signal.h"

using namespace Eigen;

using Mat = Matrix<Float32, Dynamic, Dynamic, RowMajor>;

// Compared with command:
// echo "egs egs.wav" | compute-mfcc-feats --window-type=hamming --dither=0.0  scp:-  ark,t:-

void TestMfcc() {
    MfccOpts mfcc_opts;
    std::cout << "Mfcc configure: " << std::endl << mfcc_opts.Configure();
    MfccComputer computer(mfcc_opts);

    Wave egs;
    ReadWave("egs.wav", &egs);

    Int32 num_samples = egs.NumSamples();
    Int32 num_frames = computer.NumFrames(num_samples), dim = computer.FeatureDim();
    LOG_INFO << "Compute mfcc for " << num_samples << " samples";
    Mat mfcc = Mat::Zero(num_frames, dim);
    LOG_INFO << mfcc.rows() << " x " << mfcc.cols() << "(" << mfcc.stride() << ")";
    ComputeFeature(computer, egs.Data(), num_samples, mfcc.data(), mfcc.stride());
    LOG_INFO << "Shape of mfcc: " << num_frames << " x " << dim;
    std::cout << mfcc << std::endl;
}

// Compared with command:
// echo "egs egs.wav" | compute-fbank-feats --window-type=hamming --dither=0.0 --num-mel-bins=40 scp:-  ark,t:- 

void TestFBank() {
    FbankOpts fbank_opts;
    fbank_opts.num_mel_bins = 40;
    std::cout << "Fbank configure: " << std::endl << fbank_opts.Configure();
    FbankComputer computer(fbank_opts);

    Wave egs;
    ReadWave("egs.wav", &egs);

    Int32 num_samples = egs.NumSamples();
    Int32 num_frames = computer.NumFrames(num_samples), dim = computer.FeatureDim();
    LOG_INFO << "Compute fbank for " << num_samples << " samples";
    Mat fbank = Mat::Zero(num_frames, dim);
    LOG_INFO << fbank.rows() << " x " << fbank.cols() << "(" << fbank.stride() << ")";
    ComputeFeature(computer, egs.Data(), num_samples, fbank.data(), fbank.stride());
    LOG_INFO << "Shape of fbank: " << num_frames << " x " << dim;
    std::cout << fbank << std::endl;
}

// Compared with command:
// echo "egs egs.wav" | compute-spectrogram-feats --window-type=hamming --dither=0.0 scp:-  ark,t:- 

void TestSpectrogram() {
    SpectrogramOpts spectrogram_opts;
    std::cout << "Spectrogram configure: " << std::endl << spectrogram_opts.Configure();
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
    std::cout << "FrameSplitter configure: " << std::endl << frame_opts.Configure();
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
    // TestFBank();
    TestMfcc();
    return 0;
}
