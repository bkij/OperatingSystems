//
// Created by kveld on 23.05.17.
//

#ifndef ZESTAW10_SERVER_H
#define ZESTAW10_SERVER_H

#define NUM_SERVER_THREADS 3

void parse_args(int argc, char **argv, int *port_num, char *socket_path);
void listen(int port_num, char *socket_path);

struct client_info {
    char client_name[CLIENT_NAME_LEN];
    int sock_fd;
};


#endif //ZESTAW10_SERVER_H
