//
// Created by kveld on 23.05.17.
//
#define POSIX_C_SOURCE 201113L
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include "server_threads.h"
#include "../../common/common.h"

int get_local_sock(char *path);

int check_on_client(int sockfd);

pthread_t create_listening_thread(int port_num, char *socket_path, struct client_info *clients)
{
    pthread_t tid;
    struct conn_thread_arg *arg = malloc(sizeof(struct conn_thread_arg));
    arg->port_num = port_num;
    arg->socket_path = socket_path;
    arg->clients = clients;
    arg->client_size = 0;
    if(pthread_create(&tid, NULL, handle_conns, (void *)&arg) < 0) {
        ERRNO_EXIT("Coudlnt create listening thread");
    }
    return tid;
}

pthread_t create_requests_thread(struct client_info *clients)
{
    pthread_t tid;
    if(pthread_create(&tid, NULL, handle_requests, (void *)clients) < 0) {
        ERRNO_EXIT("Coudlnt create requests thread");
    }
    return tid;
}

pthread_t create_pinging_thread(struct client_info *clients)
{
    pthread_t tid;
    if(pthread_create(&tid, NULL, ping, (void *)clients) < 0) {
        ERRNO_EXIT("Couldnt create pinging thread");
    }
    return tid;
}

void *handle_conns(void *arg) {
    struct conn_thread_arg *conn_info = (struct conn_thread_arg *)arg;
    int server_local_sock = get_local_sock(conn_info->socket_path);
    int server_net_sock = get_net_sock(conn_info->port_num);
    // TODO: Poll for connections on these sockets
}

int get_net_sock(int port_num)
{
    struct addrinfo hints, *info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if(getaddrinfo(NULL, port_num, &hints, &info) != 0) {
        ERRNO_EXIT("Couldnt create network facing socket");
    }

    int sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if(sockfd < 0) {
        ERRNO_EXIT("Couldnt create network facing socket configuration");
    }
    if(bind(sockfd, info->ai_addr, info->ai_addrlen) < 0) {
        ERRNO_EXIT("Couldnt bind on network facing socket");
    }
    return sockfd;
}

int get_local_sock(char *path) {
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd < 0) {
        ERRNO_EXIT("Error creating socket");
    }
    struct sockaddr_un addr;
    memset(&addr, 0 , sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        ERRNO_EXIT("Error binding socket");
    }
    return sockfd;
}

void *handle_requests(void *arg) {
    struct client_info *clients = (struct client_info *)arg;
    // Poll for requests
}

void *ping(void *arg) {
    struct client_info *clients = (struct client_info *)arg;
    while(1) {
        sleep(5);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].sock_fd != -1) {
                if(check_on_client(clients[i].sock_fd) < 0) {
                    close(clients[i].sock_fd);
                    clients[i].sock_fd = -1;
                }
            }
        }
    }
}

int check_on_client(int sockfd)
{
    ssize_t bytes_sent = 0;
    const char buf[] = "PING";
    while(bytes_sent != sizeof(buf)) {
        ssize_t bytes = send(sockfd, buf, sizeof(buf), 0);
        if(bytes < 0) {
            ERRNO_EXIT("Send error while pinging");
        }
        bytes_sent += bytes;
    }

    char resp_buf[3];
    ssize_t bytes_received = recv(sockfd, resp_buf, sizeof(resp_buf), MSG_DONTWAIT);
    if(bytes_received < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return -1;
    }
    else if(bytes_received < 0) {
        ERRNO_EXIT("Error while pinging");
    }
    return 0;
}
