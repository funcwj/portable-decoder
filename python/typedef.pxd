
from libc.stdint cimport *
from libcpp cimport bool

cdef extern from "decoder/type.h":

    ctypedef int32_t Int32
    ctypedef int64_t Int64
    ctypedef float  Float32
    ctypedef double Float64

    ctypedef bool Bool