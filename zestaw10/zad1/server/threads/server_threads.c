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
#include <stdbool.h>
#include "server_threads.h"
#include "../../common/common.h"

int get_local_sock(char *path);

int check_on_client(int sockfd);

bool negotiate_adding(int newconnfd, struct local_conn_thread_info *conn_info);

void notify_added(int notify_fd);

pthread_t create_local_listening_thread(char *socket_path, struct client_info *clients, pthread_mutex_t *mutex)
{
    pthread_t tid;
    struct local_conn_thread_info *arg = malloc(sizeof(struct local_conn_thread_info));
    arg->socket_path = socket_path;
    arg->clients = clients;
    arg->client_size = 0;
    arg->mutex = mutex;
    if(pthread_create(&tid, NULL, handle_local_conns, (void *) &arg) < 0) {
        ERRNO_EXIT("Couldnt create listening thread");
    }
    return tid;
}

pthread_t create_requests_thread(struct client_info *clients, pthread_mutex_t *mutex)
{
    pthread_t tid;
    if(pthread_create(&tid, NULL, handle_requests, (void *)clients) < 0) {
        ERRNO_EXIT("Couldnt create requests thread");
    }
    return tid;
}

pthread_t create_pinging_thread(struct client_info *clients, pthread_mutex_t *mutex)
{
    pthread_t tid;
    if(pthread_create(&tid, NULL, ping, (void *)clients) < 0) {
        ERRNO_EXIT("Couldnt create pinging thread");
    }
    return tid;
}

void *handle_local_conns(void *arg) {
    struct local_conn_thread_info *conn_info = (struct local_conn_thread_info *)arg;
    int server_local_sock = get_local_sock(conn_info->socket_path);

    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int newconnfd = accept(server_local_sock, (struct sockaddr *)&client_addr, &client_addr_size);
    if(newconnfd < 0) {
        // TODO: Really exit?
        ERRNO_EXIT("Accept error");
    }
    bool added = negotiate_adding(newconnfd, conn_info);
    if(added) {
        notify_added(newconnfd, conn_info->notify_wr);
    }
}

void notify_added(int newconnfd, int notify_fd)
{
    if(write(notify_fd, &newconnfd, sizeof(newconnfd)) < sizeof(newconnfd)) {
        ERRNO_EXIT("Pipe notification error");
    }
}

bool negotiate_adding(int newconnfd, struct local_conn_thread_info *conn_info)
{
    char client_name_buf[CLIENT_NAME_LEN];
    bool can_add = true;

    recvn(newconnfd, client_name_buf, CLIENT_NAME_LEN, 0);

    acquire(conn_info->mutex);
    for(int i = 0; i < conn_info->client_size; i++) {
        if(!strncmp(client_name_buf, conn_info->clients[i].client_name, CLIENT_NAME_LEN)) {
            can_add = false;
            break;
        }
    }
    if(can_add) {
        strncpy(
                conn_info->clients[conn_info->client_size].client_name,
                client_name_buf,
                CLIENT_NAME_LEN - 1
        );
        conn_info->clients[conn_info->client_size].client_name[CLIENT_NAME_LEN - 1] = '\0';
        conn_info->client_size++;
    }
    release(conn_info->mutex);

    if(can_add) {
        sendn(newconnfd, "OK", sizeof("OK"), 0);
        return true;
    }
    else {
        sendn(newconnfd, "ERR", sizeof("ERR"), 0);
        return false;
    }
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
    if(listen(sockfd, MAX_CLIENTS) < 0) {
        ERRNO_EXIT("Error listening on socket");
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
    if(listen(sockfd, MAX_CLIENTS) < 0) {
        ERRNO_EXIT("Error listening on socket");
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
