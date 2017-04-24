cmake_minimum_required(VERSION 3.0.2)
project(sysV_mq)

set(CMAKE_C_FLAGS "-g -Wall -std=c99")

set(MQ_SRV_SOURCES server.c mq_server.c util.c)
add_executable(server.out ${MQ_SRV_SOURCES})
target_link_libraries(server.out -lrt)

set(MQ_CLI_SOURCES client.c mq_client.c util.c)
add_executable(client.out ${MQ_CLI_SOURCES})
target_link_libraries(client.out -lrt)