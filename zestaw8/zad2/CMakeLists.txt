cmake_minimum_required(VERSION 3.0.2)
project(reader)

set(CMAKE_C_FLAGS "-g -Wall -std=c99 -pthread")

set(READER_SRC reader.c reader_fun.c util.c)
add_executable(reader.out ${READER_SRC})
target_link_libraries(reader.out pthread)
