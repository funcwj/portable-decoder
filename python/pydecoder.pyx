# wujian@2018


cimport pydecoder
cimport numpy as np 
import numpy as pynp

from libcpp.string cimport string

ctypedef np.float32_t F32

cdef class PyFeatureExtractor:
    cdef pydecoder.FeatureExtractor *extractor 

    def __cinit__(self, conf, feat):
        cdef string conf_str = <string>conf.encode("utf-8"), \
                    feat_str = <string>feat.encode("utf-8")
        self.extractor = new FeatureExtractor(conf_str, feat_str)

    def compute(self, np.ndarray[F32, ndim=1] wav):
        cdef Int32 num_frames = self.extractor.NumFrames(wav.size)
        cdef np.ndarray[F32, ndim=2] feats = pynp.zeros([num_frames, \
                                                        self.extractor.FeatureDim()], \
                                                        dtype=pynp.float32)
        cdef Float32 *feats_ptr = <Float32*>feats.data
        cdef Int32 stride = feats.strides[-1]
        self.extractor.Compute(<Float32*>wav.data, wav.size, <Float32*>feats.data, stride)
        return feats

    def __dealloc__(self):
        del self.extractor