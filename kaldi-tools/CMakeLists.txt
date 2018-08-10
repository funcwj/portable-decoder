cmake_minimum_required(VERSION 3.4)
project("Command tools for simple decoder based on Kaldi")

set(CMAKE_CXX_STANDARD 11)

# MESSAGE(STATUS "Detected OS: ${CMAKE_SYSTEM}")

set(KALDI_ROOT $ENV{KALDI_ROOT})
if(NOT DEFINED KALDI_ROOT)
    message(FATAL_ERROR "Can not find definition of KALDI_ROOT, export KALDI_ROOT=/path/to/kaldi first")
endif()

message(STATUS "Define KALDI_ROOT=" $ENV{KALDI_ROOT})
set(DEPEND_LIBS kaldi-fstext kaldi-base kaldi-util kaldi-hmm kaldi-matrix)

add_definitions(-O3 -g -std=c++11)
include_directories(${KALDI_ROOT}/src ${KALDI_ROOT}/tools/openfst/include)
link_directories(${KALDI_ROOT}/src/lib)

add_executable(copy-decode-graph copy-decode-graph.cc)
add_executable(copy-transition-table copy-transition-table.cc)
target_link_libraries(copy-decode-graph ${DEPEND_LIBS})
target_link_libraries(copy-transition-table ${DEPEND_LIBS})

