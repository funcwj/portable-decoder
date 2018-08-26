#!/usr/bin/env python

# wujian@2018
"""
Demo: validate inference outputs
"""

import torch as th

from kaldi_raw_tdnn.tdnn import TDNN
from utils import ArchiveReader, ArchiveWriter

def run_egs():
    tdnn_kwargs = {
        "tdnn_conf": "-2,-1,0,1,2;0;-1,0,2;-3,0,3;-7,0,2;-3,0,3;0;0",
        "param": "asset/tdnn.596.param",
        "feature_dim": 40
    }
    tdnn = TDNN(**tdnn_kwargs)
    feats_reader = ArchiveReader("asset/feats.ark")
    
    with ArchiveWriter("asset/feats.ext") as writer:
        for key, mat in feats_reader:
            print("Runing for {}...".format(key))
            output = tdnn.compute_output(mat)
            writer.write(key, output)

if __name__ == "__main__":
    run_egs()
