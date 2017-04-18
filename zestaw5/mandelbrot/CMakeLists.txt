cmake_minimum_required(VERSION 3.0.2)
project(mandelbrort)

set(CMAKE_C_FLAGS "-g -Wall -std=c99")

set(MASTER_SOURCES master.c master_fun.c)
add_executable(master.out ${MASTER_SOURCES})

set(SLAVE_SOURCES slave.c slave_fun.c)
add_executable(slave.out ${SLAVE_SOURCES})
