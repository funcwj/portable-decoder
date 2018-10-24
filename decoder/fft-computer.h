// wujian@2018

// First implemented in
// https://github.com/funcwj/asr-utils/blob/master/fft/fft-computer.h Modified
// 2018.7

#ifndef FFT_COMPUTER_H
#define FFT_COMPUTER_H

#include "decoder/common.h"

// Class for FFT computation

class FFTComputer {
 public:
  FFTComputer(Int32 register_size) : register_size_(register_size) {
    // ASSERT(RoundUpToNearestPowerOfTwo(register_size) == register_size);
    Int32 table_size = register_size >> 1;
    cos_table_ = new Float32[table_size];
    sin_table_ = new Float32[table_size];
    // pre-compute cos/sin values for FFT
    for (Int32 k = 0; k < table_size; k++) {
      cos_table_[k] = cos(PI * k / table_size);
      sin_table_[k] = sin(PI * k / table_size);
    }
    // for RealFFT data cache
    cplx_cache_ = new Float32[register_size];
  }

  // Compute (inverse)FFT values
  // cplx_values: [R0, I0, R1, I1, ... R(N - 1), I(N - 1)]
  // num_samples: length of cplx_values(N) and register_size_ == num_samples >>
  // 1 invert: for inverse FFT, invert = true
  void ComplexFFT(Float32 *cplx_values, Int32 num_samples, bool invert);

  // Compute RealFFT values
  // real_values: [R0, R1, ... R(N - 1)
  // num_samples: length of real_values(N), equals register_size_
  void RealFFT(Float32 *real_values, Int32 num_samples);

  ~FFTComputer() {
    if (sin_table_) delete[] sin_table_;
    if (cos_table_) delete[] cos_table_;
    if (cplx_cache_) delete[] cplx_cache_;
  }

 private:
  // Required 2^N
  Int32 register_size_;
  // Precomputed values
  Float32 *sin_table_, *cos_table_, *cplx_cache_;
  // BitReverse for complex values
  void ComplexBitReverse(Float32 *complex_values, Int32 num_values);
};

#endif  // FFT_COMPUTER_H