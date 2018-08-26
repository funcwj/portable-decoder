# wujian@2018

from libcpp.string cimport string
from libcpp.vector cimport vector

from libc.stdint cimport *
from libcpp cimport bool

cdef extern from "decoder/type.h":
    ctypedef int32_t Int32
    ctypedef int64_t Int64
    ctypedef float  Float32
    ctypedef double Float64
    ctypedef bool Bool

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
