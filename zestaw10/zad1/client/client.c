//
// Created by kveld on 23.05.17.
//

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <netdb.h>
#include <stdio.h>
#include "client.h"
#include "../common/common.h"

static jmp_buf jmp;
static jmp_buf jmpremote;
bool remote;

void parse_args(int argc, char **argv, char *name, char *address, char *unix_sock_path, char *port);

void handlesigint(int signo);

void run_local(char name[], char unix_sock_path[108]);
void run_remote(char name[], char address[], char port[]);

long get_result(char buf[64]);

int main(int argc, char **argv)
{
    char name[CLIENT_NAME_LEN + 1];
    char address[16];
    char unix_sock_path[UNIX_PATH_MAX];
    char port[6];
    parse_args(argc, argv, name, address, unix_sock_path, port);
    signal(SIGINT, handlesigint);
    if(port[0] == '-') {
        remote = false;
        run_local(name, unix_sock_path);
    }
    else {
        remote = true;
        run_remote(name, address, port);
    }
    exit(EXIT_SUCCESS);
}

void run_local(char name[], char unix_sock_path[108]) {
    struct op_request request_buf;
    char add_response[4];
    memset(add_response, 0, 4);
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, unix_sock_path);

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd == -1) {
        fprintf(stderr, "Error creating socket in func %s: %s\n", "run_local", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct add_request add_request;
    add_request.add = true;
    strcpy(add_request.client_name_buf, name);

    if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Error connecting add request: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(recv(sockfd, add_response, 4, 0) < 0) {
        fprintf(stderr, "Error receiving add response: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(!strncmp(add_response, "ERR", 3)) {
        printf("Error connecting to server, MAX_CLIENTS reached or duplicate name\n");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Connected to server\n");
    }
    if(setjmp(jmp) != 0) {
        add_request.add = false;
        if(send(sockfd, &add_request, sizeof(add_request), 0) < 0) {
            fprintf(stderr, "Send error, request: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    while(1) {
        memset(&request_buf, 0 , sizeof(request_buf));
        if(recv(sockfd, &request_buf, sizeof(request_buf), 0) < 0) {
            fprintf(stderr, "Recvfrom error, request %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if(!strcmp(request_buf.buf, "PING")) {
            printf("got a ping\n");
            char ok[] = "OK";
            if(send(sockfd, "OK", sizeof("OK"), 0) < 0) {
                fprintf(stderr, "Send error, request: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            printf("sent back OK\n");
        }
        else {
            printf("Got request: %d\n", request_buf.counter);
            struct op_response response = {
                    .response = get_result(request_buf.buf),
                    .counter = request_buf.counter
            };
            printf("Sending result: %ld\n", response.response);
            if (send(sockfd, &response, sizeof(response), 0)){
                fprintf(stderr, "Send error, request: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }
}

long get_result(char buf[64]) {
    char *tok;
    int a, b;
    switch(buf[0]) {
        case '1':
            buf++;
            tok = strtok(buf, " ");
            a = atoi(tok);
            tok = strtok(NULL, " ");
            b = atoi(tok);
            return (long)a + (long)b;
            break;
        case '2':
            buf++;
            tok = strtok(buf, " ");
            a = atoi(tok);
            tok = strtok(NULL, " ");
            b = atoi(tok);
            return (long)a * (long)b;
            break;
        case '3':
            buf++;
            tok = strtok(buf, " ");
            a = atoi(tok);
            tok = strtok(NULL, " ");
            b = atoi(tok);
            return (long)a - (long)b;
            break;
        case '4':
            buf++;
            tok = strtok(buf, " ");
            a = atoi(tok);
            tok = strtok(NULL, " ");
            b = atoi(tok);
            return (long)a / (long)b;
            break;
        default:
            fprintf(stderr, "%s\n", "switch err");
    }
    return -1;
}


void parse_args(int argc, char **argv, char *name, char *address, char *unix_sock_path, char *port) {
    if(argc < 4) {
        fprintf(stderr, "%s\n%s\n", "usage: ./dg_client.out NAME ADDRESS/PATH PORT", "Connects locally if port == -1");
    }
    strcpy(name, argv[1]);
    strcpy(port, argv[3]);
    if(port[0] == '-') {
        strcpy(unix_sock_path, argv[2]);
    }
    else {
        strcpy(address, argv[2]);
    }
}

void handlesigint(int signo) {
    if(remote) {
        longjmp(jmpremote, 1);
    }
    else {
        longjmp(jmp, 1);
    }
}

void run_remote(char *name, char *address, char *port) {
    struct op_request request;
    struct addrinfo serv_hints, *servinfo, *p;
    memset(&serv_hints, 0, sizeof(struct addrinfo));
    serv_hints.ai_family = AF_UNSPEC;
    serv_hints.ai_socktype = SOCK_STREAM;
    int sockfd = -1;

    if(getaddrinfo(address, port, &serv_hints, &servinfo) < 0) {
        fprintf(stderr, "Couldnt get addr info: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue;
        }
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockfd);
            continue;
        }
        break;
    }
    if(p == NULL) {
        fprintf(stderr, "failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    struct add_request add_request;
    add_request.add = true;
    strcpy(add_request.client_name_buf, name);

    if(send(sockfd, &add_request, sizeof(add_request), 0) < 0) {
        fprintf(stderr, "Error sending add request: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    char response_buf[4];
    if(recv(sockfd, response_buf, 4, 0) < 0) {
        fprintf(stderr, "Error receiving add response: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(!strncmp(response_buf, "ERR", 3)) {
        printf("Couldnt connect to server\n");
        exit(EXIT_SUCCESS);
    }
    else {
        printf("Connected to server\n");
    }
    if(setjmp(jmpremote) != 0) {
        add_request.add = false;
        if(send(sockfd, &add_request, sizeof(add_request), 0) < 0) {
            fprintf(stderr, "Sendto error, request: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    while(1) {
        memset(&request, 0, sizeof(request));
        if(recv(sockfd, &request, sizeof(request), 0) < 0) {
            fprintf(stderr, "Recv error, request %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if(!strncmp(request.buf, "PING", 4)) {
            printf("got a ping\n");
            char ok[] = "OK";
            if(send(sockfd, ok, sizeof(ok), 0) < 0) {
                fprintf(stderr, "Send error, request: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            printf("sent back OK\n");
        }
        else {
            printf("Got request %d\n", request.counter);
            struct op_response response = {
                    .response = get_result(request.buf),
                    .counter = request.counter
            };
            printf("Sending result: %ld\n", response.response);
            sendn(sockfd, &response, sizeof(response), 0);
//            if (send(sockfd, &response, sizeof(response), 0) < 0) {
//                fprintf(stderr, "Send error, request: %s\n", strerror(errno));
//                exit(EXIT_FAILURE);
//            }
        }
    }
}
