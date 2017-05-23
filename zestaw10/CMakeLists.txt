cmake_minimum_required(VERSION 3.7)
project(zestaw10)

set(CMAKE_C_FLAGS "-Wall -std=c99 -g")

set(SERVER_SRC
        zad1/server/server.c)
set(CLIENT_SRC
        zad1/client/client.c)
add_executable(server.out ${SERVER_SRC})
add_executable(client.out ${CLIENT_SRC})