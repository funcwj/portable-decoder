#!/usr/bin/env python

# wujian@2018
"""
Demo: validate inference outputs
"""

import torch as th

from kaldi_raw_tdnn.iobase import read_ark
from kaldi_raw_tdnn.iobase import write_token, write_binary_symbol, write_common_mat
from kaldi_raw_tdnn.tdnn import TDNN

def write_ark(fd, key, mat):
    write_token(fd, key)
    write_binary_symbol(fd)
    write_common_mat(fd, mat)

def run_egs():
    tdnn_kwargs = {
        "tdnn_conf": "-2,-1,0,1,2;0;-1,0,2;-3,0,3;-7,0,2;-3,0,3;0;0",
        "param": "asset/tdnn.596.param",
        "feature_dim": 40
    }
    tdnn = TDNN(**tdnn_kwargs)

    with open("asset/feats.ark", "rb") as ark, \
            open("asset/posts.hyp.ark", "wb") as dst:
        for key, mat in read_ark(ark):
            print("Runing for {}...".format(key))
            output = tdnn.compute_output(mat)
            write_ark(dst, key, output)

if __name__ == "__main__":
    run_egs()
