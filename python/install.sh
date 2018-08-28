#!/usr/bin/env bash
# wujian@2018

set -eu

rm -rf build *.so *.cpp
python setup.py install
# for library not found problem:
# 1): on Linux, egs ubuntu
# export LD_LIBRARY_PATH
# 2): on OsX
# install_dir=$(pip show pydecoder | grep Location | awk '{print $2}')
# install_name_tool -add_rpath ../lib $install_dir/_pydecoder.cpython-36m-darwin.so
cp ../lib/libdecoder.* /usr/local/lib/
rm -rf build *.so *.cpp
