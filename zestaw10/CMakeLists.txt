cmake_minimum_required(VERSION 3.7)
project(zestaw10)

set(CMAKE_C_FLAGS "-Wall -std=c99 -g")

set(SOURCE_FILES
        CMakeLists.txt zad1/server/server.c zad1/server/server.h zad1/client/client.c zad1/client/client.h zad2/dg_server/dg_server.c zad2/dg_server/dg_server.h zad2/dg_client/dg_client.c zad2/dg_client/dg_client.h)

add_executable(zestaw10 ${SOURCE_FILES})