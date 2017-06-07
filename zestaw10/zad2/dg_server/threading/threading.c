//
// Created by kveld on 30.05.17.
//

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "threading.h"

static int client_idx = 0;

pthread_t create_pinging_thread(struct thread_shared_data *data) {
    pthread_t tid;
    if(pthread_create(&tid, NULL, ping, (void *)data) < 0) {
        fprintf(stderr, "Error creating pinging thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return tid;
}

pthread_t create_requests_thread(struct thread_shared_data *data) {
    pthread_t tid;
    if(pthread_create(&tid, NULL, handle_requests, (void *)data) < 0) {
        fprintf(stderr, "Error creating requests thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return tid;
}

void *ping(void *arg) {
    struct thread_shared_data *data = (struct thread_shared_data *)arg;
    char request_buf[] = "PING";
    char response_buf[3];
    while(1) {
        sleep(30);
        acquire(data->cli_mutex);;
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(data->clients[i].id != -1) {
                struct sockaddr_storage *addr = &data->clients[i].addr_info;
                socklen_t addr_len = data->clients[i].addr_len;
                if(data->clients[i].is_remote) {
                    if(sendto(data->remote_sockfd, request_buf, sizeof(request_buf), 0, (struct sockaddr *)addr, addr_len) < 0) {
                        data->clients[i].id = -1;
                        printf("Pinging thread - client %s not responding, removed\n", data->clients[i].client_name);
                    }
                    if(recvfrom(data->remote_sockfd, response_buf, sizeof(response_buf), 0, NULL, NULL) < 0) {
                        // Client not responding
                        data->clients[i].id = -1;
                        printf("Pinging thread - client %s not responding, removed\n", data->clients[i].client_name);
                    }
                }
                else {
                    if(sendto(data->local_sockfd, request_buf, sizeof(request_buf), 0, (struct sockaddr *)addr, addr_len) < 0) {
                        data->clients[i].id = -1;
                        printf("Pinging thread - client %s not responding, removed\n", data->clients[i].client_name);
                    }
                    if(recvfrom(data->local_sockfd, response_buf, sizeof(response_buf), 0, NULL, NULL) < 0) {
                        data->clients[i].id = -1;
                        printf("Pinging thread - client %s not responding, removed\n", data->clients[i].client_name);
                    }
                }
            }
        }
        data->pinged = true;
        release(data->cli_mutex);
    }
}

void *handle_requests(void *arg)
{
    srand(time(NULL));
    int last_cli_idx = -1;
    char input_buf[64];
    long response_buf;
    struct thread_shared_data *data = (struct thread_shared_data *)arg;
    struct epoll_event events[3];
    struct epoll_event ret_events[MAX_EVENTS];
    memset(ret_events, 0, sizeof(ret_events));
    // epoll init
    int epoll_fd = init_epoll(events, data);
    while(1) {
        int nevents = epoll_wait(epoll_fd, ret_events, MAX_EVENTS, -1);
        acquire(data->cli_mutex);
        for(int i = 0; i < nevents; i++) {
            if(data->pinged) {
                data->pinged = false;
                break;
            }
            if(epoll_err(&ret_events[i])) {
                close(ret_events[i].data.fd);
                exit(EXIT_FAILURE);
            }
            if(ret_events[i].data.fd == data->local_sockfd) {
                struct add_request request_buf;
                struct sockaddr_storage addr_buf;
                socklen_t addr_len = sizeof(struct sockaddr_storage);
                if(recvfrom(ret_events[i].data.fd, &request_buf, sizeof(request_buf), 0, (struct sockaddr *)&addr_buf, &addr_len) < 0) {
                    fprintf(stderr, "Receive error from local socket(first conn): %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if(!request_buf.add) {
                    // Unregister client
                    for(int i = 0; i < MAX_CLIENTS; i++) {
                        if(!strcmp(data->clients[i].client_name, request_buf.client_name)) {
                            data->clients[i].id = -1;
                            printf("Client %s unregistered\n", request_buf.client_name);
                            break;
                        }
                    }
                    continue;
                }
                printf("Got a client request from %s on local socket\n", request_buf.client_name);
                int cli_idx = negotiate_add(ret_events[i].data.fd, data, &addr_buf, &request_buf, false, addr_len);
                struct add_response response = {cli_idx};
                if(sendto(ret_events[i].data.fd, &response, sizeof(struct add_response), 0, (struct sockaddr *)&addr_buf, addr_len) < 0) {
                    fprintf(stderr, "Send error form local socket(first conn): %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if(cli_idx != -1) {
                    printf("Client %s added\n", request_buf.client_name);
                }
                else {
                    printf("Client %s not added, MAX_CLIENTS reached or duplicate name\n", request_buf.client_name);
                }
            }
            else if(ret_events[i].data.fd == data->remote_sockfd) {
                struct add_request request_buf;
                struct sockaddr_storage addr_buf;
                socklen_t addr_len = sizeof(struct sockaddr_storage);
                if(recvfrom(ret_events[i].data.fd, &request_buf, sizeof(request_buf), 0, (struct sockaddr *)&addr_buf, &addr_len) < 0) {
                    fprintf(stderr, "Receive error from remote socket(first conn): %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if(!request_buf.add) {
                    // Unregister client
                    for(int j = 0; j < MAX_CLIENTS; j++) {
                        if(!strcmp(data->clients[j].client_name, request_buf.client_name)) {
                            data->clients[j].id = -1;
                            printf("Client %s unregistered\n", request_buf.client_name);
                            break;
                        }
                    }
                    continue;
                }
                printf("Got a client request from %s on remote socket\n", request_buf.client_name);
                int cli_idx = negotiate_add(ret_events[i].data.fd, data, &addr_buf, &request_buf, true, addr_len);
                struct add_response response = {cli_idx};
                if(sendto(ret_events[i].data.fd, &response, sizeof(struct add_response), 0, (struct sockaddr *)&addr_buf, addr_len) < 0) {
                    fprintf(stderr, "Send error form remote socket(first conn): %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if(cli_idx != -1) {
                    printf("Client %s added\n", request_buf.client_name);
                }
                else {
                    printf("Client %s not added, MAX_CLIENTS reached or duplicate name\n", request_buf.client_name);
                }
            }
            else {
                if(read(ret_events[i].data.fd, input_buf, sizeof(input_buf)) < 0) {
                    fprintf(stderr, "Error reading from io pipe\n");
                    exit(EXIT_FAILURE);
                }
                int cli_idx = rand() % MAX_CLIENTS;
                while(data->clients[cli_idx].id == -1 || cli_idx == last_cli_idx) {
                    cli_idx = (cli_idx + 1) % MAX_CLIENTS;
                }
                last_cli_idx = cli_idx;
                printf("Sending request to client %s\n", data->clients[cli_idx].client_name);
                int cli_sockfd;
                if(data->clients[cli_idx].is_remote) {
                    cli_sockfd = data->remote_sockfd;
                }
                else {
                    cli_sockfd = data->local_sockfd;
                }
                struct sockaddr_storage *addr_buf = &data->clients[cli_idx].addr_info;
                socklen_t addr_len = data->clients[cli_idx].addr_len;
                if(sendto(cli_sockfd, input_buf, sizeof(input_buf), 0, (struct sockaddr *)addr_buf, addr_len) < 0) {
                    fprintf(stderr, "Sendto error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if(recvfrom(cli_sockfd, &response_buf, sizeof(long), 0, NULL, NULL) < 0) {
                    fprintf(stderr, "Recvfrom error %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                printf("Response : %ld\n", response_buf);
            }
        }
        release(data->cli_mutex);
    }
}

int init_epoll(struct epoll_event *events, struct thread_shared_data *data)
{
    int epoll_fd = epoll_create1(0);
    if(epoll_fd < 0) {
        fprintf(stderr, "Epoll create error - %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    events[0].data.fd = data->local_sockfd;
    events[0].events = EPOLLIN;
    events[1].data.fd = data->remote_sockfd;
    events[1].events = EPOLLIN;
    events[2].data.fd = data->io_rd_fd;
    events[2].events = EPOLLIN;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, data->local_sockfd, &events[0]) < 0) {
        fprintf(stderr, "Error adding local socket to epoll: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, data->remote_sockfd, &events[1]) < 0) {
        fprintf(stderr, "Error adding remote socket to epoll: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, data->io_rd_fd, &events[2]) < 0) {
        fprintf(stderr, "Error adding pipe to epoll: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return epoll_fd;
}

bool epoll_err(struct epoll_event *event)
{
    if((event->events & EPOLLERR) ||
       (event->events & EPOLLHUP) ||
       !(event->events & EPOLLIN)) {
        fprintf(stderr, "Epoll error %s\n", strerror(errno));
        return true;
    }
    return false;
}


int
negotiate_add(int sockfd, struct thread_shared_data *data, struct sockaddr_storage *addr, struct add_request *request,
              bool remote, socklen_t addrlen)
{
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(data->clients[i].id != -1 &&
           !strcmp(data->clients[i].client_name, request->client_name)) {
            return -1;
        }
    }
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(data->clients[i].id == -1) {
            data->clients[i].id = client_idx++;
            strcpy(data->clients[i].client_name, request->client_name);
            memcpy(&data->clients[i].addr_info, addr, sizeof(struct addrinfo));
            data->clients[i].is_remote = remote;
            data->clients[i].addr_len = addrlen;
            return data->clients[i].id;
        }
    }
    return -1;
}