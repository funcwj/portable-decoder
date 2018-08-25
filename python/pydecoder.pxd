# wujian@2018

from typedef cimport Int32, Float32, Bool

from libcpp.string cimport string
from libcpp.vector cimport vector

# wrappers for feature extractor
cdef extern from "decoder/online.h":
    cdef cppclass FeatureExtractor:
        FeatureExtractor(const string&, const string&) except +
        Int32 Compute(Float32*, Int32, Float32*, Int32) except +
        Int32 FeatureDim()
        Int32 NumFrames(Int32 num_samps)
        void Reset()

# wrappers for decoder  
cdef extern from "decoder/decoder.h":
    cdef cppclass FasterDecoder:
        FasterDecoder(const string&, const string&, const string&) except +
        void Reset()
        void Decode(Float32*, Int32, Int32, Int32)
        void DecodeFrame(Float32*, Int32)
        Bool GetBestPath(vector[Int32]*)
