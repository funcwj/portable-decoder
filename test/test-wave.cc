// wujian@2018

#include <ctime>
#include "decoder/wave.h"

int main(int argc, char const *argv[]) {
    const Int32 N = 20000;

    Float32 random_data[N];
    srand(time(NULL));

    for (Int32 i = 0; i < N; i++) {
        random_data[i] = rand() % 1000 + 1;
    }
    Wave in(random_data, N);
    WriteWave("demo.wav", in);

    Wave out;
    ReadWave("demo.wav", &out);
    LOG_INFO << "num channels: " << out.NumChannels();
    LOG_INFO << "num samples: " << out.NumSamples();
    LOG_INFO << "sample rate: " << out.SampleRate();
    return 0;
}
