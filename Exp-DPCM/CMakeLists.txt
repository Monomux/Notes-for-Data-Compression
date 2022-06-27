cmake_minimum_required(VERSION 3.21)
project(Exp2)

set(CMAKE_CXX_STANDARD 14)

aux_source_directory(. DIRSRCS)
include_directories(.)
file(COPY Lena.bmp Cameraman.bmp Goldhill.bmp DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_executable(Exp2 ${DIRSRCS})
