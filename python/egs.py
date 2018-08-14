#!/usr/bin/env python

# wujian@2018

import numpy as np
from pydecoder import PyMfccComputer

def test_computor():
    computer = PyMfccComputer("mfcc.conf")
    wave = np.random.rand(1000).astype(np.float32)
    mfcc = computer.compute(wave)
    print("Mfcc dimentions: {}".format(mfcc.shape))
    print("Mfcc dimentions:\n")
    print(mfcc)

    
if __name__ == "__main__":
    test_computor()
