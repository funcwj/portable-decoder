// wujian@2018

#ifndef TYPE_H
#define TYPE_H

#include <cstdint>

#include "decoder/logger.h"

using Float32 = float;
using Float64 = double;

using Int08 = int8_t;
using Int16 = int16_t;
using Int32 = int32_t;
using Int64 = int64_t;

using UInt08 = uint8_t;
using UInt16 = uint16_t;
using UInt32 = uint32_t;
using UInt64 = uint64_t;

using Bool = bool;
using Weight    = float;
using Label     = int32_t;
using StateId   = int32_t;

const StateId NoStateId = -1;
const char END_OF_STRING = '\0';

enum WindowType {
    kHann,
    kHamm,
    kRect,
    kBlackMan
};



#endif