cmake_minimum_required(VERSION 3.4)
project("Command tools for simple decoder based on Kaldi")

set(CMAKE_CXX_STANDARD 11)

MESSAGE(STATUS "detected OS: ${CMAKE_SYSTEM}")

set(KALDI_ROOT $ENV{KALDI_ROOT})
if(NOT DEFINED KALDI_ROOT)
    message(FATAL_ERROR "Can not find definition of KALDI_ROOT, export KALDI_ROOT=/path/to/kaldi first")
endif()

message(STATUS "defined KALDI_ROOT=" $ENV{KALDI_ROOT})
set(DEPEND_LIBS kaldi-fstext kaldi-base kaldi-util)

add_definitions(-O3 -g -std=c++11)
include_directories(${KALDI_ROOT}/src ${KALDI_ROOT}/tools/openfst/include)
link_directories(${KALDI_ROOT}/src/lib)

add_executable(copy-decode-graph copy-decode-graph.cc)
target_link_libraries(copy-decode-graph ${DEPEND_LIBS})

