#!/usr/bin/env python

# wujian@2018
"""
Demo: validate TDNN inference outputs, work with configure:
input dim=40 name=input
fixed-affine-layer name=lda input=Append(-2,-1,0,1,2) affine-transform-file=exp/nnet3/tdnn_sp/configs/lda.mat
relu-batchnorm-layer name=tdnn1 dim=850
relu-batchnorm-layer name=tdnn2 dim=850 input=Append(-1,0,2)
relu-batchnorm-layer name=tdnn3 dim=850 input=Append(-3,0,3)
relu-batchnorm-layer name=tdnn4 dim=850 input=Append(-7,0,2)
relu-batchnorm-layer name=tdnn5 dim=850 input=Append(-3,0,3)
relu-batchnorm-layer name=tdnn6 dim=850
output-layer name=output input=tdnn6 dim=2960 max-change=1.5
"""

import torch as th

from kaldi_helper.tdnn import TDNN
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
