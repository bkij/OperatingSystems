//
// Created by kveld on 23.05.17.
//

#ifndef ZESTAW10_SERVER_THREADS_H
#define ZESTAW10_SERVER_THREADS_H

#include <pthread.h>
#include "../server.h"

struct local_conn_thread_info {
    pthread_mutex_t *mutex;
    char *socket_path;
    struct client_info *clients;
    int client_size;
};

pthread_t create_local_listening_thread(char *socket_path, struct client_info *clients, pthread_mutex_t *mutex);
pthread_t create_requests_thread(struct client_info *clients, pthread_mutex_t *mutex);
pthread_t create_pinging_thread(struct client_info *clients, pthread_mutex_t *mutex);

void *handle_local_conns(void *arg);
void *handle_requests(void *arg);
void *ping(void *arg);

bool negotiate_adding(int newconnfd, struct local_conn_thread_info *conn_info);
void notify_added(int newconnfd, int notify_fd);

int get_local_sock(char *path);
int get_net_sock(int port_num);

#endif //ZESTAW10_SERVER_THREADS_H
