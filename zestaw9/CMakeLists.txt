cmake_minimum_required(VERSION 3.0.2)
project(zestaw9)

set(CMAKE_C_FLAGS "-g -Wall -std=c99 -pthread")

set(MUTEX_SRC mutexes/mutex_rw.c mutexes/execution/execution.c mutexes/readers/reader.c mutexes/readers/reader.h mutexes/writers/writer.c mutexes/writers/writer.h mutexes/commons/commons.c)
add_executable(mutex_rw.out ${MUTEX_SRC})
target_link_libraries(mutex_rw.out pthread)

set(SEMAPHORE_SRC semaphores/semaphore_rw.c semaphores/sem_commons/sem_commons.c semaphores/sem_commons/sem_commons.h semaphores/sem_execution/sem_execution.c semaphores/sem_execution/sem_execution.h semaphores/sem_readers/sem_readers.c semaphores/sem_readers/sem_readers.h semaphores/sem_writers/sem_writers.c semaphores/sem_writers/sem_writers.h)
add_executable(semaphore_rw.out ${SEMAPHORE_SRC})
target_link_libraries(semaphore_rw.out pthread)