//
// Created by kveld on 23.05.17.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "threads/server_threads.h"
#include "../common/common.h"
#include "server.h"


void init_clients(struct client_info *clients);

void init_pipe(int notification_pipe[2]);

int main(int argc, char *argv[])
{
    char port_num[6];
    char socket_path[MAX_PATH_LEN];
    parse_args(argc, argv, port_num, socket_path);
    dispatch_and_listen(port_num, socket_path);
    exit(EXIT_SUCCESS);
}

void parse_args(int argc, char **argv, char *port_num, char *socket_path)
{
    if(argc < 3) {
        fprintf(stderr, "%s\n", "Usage: [sudo] ./server PORT_NUM SOCKET_PATH");
        exit(EXIT_FAILURE);
    }
    strcpy(port_num, argv[1]);
    strcpy(socket_path, argv[2]);
}

void dispatch_and_listen(char *port_num, char *socket_path)
{
    char input_buf[64];
    pthread_mutex_t ping_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_t tids[NUM_SERVER_THREADS];
    struct client_info clients[MAX_CLIENTS];
    int notification_pipe[2];
    int io_pipe[2];

    init_clients(clients);
    init_pipe(notification_pipe);
    init_pipe(io_pipe);

    struct thread_info info = {
            .clients = clients,
            .client_size = 0,
            .mutex = &mutex,
            .ping_mutex = &ping_mutex,
            .io_rd_fd = io_pipe[0],
            .notify_wr_fd = notification_pipe[1],
            .notify_rd_fd = notification_pipe[0],
            .port_num = port_num,
            .socket_path = socket_path,
            .pinged = false,
            .reset = false
    };

    int idx = 0;
    tids[idx++] = create_remote_listening_thread(&info);
    tids[idx++] = create_local_listening_thread(&info);
    tids[idx++] = create_requests_thread(&info);
//TODO: ? tids[idx] = create_pinging_thread(&info);

    while(fgets(input_buf, sizeof(input_buf), stdin) != NULL) {
        if(write(io_pipe[1], input_buf, sizeof(input_buf)) < 0) {
            fprintf(stderr, "Write error after fgets: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

}

void init_pipe(int notification_pipe[2])
{
    if(pipe(notification_pipe) < 0) {
        ERRNO_EXIT("Error creating pipe");
    }
}

void init_clients(struct client_info *clients)
{
    for(int i = 0; i < MAX_CLIENTS; i++) {
        memset(clients[i].client_name, 0, sizeof(clients[i].client_name));
        clients[i].sock_fd = -1;
    }
}
