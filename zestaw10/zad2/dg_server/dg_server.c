//
// Created by kveld on 23.05.17.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <errno.h>
#include "dg_server.h"
#include "../common/common.h"
#include "threading/threading.h"

int main(int argc, char *argv[])
{
    char port_num[PORT_STR_LEN];
    char unix_sock_path[UNIX_PATH_MAX];
    parse_args(argc, argv, port_num, unix_sock_path);
    dispatch_threads(port_num, unix_sock_path);
    exit(EXIT_SUCCESS);
}

void parse_args(int argc, char **argv, char *port_num, char *unix_sock_path) {
    if(argc < 3) {
        fprintf(stderr, "%s\n", "usage: ./dg_server.out PORT_NUM UNIX_SOCK_PATH");
    }
    strncpy(port_num, argv[1], PORT_STR_LEN);
    strncpy(unix_sock_path, argv[2], UNIX_PATH_MAX);
}

void dispatch_threads(char *port_num, char *unix_sock_path) {
    char input_buf[64];
    pthread_t tids[WORKER_THREADS_NUM];
    pthread_mutex_t cli_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;
    int io_pipe[2];
    init_pipe(io_pipe);
    struct thread_shared_data shared_data = {
            .clients = init_clients(),
            .msg_mutex = &msg_mutex,
            .cli_mutex = &cli_mutex,
            .remote_sockfd = create_remote_sock(port_num),
            .local_sockfd = create_local_sock(unix_sock_path),
            .io_rd_fd = io_pipe[0],
            .pinged = false
    };
    int tidx = 0;

    tids[tidx++] = create_requests_thread(&shared_data);
    tids[tidx++] = create_pinging_thread(&shared_data);

    for(int i = 0; i < tidx; i++) {
        if(!pthread_detach(tids[i])) {
            tids[i] = 0;
        }
    }
    /*
     * TODO: IO
     */
    while(fgets(input_buf, sizeof(input_buf), stdin) != NULL) {
        if(write(io_pipe[1], input_buf, sizeof(input_buf)) < 0) {
            fprintf(stderr, "Error writing to pipe: %s", strerror(errno));
        }
    }

    printf("ASD\n");
    for(int i = 0; i < tidx; i++) {
        if(tids[i] != 0) {
            pthread_join(tids[i], NULL);
        }
    }
}
