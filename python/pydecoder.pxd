# wujian@2018

from typedef cimport Int32, Float32, Bool, WindowType
from libcpp.string cimport string

cdef extern from "decoder/online.h":
    cdef cppclass FeatureExtractor:
        FeatureExtractor(const string& conf, const string &ftype)
        Int32 Compute(Float32 *signal, Int32 num_samps, Float32 *addr, Int32 stride)
        Int32 FeatureDim()
        Int32 NumFrames(Int32 num_samps)
    