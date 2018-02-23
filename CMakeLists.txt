cmake_minimum_required(VERSION 3.9)
project(Linux)

set(CMAKE_CXX_STANDARD 11)

set(HEADER_FILE type.h)
set(SOURCE_FILE main.cpp)

add_executable(Linux ${HEADER_FILE} ${SOURCE_FILE})