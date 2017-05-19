//
// Created by kveld on 18.05.17.
//

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <malloc.h>
#include "../commons/commons.h"
#include "../readers/reader.h"
#include "../writers/writer.h"
#include "execution.h"

void execute_threads(int *shared_array, int num_readers, int num_writers, bool verbose)
{
    pthread_t *reader_threads = malloc(num_readers * sizeof(pthread_t));
    pthread_t *writer_threads = malloc(num_writers * sizeof(pthread_t));
    /*
     * The thread arguments may be safely stack allocated
     * because we're waiting for the threads to finish
     */
    struct thread_arg arg;
    init_thread_arg(&arg, shared_array, verbose);
    init_rng();
    for(int i = 0; i < num_readers; i++) {
        if(pthread_create(&reader_threads[i], NULL, read_array, (void *)&arg) < 0) {
            ERRNO_EXIT("Error creating thread");
        }
    }
    for(int i = 0; i < num_writers; i++) {
        if(pthread_create(&writer_threads[i], NULL, write_to_array, (void *)&arg) < 0) {
            ERRNO_EXIT("Error creating thread");
        }
    }
    for(int i = 0; i < num_readers; i++) {
        if(pthread_join(reader_threads[i], NULL) < 0) {
            ERRNO_EXIT("Error joining a reader thread");
        }
    }
    for(int i = 0; i < num_writers; i++) {
        if(pthread_join(writer_threads[i], NULL) < 0) {
            ERRNO_EXIT("Error joining a writer thread");
        }
    }
}

void init_thread_arg(struct thread_arg *arg, int *shared_array, bool verbose)
{
    arg->shared_array = shared_array;
    arg->verbose = verbose;
    arg->no_writers_waiting = malloc(sizeof(pthread_cond_t));
    if(pthread_cond_init(arg->no_writers_waiting, NULL) < 0) {
        ERRNO_EXIT("Couldnt initialize condition variable");
    }
    arg->no_readers_active = malloc(sizeof(pthread_cond_t));
    if(pthread_cond_init(arg->no_readers_active, NULL) < 0) {
        ERRNO_EXIT("Couldnt initiazlize condition variable");
    }
    arg->readers_active = 0;
    arg->writers_waiting = 0;
    arg->cond_mutex = malloc(sizeof(pthread_mutex_t));
    if(pthread_mutex_init(arg->cond_mutex, NULL) < 0) {
        ERRNO_EXIT("Couldnt initialize no_writers_waiting");
    }
    arg->writer_mutex = malloc(sizeof(pthread_mutex_t));
    if(pthread_mutex_init(arg->writer_mutex, NULL) < 0) {
        ERRNO_EXIT("Error initializing writer mutex");
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
