# wujian@2018

set -eu

rm -rf build *.so *.cpp
python setup.py install
# for library not found problem:
# 1): on Linux, egs ubuntu
# cp ../lib/libdecoder.* .
# 2): on OsX
# install_name_tool -add_rpath ../lib _pydecoder.cpython-36m-darwin.so
install_dir=$(pip show pydecoder | grep Location | awk '{print $2}')
cp ../lib/libdecoder.* $install_dir
rm -rf build *.so *.cpp
