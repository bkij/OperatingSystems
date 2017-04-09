cmake_minimum_required(VERSION 3.0.2)
project(alphabet_writer)

set(CMAKE_C_FLAGS "-Wall -std=c99")

set(SOURCES alphabet_writer.c)

add_executable(alphabet_writer.out ${SOURCES})