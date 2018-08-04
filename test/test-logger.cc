// wujian@2018

#include "decoder/logger.h"


int main(int argc, char const *argv[]) {
    LOG_INFO << "TEST-INFO";
    LOG_WARN << "TEST-WARN";
    LOG_INFO << "TEST-INFO";
    ASSERT(1 == 1);
    LOG_FAIL << "TEST-FAIL";
}
