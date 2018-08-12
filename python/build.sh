#!/usr/bin/env bash

# wujian@2018

set -eu

python setup.py build_ext --inplace
# or will not find the library
cp ../lib/libdecoder.so .
python egs.py