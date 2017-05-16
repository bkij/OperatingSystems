cmake_minimum_required(VERSION 3.0.2)
project(sleepingbarber)

set(CMAKE_C_FLAGS "-g -Wall -std=c99 -pthread")

set(BARBER_SRC sleepingbarber.c util.c barber_fun.c shared.c)
add_executable(sleepingbarber.out ${BARBER_SRC})
target_link_libraries(sleepingbarber.out pthread rt)

set(CLIENT_CREATOR_SRC client_creator.c util.c)
add_executable(client_creator.out ${CLIENT_CREATOR_SRC})
target_link_libraries(client_creator.out pthread rt)

set(CLIENT_SRC client.c client_fun.c shared.c util.c)
add_executable(client.out ${CLIENT_SRC})
target_link_libraries(client.out pthread rt)
