//
// Created by kveld on 30.05.17.
//

#ifndef ZESTAW10_SOCKETS_H
#define ZESTAW10_SOCKETS_H

int mk_nonblocking_local_sock(char *unix_sock_path);
int mk_nonblocking_remote_sock(char *port_num);
int mk_and_bind(struct addrinfo *results);
void mk_nonblocking(int sockfd);

struct addrinfo *get_remote_addrinfo(char *port_num);
struct addrinfo *get_local_addrinfo(char *unix_sock_path);
#endif //ZESTAW10_SOCKETS_H
