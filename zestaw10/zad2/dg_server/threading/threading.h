//
// Created by kveld on 30.05.17.
//

#ifndef ZESTAW10_THREADING_H
#define ZESTAW10_THREADING_H

#include <pthread.h>
#include <sys/epoll.h>
#include "../sockets/sockets.h"
#include "../../common/common.h"

#define MAX_EVENTS 3

pthread_t create_pinging_thread(struct thread_shared_data *data);
pthread_t create_requests_thread(struct thread_shared_data *data);

void *ping(void *arg);
void *handle_requests(void *arg);

int
negotiate_add(int sockfd, struct thread_shared_data *data, struct sockaddr_storage *addr, struct add_request *request,
              bool remote, socklen_t addrlen);

int init_epoll(struct epoll_event *events, struct thread_shared_data *data);
bool epoll_err(struct epoll_event *event);
#endif //ZESTAW10_THREADING_H
