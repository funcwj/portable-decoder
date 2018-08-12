// wujian@2018

#include <Eigen/Dense>

#include "decoder/signal.h"
#include "decoder/online.h"

using namespace Eigen;

using Mat = Matrix<Float32, Dynamic, Dynamic, RowMajor>;

void TestOnlineVad() {
    EnergyVadOpts vad_opts(20, 10, 10, 7);
    EnergyVadWrapper vad(vad_opts);

    FrameOpts frame_opts(10, 7, 4000, 0.0, kNone, false);
    FrameSplitter splitter(frame_opts);

    const Int32 N = 200;
    Float32 *egs = new Float32[N];
    std::vector<Int32> vad_stats;
    for (Int32 n = 0; n < N; n++)
        egs[n] = n;
    vad.Run(egs, N, &vad_stats, splitter.NumFrames(N));
    delete[] egs;
}

void TestOnlineSplitter() {
    FrameOpts frame_opts(10, 7, 4000, 0.0, kNone, false);
    FrameSplitter splitter(frame_opts);
    const Int32 N = 200, C = 50;
    Float32 *egs = new Float32[N];
    for (Int32 n = 0; n < N; n++)
        egs[n] = n;
    Int32 num_frames, dim = splitter.FrameLength();
    for (Int32 i = 0; i < N / C; i++) {
        num_frames = splitter.NumFrames(C);
        Mat online_frames = Mat::Zero(num_frames, dim);
        splitter.Frame(egs + i * C, C, online_frames.data(), online_frames.stride());
        std::cout << "Online " << num_frames <<  " frames: " << std::endl << online_frames << std::endl;
    }
    splitter.Reset();

    num_frames = splitter.NumFrames(N);
    Mat offline_frames = Mat::Zero(num_frames, dim);
    splitter.Frame(egs, N, offline_frames.data(), offline_frames.stride());
    std::cout << "Offline " << num_frames <<  " frames: " << std::endl << offline_frames << std::endl;
    delete[] egs;
}


int main(int argc, char const *argv[]) {
    // TestOnlineSplitter();
    TestOnlineVad();
    return 0;
}
