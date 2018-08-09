// wujian@2018

#include <sys/time.h>

#include "decoder/type.h"

const Int32 SEC_TO_USEC = 1000 * 1000;

class Timer {
public:
    Timer() { Reset(); }

    void Reset() {
        gettimeofday(&start_, NULL);
    }
    
    Float64 Elapsed() const {
        struct timeval stop;
        gettimeofday(&stop, NULL);
        Int64 beg = start_.tv_sec * SEC_TO_USEC + start_.tv_usec, 
            end = stop.tv_sec * SEC_TO_USEC + stop.tv_usec;
        return static_cast<Float64>(end - beg) / SEC_TO_USEC;
    }

private:
    struct timeval start_;
};