# wujian@2018


cimport pydecoder
cimport numpy as np 
import numpy as pynp

from libcpp.string cimport string

ctypedef np.float32_t F32

cdef class PyMfccComputer:
    cdef pydecoder.MfccComputer *computer

    def __cinit__(self, conf):
        # from python bytes to string
        cdef string mfcc_conf = <string>conf.encode("utf-8")
        cdef pydecoder.ConfigureParser *parser = new ConfigureParser(mfcc_conf)
        cdef pydecoder.MfccOpts opts
        opts.ParseConfigure(parser)
        self.computer = new MfccComputer(opts)

    def dim(self):
        return self.computer.FeatureDim()

    def compute(self, np.ndarray[F32, ndim=1] wav):
        cdef Int32 num_frames = self.computer.NumFrames(wav.size)
        cdef np.ndarray[F32, ndim=2] mfcc = pynp.zeros([num_frames, self.dim()], dtype=pynp.float32)
        self.computer.ComputeFrame(<Float32*>wav.data, wav.size, 0, <Float32*>mfcc.data)
        return mfcc
