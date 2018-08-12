# wujian@2018


cimport pydecoder

from typedef cimport kHamm, kHann, kRect, kBlackMan
from libcpp.string cimport string

supported_window = {
    "hamming": kHamm,
    "hanning": kHann,
    "rectangle": kRect,
    "blackman": kBlackMan
}

cdef class PyFrameSplitter:
    cdef pydecoder.FrameOpts frame_opts

    def __cinit__(self, frame_length=400,
                        frame_shift=160,
                        sample_rate=16000, 
                        preemph_coeff=0.97,
                        window="hamming",
                        remove_dc=True):
        if window not in supported_window:
            raise ValueError("Unsupported window type: {}".format(window))
        self.frame_opts = FrameOpts(frame_length, 
                                    frame_shift, 
                                    sample_rate, 
                                    preemph_coeff, 
                                    supported_window[window], 
                                    remove_dc)        
        
    def generate_config(self):
        cdef string config = self.frame_opts.Configure()
        py_bytes = <bytes>config
        return py_bytes.decode()

    

