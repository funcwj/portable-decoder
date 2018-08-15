#!/usr/bin/env python

# wujian@2018

import numpy as np
import scipy.io.wavfile as wf

from pydecoder import PyFeatureExtractor

def test_computor():
    computer = PyFeatureExtractor("mfcc.conf", "fbank")
    _, wave = wf.read("egs.wav")
    wave = wave.astype(np.float32)
    mfcc = computer.compute(wave)
    print("Shape of mfcc: {}".format(mfcc.shape))
    print(mfcc)

    
if __name__ == "__main__":
    test_computor()
