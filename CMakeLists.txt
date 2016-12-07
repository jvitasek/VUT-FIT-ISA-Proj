cmake_minimum_required(VERSION 3.6)
project(ISA)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp imap.cpp error.cpp input_parser.cpp imaps.cpp imaps.h)
add_executable(ISA ${SOURCE_FILES})