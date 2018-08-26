#!/usr/bin/env python

# wujian@2018
"""
Demo: decoding from raw waveform
"""
import os

import argparse
import numpy as np
import torch as th

from pydecoder import PyFeatureExtractor, UtteranceDecoder

from kaldi_helper.tdnn import TDNN
from utils import WaveReader


def uttdecoder_demo(egs_scp):
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
    decoder = UtteranceDecoder(**decoder_kwargs)
    # network instance
    tdnn = TDNN(**tdnn_kwargs)

    reader = WaveReader(egs_scp)
    for key, samps in reader:
        computer.reset()  # clear cached samples
        fbank = computer.compute(samps)
        loglikes = tdnn.compute_output(fbank)
        words = decoder.decode(loglikes)
        print("{} {}".format(key, words))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Command to decode from raw waveform using inner configures")
    parser.add_argument(
        "wav_scp", type=str, help="Path of wav.scp for decoding")
    args = parser.parse_args()
    uttdecoder_demo(args.wav_scp)
