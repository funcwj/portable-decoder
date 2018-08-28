#!/usr/bin/env python

# wujian@2018
"""
Demo: decoding from loglikelihoods
"""
import os

import argparse
import numpy as np
import torch as th

from kaldi_helper.tdnn import TDNN
from pydecoder import UtteranceDecoder, PyFeatureExtractor
from utils import ScriptReader


def uttdecoder_demo(post_scp):
    # decoder
    decoder_kwargs = {
        "graph": "asset/graph.fst",
        "table": "asset/trans.tab",
        "decode_conf": "asset/decode.conf",
        "words": "asset/words.txt"
    }
    # decoder instance
    decoder = UtteranceDecoder(**decoder_kwargs)

    reader = ScriptReader(post_scp)
    for key, posts in reader:
        posts = posts.astype(np.float32)
        words = decoder.decode(posts)
        print("{} {}".format(key, words))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Command to decode from network's output using inner configures")
    parser.add_argument(
        "post_scp",
        type=str,
        help="Path of loglikelihoods(in .scp) for decoding")
    args = parser.parse_args()
    uttdecoder_demo(args.post_scp)