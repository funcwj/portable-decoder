# wujian@2018

"""
A simple example to show the way to use Cython
"""

import numpy as np
cimport numpy as cnp

from typedef cimport Int32, Float32


cdef extern from "decoder/fft-computer.h":
    cdef cppclass FFTComputer:
        FFTComputer(Int32 register_size)
        void RealFFT(Float32 *real_values, Int32 num_samples)
    
ctypedef cnp.float_t NpFloat32

cdef class PyRealFFT:
    cdef FFTComputer *computer
    cdef Int32 N

    def __cinit__(self, N):
        self.computer = new FFTComputer(N)
        self.N = N

    def transform(self, mat):
        dim = mat.ndim
        if dim != 1:
            raise TypeError("Expect input as 1D matrix, get dim = {}".format(dim))
        if mat.dtype != np.float32:
            mat = mat.astype(np.float32)
        cdef cnp.ndarray c = np.array(mat)
        cdef Float32 *data_ptr = <Float32*>c.data
        self.computer.RealFFT(data_ptr, mat.shape[-1])
        return c

    def __dealloc__(self):
        del self.computer

"""
#!/usr/bin/env python
# wujian@2018

# Test scripts for fft.pyx

import numpy as np 

from pydecoder import PyRealFFT

def test_fft():
    N = 8
    computer = PyRealFFT(N)
    for e in range(10):
        mat = np.random.rand(N)
        print("OUT-Cxx: {}".format(computer.transform(mat)))
        print("OUT-Pyt: {}".format(np.fft.rfft(mat)))

if __name__ == "__main__":
    test_fft()
"""