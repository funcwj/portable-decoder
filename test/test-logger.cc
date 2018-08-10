// wujian@2018

#include "decoder/logger.h"

int main(int argc, char const *argv[]) {
    LOG_INFO << "***TEST-INFO***\n";
    LOG_WARN << "***\nTEST-WARN***";
    LOG_INFO << "TEST-\nINFO";
    ASSERT(1 == 1);
    LOG_FAIL << "TEST-FAIL";
}
