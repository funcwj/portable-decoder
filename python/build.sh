# wujian@2018

set -eu

rm -rf *.so
python setup.py build_ext --inplace
# or will not find the library
# cp ../lib/libdecoder.* .
# fix rpath on macbook
install_name_tool -add_rpath ../lib pydecoder.cpython-36m-darwin.so
# python egs.py
