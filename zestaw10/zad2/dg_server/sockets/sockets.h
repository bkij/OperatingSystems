//
// Created by kveld on 30.05.17.
//

#ifndef ZESTAW10_SOCKETS_H
#define ZESTAW10_SOCKETS_H

#define _XOPEN_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int create_local_sock(char *unix_sock_path);
int create_remote_sock(char *port_num);
int create_and_bind(struct addrinfo *results);
void make_nonblocking(int sockfd);

struct addrinfo *get_remote_addrinfo(char *port_num);
#endif //ZESTAW10_SOCKETS_H
