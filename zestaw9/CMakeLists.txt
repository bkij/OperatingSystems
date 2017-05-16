cmake_minimum_required(VERSION 3.0.2)
project(zestaw9)

set(CMAKE_C_FLAGS "-g -Wall -std=c99 -pthread")

set(MUTEX_SRC mutex_rw.c)
add_executable(mutex_rw.out ${MUTEX_SRC})
target_link_libraries(mutex_rw.out pthread)

set(SEMAPHORE_SRC semaphore_rw.c)
add_executable(semaphore_rw.out ${SEMAPHORE_SRC})
target_link_libraries(semaphore_rw.out pthread)