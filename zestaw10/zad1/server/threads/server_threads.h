//
// Created by kveld on 23.05.17.
//

#ifndef ZESTAW10_SERVER_THREADS_H
#define ZESTAW10_SERVER_THREADS_H

#include <pthread.h>
#include <stdbool.h>
#include "../server.h"

struct thread_info {
    pthread_mutex_t *mutex;
    pthread_mutex_t *ping_mutex;
    char *port_num;
    char *socket_path;
    struct client_info *clients;
    int client_size;
    int io_rd_fd;
    int notify_rd_fd;
    int notify_wr_fd;
    bool pinged;
    bool reset;
};

void check_err(struct epoll_event *pEvent);

pthread_t create_local_listening_thread(struct thread_info *info);
pthread_t create_requests_thread(struct thread_info *info);
pthread_t create_pinging_thread(struct thread_info *info);
pthread_t create_remote_listening_thread(struct thread_info *info);

void *handle_local_conns(void *arg);
void *handle_remote_conns(void *arg);
void *handle_requests(void *arg);
void *ping(void *arg);

bool negotiate_adding(int newconnfd, struct thread_info *conn_info);
void notify_added(int newconnfd, int notify_fd);

int get_local_sock(char *path);
int get_net_sock(char *port_num);

int check_on_client(int sockfd);
bool negotiate_adding(int newconnfd, struct thread_info *conn_info);


#endif //ZESTAW10_SERVER_THREADS_H
