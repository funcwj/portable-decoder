// decoder/math.h
// wujian@2018

#ifndef MATH_H
#define MATH_H

#include <cmath>
#include <algorithm>

#include "decoder/type.h"

// #include "blas-wrapper.h"

const Float32 PI  = 3.1415926535897932384626433832795;
const Float32 PI2 = 6.283185307179586476925286766559005;
const Float32 EPS_F32 = std::numeric_limits<Float32>::epsilon();

const Int16 MIN_INT16 = std::numeric_limits<Int16>::min();
const Int16 MAX_INT16 = std::numeric_limits<Int16>::max();

const Float32 FLOAT32_INF = std::numeric_limits<Float32>::infinity();
const Float64 FLOAT64_INF = std::numeric_limits<Float64>::infinity();

// define Zero/One in tropical semiring
const Float32 TROPICAL_ZERO32 = FLOAT32_INF;
const Float32 TROPICAL_ONE32  = 1;

#define REAL_PART(complex_values, index) (complex_values[(index) << 1])
#define IMAG_PART(complex_values, index) (complex_values[((index) << 1) + 1])


Int32 RoundUpToNearestPowerOfTwo(Int32 n);

Float32 LogFloat32(Float32 linear);

Float32 ToMelScale(Float32 linear);

Float32 ToDB(Float32 linear);



#endif 