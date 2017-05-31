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
#include <sys/un.h>
#include "sockets.h"

int create_local_sock(char *unix_sock_path) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, unix_sock_path);
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(sockfd == -1) {
        fprintf(stderr, "Error creating socket at func %s: %s\n", "create_local_sock", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
        fprintf(stderr, "Error binding local socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
//    if(unlink(unix_sock_path) < 0) {
//        fprintf(stderr, "Error unlinking local socket: %s\n", strerror(errno));
//        exit(EXIT_FAILURE);
//    }

    return sockfd;
}

int create_remote_sock(char *port_num) {
    struct addrinfo *results = get_remote_addrinfo(port_num);
    int sockfd = create_and_bind(results);
    freeaddrinfo(results);

    return sockfd;
}

int create_and_bind(struct addrinfo *results) {
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

void make_nonblocking(int sockfd) {
    if(fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        fprintf(stderr, "fcntl error in func: %s: %s\n", "make_nonblocking", strerror(errno));
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
