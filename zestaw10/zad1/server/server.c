//
// Created by kveld on 23.05.17.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "threads/server_threads.h"
#include "../common/common.h"
#include "server.h"


void init_clients(struct client_info *clients);

int main(int argc, char *argv[])
{
    int port_num;
    char socket_path[MAX_PATH_LEN];
    parse_args(argc, argv, &port_num, socket_path);
    listen(port_num, socket_path);
    exit(EXIT_SUCCESS);
}

void parse_args(int argc, char **argv, int *port_num, char *socket_path)
{
    if(argc < 3) {
        fprintf(stderr, "%s\n", "Usage: ./server PORT_NUM SOCKET_PATH");
        exit(EXIT_FAILURE);
    }
    *port_num = atoi(argv[1]);
    strncpy(socket_path, argv[2], MAX_PATH_LEN - 1);
    socket_path[MAX_PATH_LEN - 1] = '\0';
}

void listen(int port_num, char *socket_path)
{
    pthread_t tids[NUM_SERVER_THREADS];
    struct client_info clients[MAX_CLIENTS];
    init_clients(clients);

    int idx = 0;
    tids[idx++] = create_listening_thread(port_num, socket_path, clients);
    tids[idx++] = create_requests_thread(clients);
    tids[idx++] = create_pinging_thread(clients);

    for(int i = 0; i < NUM_SERVER_THREADS; i++) {
        if(pthread_join(tids[i], NULL) < 0) {
            ERRNO_EXIT("Error joining a thread");
        }
    }
}

void init_clients(struct client_info *clients)
{
    for(int i = 0; i < MAX_CLIENTS; i++) {
        memset(clients[i].client_name, 0, sizeof(clients[i].client_name));
        clients[i].sock_fd = -1;
    }
}
