#!/usr/bin/env python

# wujian@2018
"""
Demo: offline decoding
"""
import os
import glob

import numpy as np
import torch as th
import scipy.io.wavfile as wf

from pydecoder import PyFeatureExtractor

from kaldi_raw_tdnn.tdnn import TDNN
from decoder import OfflineDecoder


def filekey(path):
    fname = os.path.basename(path)
    if not fname:
        raise ValueError("{}(Is directory path?)".format(path))
    token = fname.split(".")
    if len(token) == 1:
        return token[0]
    else:
        return '.'.join(token[:-1])


def decoder_demo():
    # decoder
    decoder_kwargs = {
        "graph": "asset/graph.fst",
        "table": "asset/trans.tab",
        "decode_conf": "asset/decode.conf",
        "words": "asset/words.txt"
    }
    tdnn_kwargs = {
        "tdnn_conf": "-2,-1,0,1,2;0;-1,0,2;-3,0,3;-7,0,2;-3,0,3;0;0",
        "param": "asset/tdnn.596.param",
        "feature_dim": 40
    }
    feature_kwargs = {"conf": "asset/fbank.conf", "type": "fbank"}

    if not os.path.exists("asset"):
        raise FileNotFoundError("Prepare assert directory first!")
    # feature extractor
    computer = PyFeatureExtractor(feature_kwargs["conf"],
                                  feature_kwargs["type"])
    # decoder instance
    decoder = OfflineDecoder(**decoder_kwargs)
    # network instance
    tdnn = TDNN(**tdnn_kwargs)

    for wav in glob.glob("asset/wav/*.wav"):
        # read wave in int16
        _, samp_int16 = wf.read(wav)
        samp_float32 = samp_int16.astype(np.float32)
        fbank = computer.compute(samp_float32)
        loglikes = tdnn.compute_output(fbank)
        words = decoder.decode(loglikes)
        print("{} {}".format(filekey(wav), words))


if __name__ == "__main__":
    decoder_demo()
