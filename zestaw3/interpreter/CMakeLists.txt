cmake_minimum_required(VERSION 3.02)
project(interpreter)

set(CMAKE_C_FLAGS "-Wall -std=c99")

set(SOURCE_FILES batch.c interpreter.c)

add_executable(interpreter.out ${SOURCE_FILES})