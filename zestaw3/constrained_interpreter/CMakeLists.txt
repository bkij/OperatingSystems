cmake_minimum_required(VERSION 3.02)
project(constrained_interpreter)

set(CMAKE_C_FLAGS "-Wall -std=c99")

set(CONSTRAINED_INTERPRETER_SOURCES constrained_batch.c constrained_interpreter.c)

add_executable(constrained_interpreter.out ${CONSTRAINED_INTERPRETER_SOURCES})


set(STUPID_PROGRAM_SOURCES stupid_program.c)

add_executable(stupid_program.out ${STUPID_PROGRAM_SOURCES})