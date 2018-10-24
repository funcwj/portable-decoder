// decoder/math.cc
// wujian@2018

#include "math.h"

Int32 RoundUpToNearestPowerOfTwo(Int32 n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return n + 1;
}

Float32 LogFloat32(Float32 linear) { return logf(std::max(EPS_F32, linear)); }

Float32 ToMelScale(Float32 linear) {
  return 1127.0f * logf(1.0f + linear / 700.0f);
}

Float32 ToDB(Float32 linear) { return 10 * log10f(std::max(EPS_F32, linear)); }