cmake_minimum_required(VERSION 3.4)

set(DECODER_SRC ${CMAKE_SOURCE_DIR}/decoder/io.cc
                ${CMAKE_SOURCE_DIR}/decoder/fft-computer.cc
                ${CMAKE_SOURCE_DIR}/decoder/signal.cc
                ${CMAKE_SOURCE_DIR}/decoder/simple-fst.cc
                ${CMAKE_SOURCE_DIR}/decoder/wave.cc
                ${CMAKE_SOURCE_DIR}/decoder/math.cc
                ${CMAKE_SOURCE_DIR}/decoder/online.cc
                ${CMAKE_SOURCE_DIR}/decoder/decoder.cc)

add_library(${DECODER_LIB} SHARED ${DECODER_SRC})
target_link_libraries(${DECODER_LIB})
