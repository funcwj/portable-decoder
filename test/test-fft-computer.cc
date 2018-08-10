// wujian@2018

#include <Eigen/Dense>
#include "decoder/fft-computer.h"

using namespace Eigen;

int main() {
    Int32 N = 8;
    
    VectorXf real_vector = VectorXf::Random(N);
    std::cout << real_vector << std::endl;
    FFTComputer fftcomputer(N);
    
    fftcomputer.RealFFT(reinterpret_cast<Float32*>(real_vector.data()), N);
    std::cout << real_vector << std::endl;
    return 0;
}