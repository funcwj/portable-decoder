// wujian@2018

#include <fstream>
#include "decoder/simple-fst.h"

int main(int argc, char const *argv[]) {
    SimpleFst fst;
    Timer timer;
    ReadSimpleFst("graph.fst", &fst);
    LOG_INFO << "Cost " << timer.Elapsed() << " s";
    return 0;
}
