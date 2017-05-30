//
// Created by kveld on 30.05.17.
//

#ifndef ZESTAW10_THREADING_H
#define ZESTAW10_THREADING_H

#include <pthread.h>
#include "../sockets/sockets.h"

struct socket_info {
    char *port_num;
    char *unix_sock_path;
};

pthread_t create_remote_listening_thread(struct socket_info *socket_info);
pthread_t create_local_listening_thread(struct socket_info *socket_info);
pthread_t create_pinging_thread();
pthread_t create_requests_thread();

#endif //ZESTAW10_THREADING_H
