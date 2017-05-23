//
// Created by kveld on 23.05.17.
//

#include <malloc.h>
#include <stdlib.h>
#include "server_threads.h"
#include "../../common/common.h"

pthread_t create_listening_thread(int port_num, char *socket_path, struct client_info *clients)
{
    pthread_t tid;
    struct conn_thread_arg *arg = malloc(sizeof(struct conn_thread_arg));
    arg->port_num = port_num;
    arg->socket_path = socket_path;
    arg->clients = clients;
    if(pthread_create(&tid, NULL, handle_conns, (void *)&arg) < 0) {
        ERRNO_EXIT("Coudlnt create listening thread");
    }
    return tid;
}

pthread_t create_requests_thread(struct client_info *clients)
{
    pthread_t tid;
    if(pthread_create(&tid, NULL, handle_requests, (void *)clients) < 0) {
        ERRNO_EXIT("Coudlnt create requests thread");
    }
    return tid;
}

pthread_t create_pinging_thread(struct client_info *clients)
{
    pthread_t tid;
    if(pthread_create(&tid, NULL, ping, (void *)clients) < 0) {
        ERRNO_EXIT("Couldnt create pinging thread");
    }
    return tid;
}
