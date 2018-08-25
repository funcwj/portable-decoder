#!/usr/bin/env python

# wujian@2018

import os

import numpy as np

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

cur_dir = os.getcwd()

include_dirs = [
    "{}/../".format(cur_dir),
    np.get_include(),  # include for numpy
]

library_dirs = [
    "{}/../lib".format(cur_dir),
]

depend_libs = [
    "decoder",
]

complie_args = [
    "-std=c++11",  # important
]

extensions = Extension(
    "pydecoder",
    language="c++",
    extra_compile_args=complie_args,
    include_dirs=include_dirs,
    library_dirs=library_dirs,
    libraries=depend_libs,
    sources=["pydecoder.pyx"])

setup(
    name="simple decoder",
    version="0.1",
    license="Apache V2",
    keywords=["speech recognition", "decoder"],
    ext_modules=cythonize([extensions]),
    author="Jian Wu",
    author_email="funcwj@foxmail.com",
    description="Python wrappers for mini-asr-decoder",
)