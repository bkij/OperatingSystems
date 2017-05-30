cmake_minimum_required(VERSION 3.0.2)
project(zestaw10)

set(CMAKE_C_FLAGS "-Wall -std=gnu99 -g")

set(SERVER_SRC
        zad1/server/server.c zad1/common/common.c zad1/server/server.h zad1/common/common.h zad1/server/threads/server_threads.c zad1/server/threads/server_threads.h)
set(CLIENT_SRC
        zad1/client/client.c zad1/common/common.c zad1/client/client.h zad1/common/common.h)
add_executable(server.out ${SERVER_SRC})
target_link_libraries(server.out pthread)
add_executable(client.out ${CLIENT_SRC})
target_link_libraries(client.out pthread)

set(DG_SERVER_SRC zad2/dg_server/dg_server.c zad2/common/common.c zad2/dg_server/threading/threading.c zad2/dg_server/threading/threading.h zad2/dg_server/sockets/sockets.c zad2/dg_server/sockets/sockets.h)
set(DG_CLIENT_SRC zad2/dg_client/dg_client.c zad2/common/common.c)
add_executable(dg_server.out ${DG_SERVER_SRC})
target_link_libraries(dg_server.out pthread)
add_executable(dg_client.out ${DG_CLIENT_SRC})
target_link_libraries(dg_client.out pthread)