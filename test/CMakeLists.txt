cmake_minimum_required(VERSION 3.4)

include_directories(${CMAKE_SOURCE_DIR}/third_party/eigen)

add_executable(test-fft-computer test-fft-computer.cc)
add_executable(test-io test-io.cc)
add_executable(test-wave test-wave.cc)
add_executable(test-logger test-logger.cc)
add_executable(test-simple-fst test-simple-fst.cc)
add_executable(test-feature test-feature.cc)
add_executable(test-transition-table test-transition-table.cc)

target_link_libraries(test-fft-computer ${DECODER_LIB})
target_link_libraries(test-io ${DECODER_LIB})
target_link_libraries(test-wave ${DECODER_LIB})
target_link_libraries(test-logger ${DECODER_LIB})
target_link_libraries(test-simple-fst ${DECODER_LIB})
target_link_libraries(test-feature ${DECODER_LIB})
target_link_libraries(test-transition-table ${DECODER_LIB})