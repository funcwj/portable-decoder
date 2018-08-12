# wujian@2018

import numpy as np
cimport numpy as cnp

from typedef cimport Int32, Float32, Bool, WindowType
from libcpp.string cimport string

cdef extern from "decoder/signal.h":
    cdef cppclass FrameOpts:
        FrameOpts()
        FrameOpts(Int32, Int32, Int32, Float32, WindowType, Bool)
        string Configure()

    cdef cppclass SpectrogramOpts:
        SpectrogramOpts()
        SpectrogramOpts(Bool, Bool, Bool)
        string Configure()
    
    cdef cppclass FbankOpts:
        FbankOpts()
        FbankOpts(Int32, Int32, Int32, Bool, Bool)
        string Configure()

    cdef cppclass MfccOpts:
        MfccOpts()
        # cdef FbankOpts fbank_opts
        MfccOpts(Int32, Bool, Float32)
        string Configure()


