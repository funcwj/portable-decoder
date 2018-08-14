# wujian@2018

import numpy as np
cimport numpy as cnp

from typedef cimport Int32, Float32, Bool, WindowType
from libcpp.string cimport string

cdef extern from "decoder/config.h":
    cdef cppclass ConfigureParser:
        ConfigureParser(const string &conf)


cdef extern from "decoder/signal.h":
    cdef cppclass SpectrogramOpts:
        SpectrogramOpts()
        void ParseConfigure(ConfigureParser *parser)
        string Configure()

    cdef cppclass MfccOpts:
        MfccOpts()
        void ParseConfigure(ConfigureParser *parser)
        string Configure()

    cdef cppclass FbankOpts:
        FbankOpts()
        void ParseConfigure(ConfigureParser *parser)
        string Configure()

cdef extern from "decoder/signal.h":
    cdef cppclass SpectrogramComputer:
        SpectrogramComputer(const SpectrogramOpts &spectrogram_opts)
        Float32 ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *spectrum_addr);
        Int32 NumFrames(Int32 num_samps)
        Int32 FeatureDim()

    cdef cppclass FbankComputer:
        FbankComputer(const FbankOpts &spectrogram_opts)
        Float32 ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *spectrum_addr);
        Int32 NumFrames(Int32 num_samps)
        Int32 FeatureDim()

    cdef cppclass MfccComputer:
        MfccComputer(const MfccOpts &spectrogram_opts)
        Float32 ComputeFrame(Float32 *signal, Int32 num_samps, Int32 t, Float32 *spectrum_addr);
        Int32 NumFrames(Int32 num_samps)
        Int32 FeatureDim()

ctypedef fused FeatureComputer:
    SpectrogramComputer
    FbankComputer
    MfccComputer

cdef extern from "decoder/signal.h":
    cdef Compute(FeatureComputer *computer, Float32 *signal, Int32 num_samps, Float32 *addr, Int32 stride)
