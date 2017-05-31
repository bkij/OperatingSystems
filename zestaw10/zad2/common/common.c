//
// Created by kveld on 30.05.17.
//

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"

struct client_info *init_clients()
{
    struct client_info *clients = malloc(sizeof(struct client_info) * MAX_CLIENTS);
    for(int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].id = -1;
    }
    return clients;
}

void init_pipe(int io_pipe[2])
{
    if(pipe(io_pipe) < 0) {
        fprintf(stderr, "Pipe error at func: %s - %s\n", "init_pipe", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void acquire(pthread_mutex_t *mutex) {
    if(pthread_mutex_lock(mutex) < 0) {
        fprintf(stderr, "Mutex error at function: %s: %s\n", "acquire", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void release(pthread_mutex_t *mutex) {
    if(pthread_mutex_unlock(mutex) < 0) {
        fprintf(stderr, "Mutex error at function: %s: %s\n", "release", strerror(errno));
        exit(EXIT_FAILURE);
    }
}