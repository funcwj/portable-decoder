// wujian@2018

#include <fstream>

#include "io.h"

int main(int argc, char const *argv[]) {
    std::fstream os("IO.test", std::ios::binary|std::ios::in);
    // WriteBinaryBasicType(os, 2.453f);
    // WriteBinaryBasicType(os, 1234);
    Float32 f;
    Int32 a; 
    ReadBinaryBasicType(os, &f);
    ReadBinaryBasicType(os, &a);
    LOG_INFO << f << "/" << a;
    return 0;
}
