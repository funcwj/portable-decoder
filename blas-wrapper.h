// wujian@2018


#include "openblas/cblas.h"
#include "type.h"

Float32 VdotV(const Float32 *a, const Float32 *b, const Int32 N) {
    return cblas_sdot(N, a, 1, b, 1);
}

Float64 VdotV(const Float64 *a, const Float64 *b, const Int32 N) {
    return cblas_ddot(N, a, 1, b, 1);
}

