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
#include <sys/epoll.h>
#include "server_threads.h"
#include "../../common/common.h"
#include "../../../zad2/common/common.h"

static int request_counter = 0;

pthread_t create_local_listening_thread(struct thread_info *info)
{
    pthread_t tid;
    if(pthread_create(&tid, NULL, handle_local_conns, (void *)info) < 0) {
        ERRNO_EXIT("Couldnt create listening thread");
    }
    return tid;
}

pthread_t create_requests_thread(struct thread_info *info)
{
    pthread_t tid;
    if(pthread_create(&tid, NULL, handle_requests, (void *)info) < 0) {
        ERRNO_EXIT("Couldnt create requests thread");
    }
    return tid;
}

pthread_t create_pinging_thread(struct thread_info *info)
{
    pthread_t tid;
    if(pthread_create(&tid, NULL, ping, (void *)info) < 0) {
        ERRNO_EXIT("Couldnt create pinging thread");
    }
    return tid;
}

void *handle_local_conns(void *arg) {
    struct thread_info *conn_info = (struct thread_info *)arg;
    int server_local_sock = get_local_sock(conn_info->socket_path);

    while(1) {
        int newconnfd = accept(server_local_sock, NULL, NULL);
        if (newconnfd < 0) {
            fprintf(stderr, "Accpet error: %s\n", strerror(errno));
        }
        acquire(conn_info->mutex);
        bool added = negotiate_adding(newconnfd, conn_info);
        release(conn_info->mutex);
        if (added) {
            notify_added(newconnfd, conn_info->notify_wr_fd);
        }
    }
}

void notify_added(int newconnfd, int notify_fd)
{
    if(write(notify_fd, &newconnfd, sizeof(newconnfd)) < sizeof(newconnfd)) {
        ERRNO_EXIT("Pipe notification error");
    }
}

bool negotiate_adding(int newconnfd, struct thread_info *conn_info)
{
    struct add_request request_buf;
    memset(&request_buf, 0 ,sizeof(request_buf));
    bool can_add = true;

    recvn(newconnfd, &request_buf, CLIENT_NAME_LEN, 0);

    for(int i = 0; i < conn_info->client_size; i++) {
        if(conn_info->clients[i].sock_fd != -1 && !strcmp(request_buf.client_name_buf, conn_info->clients[i].client_name)) {
            can_add = false;
            break;
        }
    }
    if(can_add) {
        strcpy(conn_info->clients[conn_info->client_size].client_name, request_buf.client_name_buf);
        conn_info->clients[conn_info->client_size].sock_fd = newconnfd;
        conn_info->client_size++;
    }

    if(can_add) {
        printf("Registered new client: %s\n", request_buf.client_name_buf);
        sendn(newconnfd, "OK", sizeof("OK"), 0);
        return true;
    }
    else {
        printf("Didnt register client: %s, duplicate name or MAX_CLIENTS reached\n", request_buf.client_name_buf);
        sendn(newconnfd, "ERR", sizeof("ERR"), 0);
        return false;
    }
}

int get_local_sock(char *path) {
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd < 0) {
        ERRNO_EXIT("Error creating socket");
    }
    struct sockaddr_un addr;
    memset(&addr, 0 , sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    unlink(addr.sun_path);
    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        ERRNO_EXIT("Error binding socket");
    }
    if(listen(sockfd, MAX_CLIENTS) < 0) {
        ERRNO_EXIT("Error listening on socket");
    }
    return sockfd;
}

void *handle_requests(void *arg) {
    int newly_added = -1;
    int newfd;
    char input_buf[64];
    struct op_response response;
    srand(time(NULL));
    int last_cli_idx = -1;
    struct thread_info *data = (struct thread_info *)arg;
    // Poll for requests
    struct epoll_event events[MAX_CLIENTS + 2];
    int epoll_last_idx = 2;
    struct epoll_event ret_events[MAX_CLIENTS + 2];
    int epoll_fd = epoll_create1(0);
    if(epoll_fd == -1) {
        fprintf(stderr, "epoll create error: %s\n", strerror(errno));
    }
    events[0].data.fd = data->io_rd_fd;
    events[0].events = EPOLLIN;
    events[1].data.fd = data->notify_rd_fd;
    events[1].events = EPOLLIN;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, data->io_rd_fd, &events[0]) < 0) {
        fprintf(stderr, "Error adding io_rd_fd to epoll: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, data->notify_rd_fd, &events[1]) < 0) {
        fprintf(stderr, "Error adding notify_rd_fd to epoll: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    while(1) {
        int n = epoll_wait(epoll_fd, ret_events, MAX_CLIENTS + 2, -1);
        acquire(data->mutex);
        for(int i = 0; i < n; i++) {
            if(data->pinged) {
                data->pinged = false;
                break;
            }
            if(     (ret_events[i].events & EPOLLERR) ||
                    (ret_events[i].events & EPOLLHUP) ||
                    (!(ret_events[i].events & EPOLLIN))) {
        //        fprintf(stderr, "Miscellaneous epoll error\n");
        //        fprintf(stderr, "%s, %s, %s",
        //                pEvent->events & EPOLLERR    ? "EPOLLERR" : "",
        //                pEvent->events & EPOLLHUP    ? "EPOLLHUP" : "",
        //                !(pEvent->events & EPOLLIN)  ? "~EPOLLIN" : ""
        //        );
        //        exit(EXIT_FAILURE);
                close(ret_events[i].data.fd);
                int j;
                for(j = 0; j < MAX_CLIENTS; j++) {
                    if(data->clients[j].sock_fd == ret_events[i].data.fd) {
                        data->clients[j].sock_fd = -1;
                        break;
                    }
                }
                printf("%s disconnected\n", data->clients[j].client_name);
                continue;
            }
            if(ret_events[i].data.fd == data->io_rd_fd) {
                if(read(ret_events[i].data.fd, input_buf, sizeof(input_buf)) < 0) {
                    fprintf(stderr, "Error reading from io pipe\n");
                    exit(EXIT_FAILURE);
                }
                int cli_idx = rand() % MAX_CLIENTS;
                while(data->clients[cli_idx].sock_fd == -1 || cli_idx == last_cli_idx) {
                    cli_idx = (cli_idx + 1) % MAX_CLIENTS;
                }
                last_cli_idx = cli_idx;
                struct op_request request;
                request.counter = request_counter;
                memcpy(request.buf, input_buf, 64);
                printf("Sending request %d to client %s\n", request_counter, data->clients[cli_idx].client_name);
                if(send(data->clients[cli_idx].sock_fd, &request, sizeof(struct op_request), 0) < 0) {
                    fprintf(stderr, "Send error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                request_counter++;
                acquire(data->ping_mutex);
                data->reset = true;
                release(data->ping_mutex);
            }
            else if(ret_events[i].data.fd == data->notify_rd_fd) {
                if(read(ret_events[i].data.fd, &newfd, sizeof(newfd)) < 0) {
                    fprintf(stderr, "Err reading notification pipe: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                events[epoll_last_idx].data.fd = newfd;
                events[epoll_last_idx].events = EPOLLIN;
                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, newfd, &events[epoll_last_idx]) < 0) {
                    fprintf(stderr, "epoll_ctl add error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                newly_added = newfd;
                epoll_last_idx++;
            }
            else {
                ssize_t ret;
                if((ret = recv(ret_events[i].data.fd, &response, sizeof(response), 0)) < 0) {
                    fprintf(stderr, "Recv error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if(ret == 0) {
                    //EOF

                    close(ret_events[i].data.fd);
                    int j;
                    for(j = 0; j < MAX_CLIENTS; j++) {
                        if(data->clients[j].sock_fd == ret_events[i].data.fd) {
                            data->clients[j].sock_fd = -1;
                            break;
                        }
                    }
                    printf("%s disconnected\n", data->clients[j].client_name);
                    continue;
                }
                if(newly_added == ret_events[i].data.fd) {
                    newly_added = -1;
                    continue;
                }
                printf("Got response: %d, result is: %ld\n", response.counter, response.response);
                acquire(data->ping_mutex);
                data->reset = true;
                release(data->ping_mutex);
            }
        }
        release(data->mutex);
    }
}

void *ping(void *arg) {
    struct thread_info *info = (struct thread_info *)arg;
    while(1) {
        unsigned time_slept = 0;
        while(time_slept < 30) {
            time_slept += sleep(1);
            acquire(info->ping_mutex);
            if(info->reset) {
                time_slept = 0;
                info->reset = false;
            }
            release(info->ping_mutex);
        }
        acquire(info->mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (info->clients[i].sock_fd != -1) {
                if(check_on_client(info->clients[i].sock_fd) < 0) {
                    close(info->clients[i].sock_fd);
                    info->clients[i].sock_fd = -1;
                }
            }
        }
        info->pinged = true;
        release(info->mutex);
    }
}

int check_on_client(int sockfd)
{
    ssize_t bytes_sent = 0;
    const char buf[] = "PING";
    while(bytes_sent != sizeof(buf)) {
        ssize_t bytes = send(sockfd, buf, sizeof(buf), 0);
        if(bytes < 0) {
            return -1;
        }
        bytes_sent += bytes;
    }

    char resp_buf[3];
    ssize_t bytes_received = recv(sockfd, resp_buf, sizeof(resp_buf), 0);
    if(bytes_received < 0) {
        return -1;
    }
    return 0;
}

pthread_t create_remote_listening_thread(struct thread_info *info) {
    pthread_t tid;
    if(pthread_create(&tid, NULL, handle_remote_conns, (void *)info) < 0) {
        fprintf(stderr, "pthread create err: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return tid;
}

void *handle_remote_conns(void *arg) {
    struct thread_info *info = (struct thread_info *)arg;
    int server_remote_sock = get_net_sock(info->port_num);

    while(1) {
        int newconnfd = accept(server_remote_sock, NULL, NULL);
        if (newconnfd < 0) {
            fprintf(stderr, "Accpet error: %s\n", strerror(errno));
        }
        acquire(info->mutex);
        bool added = negotiate_adding(newconnfd, info);
        release(info->mutex);
        if (added) {
            notify_added(newconnfd, info->notify_wr_fd);
        }
    }
}

int get_net_sock(char *port_num) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int err;
    int sockfd;

    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((err = getaddrinfo(NULL, port_num, &hints, &result)) < 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(err));
        exit(EXIT_FAILURE);
    }
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd < 0) {
            continue;
        }
        if(!bind(sockfd, rp->ai_addr, rp->ai_addrlen)) {
            break;
        }
        close (sockfd);
    }
    if (rp == NULL) {
        fprintf (stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo (result);

    if(listen(sockfd, MAX_CLIENTS) < 0) {
        fprintf(stderr, "listen err\n");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}
