// wujian@2018

#include "decoder/fft-computer.h"


int main() {
    size_t N = 8;
    
    float cplx_values[16] = {
        1, 0, 1, 0, 1, 0, 1, 0, 
        0, 0, 0, 0, 0, 0, 0, 0
    };
    
    float real_values[8] = {
        1, 1, 1, 1, 0, 0, 0, 0
    };

    FFTComputer fftcomputer(N);

    fftcomputer.ComplexFFT(cplx_values, N * 2, false);
    for (size_t i = 0; i < N; i++) {
        std::cout << "[" << REAL_PART(cplx_values, i) << ", " 
            << IMAG_PART(cplx_values, i) << "]" << std::endl;
    }
    fftcomputer.RealFFT(real_values, N);
    for (size_t i = 0; i < N / 2; i++) {
        std::cout << "[" << REAL_PART(real_values, i) << ", "  
            << IMAG_PART(real_values, i) << "]" << std::endl;
    }
    return 0;
}