//
// Created by kveld on 18.05.17.
//

#ifndef ZESTAW9_COMMONS_H
#define ZESTAW9_COMMONS_H

#include <pthread.h>
#include <stdbool.h>

#define SHARED_ARR_LEN 256

#define ERRNO_EXIT(S) do {perror(S); exit(EXIT_FAILURE);} while(0)

struct thread_arg {
    int *shared_array;
    bool verbose;
    int readers_active;
    int writers_waiting;
    pthread_cond_t *no_writers_waiting;
    pthread_cond_t *no_readers_active;
    pthread_mutex_t *cond_mutex;
    pthread_mutex_t *writer_mutex;
};

void take_mutex(pthread_mutex_t *mutex);
void give_mutex(pthread_mutex_t *mutex);

#endif //ZESTAW9_COMMONS_H
