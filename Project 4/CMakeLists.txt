cmake_minimum_required(VERSION 3.5)
project(code)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
inputbuf.cc
    lexer.cc
    )

add_executable(code ${SOURCE_FILES})