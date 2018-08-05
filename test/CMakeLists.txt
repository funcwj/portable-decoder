cmake_minimum_required(VERSION 3.4)


add_executable(test-fft-computer test-fft-computer.cc)
add_executable(test-io test-io.cc)
add_executable(test-wave test-wave.cc)
add_executable(test-logger test-logger.cc)
add_executable(test-simple-fst test-simple-fst.cc)

target_link_libraries(test-fft-computer ${DECODER_LIB})
target_link_libraries(test-io ${DECODER_LIB})
target_link_libraries(test-wave ${DECODER_LIB})
target_link_libraries(test-logger ${DECODER_LIB})
target_link_libraries(test-simple-fst ${DECODER_LIB})