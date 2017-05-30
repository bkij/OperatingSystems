//
// Created by kveld on 30.05.17.
//

#ifndef ZESTAW10_COMMON_H
#define ZESTAW10_COMMON_H

#include <pthread.h>
#include <malloc.h>

#define PORT_STR_LEN 6
#define UNIX_PATH_MAX 108

#define WORKER_THREADS_NUM 2

#define CLIENT_NAME_LEN 32
#define MAX_CLIENTS 16

struct client_info {
    int id;
    char client_name[CLIENT_NAME_LEN + 1];
};

struct thread_shared_data {
    int local_sockfd;
    int remote_sockfd;
    pthread_mutex_t *cli_mutex;
    pthread_mutex_t *msg_mutex;
    struct client_info *clients;
};


pthread_mutex_t *init_mutex();
struct client_info *init_clients();

#endif //ZESTAW10_COMMON_H
