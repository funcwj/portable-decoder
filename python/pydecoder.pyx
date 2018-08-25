# wujian@2018


cimport pydecoder
cimport numpy as np 
import numpy as pynp

from libcpp.string cimport string
from libcpp.vector cimport vector

ctypedef np.float32_t F32

def to_cstr(pystr):
    return <string>pystr.encode("utf-8")

cdef class PyFeatureExtractor:
    cdef pydecoder.FeatureExtractor *extractor 

    def __cinit__(self, conf, feat):
        cdef string conf_str = to_cstr(conf), feat_str = to_cstr(feat)
        self.extractor = new FeatureExtractor(conf_str, feat_str)

    def __dealloc__(self):
        del self.extractor

    def compute(self, np.ndarray[F32, ndim=1] wav):
        cdef Int32 num_frames = self.extractor.NumFrames(wav.size)
        cdef np.ndarray[F32, ndim=2] feats = pynp.zeros([num_frames, \
                                                        self.extractor.FeatureDim()], \
                                                        dtype=pynp.float32)
        cdef Float32 *feats_ptr = <Float32*>feats.data
        cdef Int32 stride = feats.strides[-1]
        self.extractor.Compute(<Float32*>wav.data, wav.size, <Float32*>feats.data, stride)
        return feats

cdef class PyDecoder:
    cdef pydecoder.FasterDecoder *decoder 
    cdef vector[Int32] word_seq

    def __cinit__(self, fst, tab, opt_conf):
        cdef string fst_str = to_cstr(fst), tab_str = to_cstr(tab)
        cdef string conf_str = to_cstr(opt_conf)
        self.decoder = new FasterDecoder(fst_str, tab_str, conf_str)
        self.decoder.Reset()

    def __dealloc__(self):
        del self.decoder

    def reset(self):
        self.decoder.Reset()

    def decode(self, np.ndarray[F32, ndim=2] loglikes):
        cdef Int32 stride = loglikes.strides[-1]
        cdef Int32 num_frames = loglikes.shape[0], num_pdfs = loglikes.shape[1]
        cdef Float32 *data_ptr = <Float32*>loglikes.data
        # print("LogLikelihoods: {:d} x {:d}, stride = {:d}".format(num_frames, num_pdfs, stride))
        self.decoder.Decode(data_ptr, num_frames, stride, num_pdfs)

    def best_sequence(self):
        self.decoder.GetBestPath(&self.word_seq)
        best_seq =  [id for id in self.word_seq]
        self.word_seq.clear()
        return best_seq

