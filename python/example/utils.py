#!/usr/bin/env python

# wujian@2018

import os
import logging
import warnings
import sys
import numpy as np
import scipy.io.wavfile as wf

import kaldi_helper.iobase as io

def parse_scps(scp_path, addr_processor=lambda x: x):
    assert os.path.exists(scp_path)
    scp_dict = dict()
    with open(scp_path, 'r') as f:
        for scp in f:
            scp_tokens = scp.strip().split()
            if len(scp_tokens) != 2:
                raise RuntimeError(
                    "Error format of context \'{}\'".format(scp))
            key, addr = scp_tokens
            if key in scp_dict:
                raise ValueError("Duplicate key \'{}\' exists!".format(key))
            scp_dict[key] = addr_processor(addr)
    return scp_dict


def filekey(path):
    fname = os.path.basename(path)
    if not fname:
        raise ValueError("{}(Is directory path?)".format(path))
    token = fname.split(".")
    if len(token) == 1:
        return token[0]
    else:
        return '.'.join(token[:-1])

def get_logger(
        name,
        format_str="%(asctime)s [%(pathname)s:%(lineno)s - %(levelname)s ] %(message)s",
        date_format="%Y-%m-%d %H:%M:%S"):
    logger = logging.getLogger(name)
    logger.setLevel(logging.INFO)
    handler = logging.StreamHandler()
    handler.setLevel(logging.INFO)
    formatter = logging.Formatter(fmt=format_str, datefmt=date_format)
    handler.setFormatter(formatter)
    logger.addHandler(handler)
    return logger


class Reader(object):
    """
        Base class for sequential/random accessing, to be implemented
    """

    def __init__(self, scp_path, addr_processor=lambda x: x):
        if not os.path.exists(scp_path):
            raise FileNotFoundError("Could not find file {}".format(scp_path))
        self.index_dict = parse_scps(scp_path, addr_processor=addr_processor)
        self.index_keys = [key for key in self.index_dict.keys()]

    def _load(self, key):
        raise NotImplementedError

    # number of utterance
    def __len__(self):
        return len(self.index_dict)

    # avoid key error
    def __contains__(self, key):
        return key in self.index_dict

    # sequential index
    def __iter__(self):
        for key in self.index_keys:
            yield key, self._load(key)

    # random index, support str/int as index
    def __getitem__(self, index):
        if type(index) == int:
            num_utts = len(self.index_keys)
            if index >= num_utts or index < 0:
                raise KeyError("Interger index out of range, {} vs {}".format(
                    index, num_utts))
            key = self.index_keys[index]
            return self._load(key)
        elif type(index) is str:
            if index not in self.index_dict:
                raise KeyError("Missing utterance {}!".format(index))
            return self._load(index)
        else:
            raise IndexError("Unsupported index type: {}".format(type(index)))

class Writer(object):
    """
        Base Writer class to be implemented
    """

    def __init__(self, ark_path, scp_path=None):
        if scp_path == '-':
            raise ValueError("Could not write .scp to stdout")
        self.scp_path = scp_path
        self.ark_path = ark_path
        if self.ark_path == '-' and self.scp_path:
            self.scp_path = None
            warnings.warn("Ignore .scp output discriptor")

    def __enter__(self):
        self.scp_file = None if self.scp_path is None else open(
            self.scp_path, "w")
        self.ark_file = sys.stdout if self.ark_path == '-' else open(
            self.ark_path, "wb")
        return self

    def __exit__(self, *args):
        if self.scp_file:
            self.scp_file.close()
        if self.ark_path != '-':
            self.ark_file.close()

    def write(self, key, data):
        raise NotImplementedError


class WaveReader(Reader):
    def __init__(self, scp_path, sample_rate=None):
        super(WaveReader, self).__init__(scp_path)
        self.sample_rate = sample_rate

    def _load(self, key):
        wav_addr = self.index_dict[key]
        _, samps = wf.read(wav_addr)
        return samps.astype(np.float32)


class ArchiveReader(object):
    """
        Sequential Reader for .ark object
    """
    def __init__(self, ark_path):
        if not os.path.exists(ark_path):
            raise FileNotFoundError("Could not find {}".format(ark_path))
        self.ark_path = ark_path
    
    def __iter__(self):
        with open(self.ark_path, "rb") as fd:
            for key, mat in io.read_ark(fd):
                yield key, mat

class ScriptReader(Reader):
    """
        Reader for kaldi's scripts(for BaseFloat matrix)
    """

    def __init__(self, ark_scp):
        def addr_processor(addr):
            addr_token = addr.split(":")
            if len(addr_token) == 1:
                raise ValueError("Unsupported scripts address format")
            path, offset = ":".join(addr_token[0:-1]), int(addr_token[-1])
            return (path, offset)

        super(ScriptReader, self).__init__(
            ark_scp, addr_processor=addr_processor)

    def _load(self, key):
        path, offset = self.index_dict[key]
        with open(path, 'rb') as f:
            f.seek(offset)
            io.expect_binary(f)
            ark = io.read_general_mat(f)
        return ark

class ArchiveWriter(Writer):
    """
        Writer for kaldi's scripts && archive(for BaseFloat matrix)
    """

    def __init__(self, ark_path, scp_path=None):
        super(ArchiveWriter, self).__init__(ark_path, scp_path)

    def write(self, key, matrix):
        io.write_token(self.ark_file, key)
        offset = self.ark_file.tell()
        io.write_binary_symbol(self.ark_file)
        io.write_common_mat(self.ark_file, matrix)
        abs_path = os.path.abspath(self.ark_path)
        if self.scp_file:
            self.scp_file.write("{}\t{}:{:d}\n".format(key, abs_path, offset))
