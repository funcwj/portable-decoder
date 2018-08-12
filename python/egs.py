#!/usr/bin/env python

# wujian@2018

from pydecoder import PyFrameSplitter

def test_opts():
    splitter = PyFrameSplitter()
    print(splitter.generate_config())

if __name__ == "__main__":
    test_opts()
