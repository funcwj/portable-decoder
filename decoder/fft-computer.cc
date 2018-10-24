// wujian@2018

#include "fft-computer.h"

void FFTComputer::ComplexBitReverse(Float32 *cplx_values, Int32 num_values) {
  for (Int32 j = 0, i = 0; i < num_values - 1; i++) {
    if (i < j) {
      std::swap(REAL_PART(cplx_values, i), REAL_PART(cplx_values, j));
      std::swap(IMAG_PART(cplx_values, i), IMAG_PART(cplx_values, j));
    }
    Int32 m = num_values >> 1;
    while (j >= m) {
      j = j - m;
      m = m >> 1;
    }
    j = j + m;
  }
}

void FFTComputer::ComplexFFT(Float32 *cplx_values, Int32 num_samples,
                             bool invert) {
  Int32 n = num_samples >> 1, s = register_size_ / n;
  // ASSERT(n == register_size_);

  ComplexBitReverse(cplx_values, n);

  Int32 i, j, m = 1, cnt, inc, k;
  Float32 WR, WI, Ri, Ii, Rj, Ij;

  while (m < n) {
    cnt = 0, inc = n / (m << 1);
    while (cnt < inc) {
      i = cnt * m * 2;
      for (int t = 0; t < m; t++, i++) {
        j = i + m, k = t * inc;
        // WR = cos(PI * k * 2 / n), WI = sin(PI * k * 2 / n);
        WR = cos_table_[k * s],
        WI = (invert ? -sin_table_[k * s] : sin_table_[k * s]);
        Rj = REAL_PART(cplx_values, j), Ij = IMAG_PART(cplx_values, j);
        Ri = REAL_PART(cplx_values, i), Ii = IMAG_PART(cplx_values, i);
        REAL_PART(cplx_values, i) = Ri + WR * Rj - WI * Ij;
        IMAG_PART(cplx_values, i) = Ii + WR * Ij + WI * Rj;
        REAL_PART(cplx_values, j) = Ri - WR * Rj + WI * Ij;
        IMAG_PART(cplx_values, j) = Ii - WR * Ij - WI * Rj;
      }
      cnt++;
    }
    m = m << 1;
  }

  if (invert)
    for (i = 0; i < num_samples; i++) cplx_values[i] = cplx_values[i] / n;
}

void FFTComputer::RealFFT(Float32 *real_values, Int32 num_samples) {
  if (num_samples != register_size_) {
    LOG_FAIL << "Assert num_samples == register_size_ failed, " << num_samples
             << " vs " << register_size_;
  }

  Int32 n = num_samples >> 1, s = register_size_ / num_samples;
  memcpy(cplx_cache_, real_values, sizeof(Float32) * num_samples);

  ComplexFFT(cplx_cache_, num_samples, false);
  Float32 FR, FI, GR, GI, YR, YI, CYR, CYI, XR, XI, cosr, sinr;

  for (int r = 1; r < n; r++) {
    YR = REAL_PART(cplx_cache_, r), CYR = REAL_PART(cplx_cache_, n - r);
    YI = IMAG_PART(cplx_cache_, r), CYI = -IMAG_PART(cplx_cache_, n - r);
    FR = (YR + CYR) / 2, FI = (YI + CYI) / 2;
    GR = (YI - CYI) / 2, GI = (CYR - YR) / 2;
    cosr = cos_table_[r * s];
    sinr = sin_table_[r * s];
    XR = FR + cosr * GR - sinr * GI;
    XI = FI + cosr * GI + sinr * GR;
    REAL_PART(real_values, r) = XR;
    IMAG_PART(real_values, r) = XI;
  }
  FR = REAL_PART(cplx_cache_, 0);
  GR = IMAG_PART(cplx_cache_, 0);
  REAL_PART(real_values, 0) = FR + GR;
  IMAG_PART(real_values, 0) = FR - GR;
}