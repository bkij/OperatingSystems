//
// Created by kveld on 23.05.17.
//

#ifndef ZESTAW10_SERVER_H
#define ZESTAW10_SERVER_H

#include "../common/common.h"

#define NUM_SERVER_THREADS 3

void init_pipe(int notification_pipe[2]);
void parse_args(int argc, char **argv, char *port_num, char *socket_path);
void dispatch_and_listen(char *port_num, char *socket_path);

struct client_info {
    char client_name[CLIENT_NAME_LEN];
    int sock_fd;
};


#endif //ZESTAW10_SERVER_H
