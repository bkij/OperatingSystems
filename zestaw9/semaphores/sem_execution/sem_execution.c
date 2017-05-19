//
// Created by kveld on 19.05.17.
//

#include <pthread.h>
#include <malloc.h>
#include <stdlib.h>
#include <semaphore.h>
#include "sem_execution.h"
#include "../sem_commons/sem_commons.h"
#include "../sem_writers/sem_writers.h"
#include "../sem_readers/sem_readers.h"

void execute_threads(int *shared_array, int num_readers, int num_writers, bool verbose)
{
    pthread_t *readers = malloc(num_readers * sizeof(pthread_t));
    pthread_t *writers = malloc(num_writers * sizeof(pthread_t));
    struct thread_arg info;
    init_thread_arg(&info, shared_array, verbose);
    init_rng();
    for(int i = 0; i < num_readers; i++) {
        if(pthread_create(&readers[i], NULL, read_array, (void *)&info) < 0) {
            ERRNO_EXIT("Couldnt create reader thread");
        }
    }
    for(int i = 0; i < num_writers; i++) {
        if(pthread_create(&writers[i], NULL, write_to_array, (void *)&info) < 0) {
            ERRNO_EXIT("Couldnt create writer thread");
        }
    }
    for(int i = 0; i < num_readers; i++) {
        if(pthread_join(readers[i], NULL) < 0) {
            ERRNO_EXIT("Couldnt join a reader thread");
        }
    }
    for(int i = 0; i < num_writers; i++) {
        if(pthread_join(writers[i], NULL) < 0) {
            ERRNO_EXIT("Couldnt join a writer thread");
        }
    }
}

void init_thread_arg(struct thread_arg *arg, int *shared_array, bool verbose)
{
    arg->shared_array = shared_array;
    arg->verbose = verbose;
    arg->writer_sem = malloc(sizeof(sem_t));
    if(sem_init(arg->writer_sem, 0, 1) < 0) {
        ERRNO_EXIT("Couldnt initialize writer semaphore");
    }
    arg->reader_sem = malloc(sizeof(sem_t));
    if(sem_init(arg->reader_sem, 0, 0) < 0) {
        ERRNO_EXIT("Couldnt initialize reader semaphore");
    }
    arg->rw_sem = malloc(sizeof(sem_t));
    if(sem_init(arg->rw_sem, 0, 1) < 0) {
        ERRNO_EXIT("Couldnt initialize rw semaphore");
    }
}

void init_rng()
{
    char buffer[4];
    FILE *urandom = fopen("/dev/urandom", "r");
    if(urandom == NULL) {
        ERRNO_EXIT("Couldnt open urandom");
    }
    if(fread(buffer, sizeof(buffer), 1, urandom) == 0 && ferror(urandom)) {
        ERRNO_EXIT("Couldnt initialize random data");
    }
    srand(*(unsigned int *)&buffer);
}