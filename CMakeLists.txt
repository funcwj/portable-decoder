cmake_minimum_required(VERSION 3.4)
project("Simple decoder for ASR")

set(CMAKE_CXX_STANDARD 11)

MESSAGE(STATUS "Detect OS: ${CMAKE_SYSTEM}")

add_definitions(-O3 -g -std=c++11)
include_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_SOURCE_DIR}/lib)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
set(DECODER_LIB decoder)

add_subdirectory(decoder)
add_subdirectory(test)
add_subdirectory(kaldi-tools)
