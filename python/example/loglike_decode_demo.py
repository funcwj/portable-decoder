#!/usr/bin/env python

# wujian@2018
"""
Demo: decoding from log-likelihoods

For TDNN models trained on aishell:
input dim=40 name=input
fixed-affine-layer name=lda input=Append(-2,-1,0,1,2) affine-transform-file=exp/nnet3/tdnn_sp/configs/lda.mat
relu-batchnorm-layer name=tdnn1 dim=850
relu-batchnorm-layer name=tdnn2 dim=850 input=Append(-1,0,2)
relu-batchnorm-layer name=tdnn3 dim=850 input=Append(-3,0,3)
relu-batchnorm-layer name=tdnn4 dim=850 input=Append(-7,0,2)
relu-batchnorm-layer name=tdnn5 dim=850 input=Append(-3,0,3)
relu-batchnorm-layer name=tdnn6 dim=850
output-layer name=output input=tdnn6 dim=2960 max-change=1.5

This command get WER 19.23% vs 19.00%, with decode configure:
--DecodeOpts.min_active=200
--DecodeOpts.max_active=7000
--DecodeOpts.beam=15.0
--DecodeOpts.acwt=0.06
--DecodeOpts.penalty=0.5

and fbank configure(without cmvn):
--FbankOpts.num_mel_bins=40
--FrameOpts.window=hamming
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