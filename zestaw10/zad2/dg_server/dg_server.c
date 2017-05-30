//
// Created by kveld on 23.05.17.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include "dg_server.h"
#include "../common/common.h"
#include "threading/threading.h"

int main(int argc, char *argv[])
{
    char port_num[PORT_STR_LEN];
    char unix_sock_path[UNIX_PATH_MAX];
    parse_args(argc, argv, port_num, unix_sock_path);

    exit(EXIT_SUCCESS);
}

void parse_args(int argc, char **argv, char *port_num, char *unix_sock_path) {
    if(argc < 3) {
        fprintf(stderr, "%s\n", "usage: ./dg_server.out PORT_NUM UNIX_SOCK_PATH");
    }
    strncpy(port_num, argv[1], PORT_STR_LEN);
    strncpy(unix_sock_path, argv[2], UNIX_PATH_MAX);
}

void dipatch_threads(char *port_num, char *unix_sock_path) {
    pthread_t tids[WORKER_THREADS_NUM];
    int tidx = 0;

    struct socket_info socket_info = {.port_num = port_num, .unix_sock_path = unix_sock_path};

    tids[tidx++] = create_remote_listening_thread(&socket_info);
    tids[tidx++] = create_local_listening_thread(&socket_info);
    tids[tidx++] = create_requests_thread();
    tids[tidx++] = create_pinging_thread();

    for(int i = 0; i < tidx; i++) {
        if(!pthread_detach(tids[i])) {
            tids[i] = 0;
        }
    }

    /*
     * TODO: IO
     */

    for(int i = 0; i < tids; i++) {
        if(tids[i] != 0) {
            pthread_join(tids[i], NULL);
        }
    }
}
