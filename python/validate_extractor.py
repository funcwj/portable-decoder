#!/usr/bin/env python

# wujian@2018

import os

import numpy as np
import torch as th

from pydecoder import PyFeatureExtractor
from utils import WaveReader, ArchiveWriter

def run_egs():
    feature_kwargs = {"conf": "asset/fbank.conf", "type": "fbank"}

    if not os.path.exists("asset"):
        raise FileNotFoundError("Prepare assert directory first!")
    # feature extractor
    computer = PyFeatureExtractor(feature_kwargs["conf"],
                                  feature_kwargs["type"])
    reader = WaveReader("asset/egs.scp")

    with ArchiveWriter("asset/feats.ext") as writer:
        for key, samps in reader:
            computer.reset()    # clear cached samples
            fbank = computer.compute(samps)
            writer.write(key, fbank)

if __name__ == "__main__":
    run_egs()
