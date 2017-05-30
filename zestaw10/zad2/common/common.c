//
// Created by kveld on 30.05.17.
//

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

struct client_info *init_clients()
{
    struct client_info *clients = malloc(sizeof(struct client_info) * MAX_CLIENTS);
    for(int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].id = -1;
    }
    return clients;
}

pthread_mutex_t *init_mutex()
{
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    if(pthread_mutex_init(mutex, NULL) < 0) {
        fprintf(stderr, "Mutex err at function: %s: %s\n", "init_mutex()", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return mutex;
}