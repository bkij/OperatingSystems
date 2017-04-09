cmake_minimum_required(VERSION 3.0.2)
project(zad3)

set(CMAKE_C_FLAGS "-Wall -std=c99")

set(SOURCES zad3.c)
add_executable(zad3.out ${SOURCES})

set(CHLD_SOURCES zad3_child.c)
add_executable(zad3_child.out ${CHLD_SOURCES})