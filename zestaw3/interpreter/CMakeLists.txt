cmake_minimum_required(VERSION 3.02)
project(interpreter)

set(CMAKE_C_FLAGS "-Wall -std=c99")

set(INTERPRETER_SOURCES batch.c interpreter.c)

add_executable(interpreter.out ${INTERPRETER_SOURCES})


set(ENVREADER_SOURCES env_reader.c)

add_executable(env_reader.out ${ENVREADER_SOURCES})