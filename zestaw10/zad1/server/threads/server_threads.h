//
// Created by kveld on 23.05.17.
//

#ifndef ZESTAW10_SERVER_THREADS_H
#define ZESTAW10_SERVER_THREADS_H

#include <pthread.h>
#include "../server.h"

struct conn_thread_arg {
    int port_num;
    char *socket_path;
    struct client_info *clients;
    int client_size;
};

pthread_t create_listening_thread(int port_num, char *socket_path, struct client_info *clients);
pthread_t create_requests_thread(struct client_info *clients);
pthread_t create_pinging_thread(struct client_info *clients);

void *handle_conns(void *arg);
void *handle_requests(void *arg);
void *ping(void *arg);

int get_local_sock(char *path);
int get_net_sock(int port_num);

#endif //ZESTAW10_SERVER_THREADS_H
