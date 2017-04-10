cmake_minimum_required(VERSION 3.0.2)
project(pipe_interpreter)

set(CMAKE_C_FLAGS "-g -Wall -std=c99")

set(SOURCES pipe_interpreter.c pipe.c)

add_executable(pipe_interpreter.out ${SOURCES})
