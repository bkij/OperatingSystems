cmake_minimum_required(VERSION 3.0.2)
project(children_creator)

set(CMAKE_C_FLAGS "-Wall -std=c99")

set(PARENT_SOURCES children_creator.c)
add_executable(children_creator.out ${PARENT_SOURCES})

set(CHILD_SOURCES child.c)
add_executable(child.out ${CHILD_SOURCES})