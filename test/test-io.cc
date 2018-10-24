// wujian@2018

#include "decoder/io.h"

int main(int argc, char const *argv[]) {
  {
    BinaryOutput bo("TEST.IO");
    Float32 f = 3.141438;
    Int32 i = 1234;
    WriteBinaryBasicType(bo.Stream(), f);
    WriteBinaryBasicType(bo.Stream(), i);
    LOG_INFO << "Write Float32(" << f << ") Int32(" << i << ") done";
  }
  {
    BinaryInput bi("TEST.IO");
    Float32 f;
    Int32 i;
    ReadBinaryBasicType(bi.Stream(), &f);
    ReadBinaryBasicType(bi.Stream(), &i);
    LOG_INFO << "Read Float32(" << f << ") Int32(" << i << ") done";
  }

  return 0;
}
