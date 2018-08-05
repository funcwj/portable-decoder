// wujian@2018

#include <fstream>
#include "decoder/simple-fst.h"

int main(int argc, char const *argv[]) {
    std::fstream os("graph.fst", std::ios::binary|std::ios::in);
    SimpleFst fst;
    fst.Read(os);
    return 0;
}
