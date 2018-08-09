// wujian@2018

#include <fstream>
#include "decoder/simple-fst.h"

int main(int argc, char const *argv[]) {
    SimpleFst fst;
    ReadSimpleFst("graph.fst", &fst);
    return 0;
}
