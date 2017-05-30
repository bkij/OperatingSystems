//
// Created by kveld on 30.05.17.
//

#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "sockets.h"

int mk_nonblocking_local_sock(char *unix_sock_path) {
    struct addrinfo *results = get_local_addrinfo(unix_sock_path);
    int sockfd = mk_and_bind(results);
    mk_nonblocking(sockfd);
    freeaddrinfo(results);

    return sockfd;
}

int mk_nonblocking_remote_sock(char *port_num) {
    struct addrinfo *results = get_remote_addrinfo(port_num);
    int sockfd = mk_and_bind(results);
    mk_nonblocking(sockfd);
    freeaddrinfo(results);

    return sockfd;
}

int mk_and_bind(struct addrinfo *results) {
    int sockfd;
    for(struct addrinfo *rp = results; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sockfd < 0) {
            continue;
        }
        if(!bind(sockfd, rp->ai_addr, rp->ai_addrlen)) {
            // Bind success
            break;
        }
        else {
            close(sockfd);
        }
    }
    return sockfd;
}

void mk_nonblocking(int sockfd) {
    if(fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        fprintf(stderr, "fcntl error in func: %s: %s\n", "mk_nonblocking", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

struct addrinfo *get_remote_addrinfo(char *port_num) {
    int err;
    struct addrinfo hints;
    struct addrinfo *results;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        // IPv4 and IPv6
    hints.ai_socktype = SOCK_DGRAM;     // UDP

    if((err = getaddrinfo(NULL, port_num, &hints, &results)) < 0) {
        fprintf(stderr, "getaddrinfo error in func %s: %s\n", "get_remote_addrinfo", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    return results;
}

struct addrinfo *get_local_addrinfo(char *unix_sock_path) {
    int err;
    struct addrinfo hints;
    struct addrinfo *results;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNIX;          // Local
    hints.ai_socktype = SOCK_DGRAM;     // Datagram

    if((err = getaddrinfo(NULL, unix_sock_path, &hints, &results)) < 0) {
        fprintf(stderr, "getaddrinfo error in func %s: %s\n", "get_local_addrinfo", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    return results;
}
