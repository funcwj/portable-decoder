#!/usr/bin/env python

# wujian@2018
"""
Demo: decoding from log-likelihoods
"""
import os

import argparse
import numpy as np
import torch as th

from kaldi_helper.tdnn import TDNN
from pydecoder import LogLikeDecoder, PyFeatureExtractor
from utils import ScriptReader


def uttdecoder_demo(loglike_scp):
    # decoder
    decoder_kwargs = {
        "graph": "asset/graph.fst",
        "table": "asset/trans.tab",
        "decode_conf": "asset/decode.conf",
        "words": "asset/words.txt"
    }
    # decoder instance
    decoder = LogLikeDecoder(**decoder_kwargs)
    reader = ScriptReader(loglike_scp)
    for key, loglikes in reader:
        loglikes = loglikes.astype(np.float32)
        words = decoder.decode(loglikes)
        print("{} {}".format(key, words))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=
        "Command to decode from network's output(usually log-likelihoods) using inner configures"
    )
    parser.add_argument(
        "loglike_scp",
        type=str,
        help="Path of log likelihoods(in .scp) for decoding")
    args = parser.parse_args()
    uttdecoder_demo(args.loglike_scp)