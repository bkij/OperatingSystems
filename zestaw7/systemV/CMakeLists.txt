cmake_minimum_required(VERSION 3.0.2)
project(sleepingbarber)

set(CMAKE_C_FLAGS "-g -Wall -std=c99")

set(BARBER_SRC sleepingbarber.c util.c barber_fun.c)
add_executable(sleepingbarber.out ${BARBER_SRC})

set(CLIENT_CREATOR_SRC client_creator.c util.c)
add_executable(client_creator.out ${CLIENT_CREATOR_SRC})
