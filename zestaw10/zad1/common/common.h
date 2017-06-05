//
// Created by kveld on 23.05.17.
//

#ifndef ZESTAW10_COMMON_H
#define ZESTAW10_COMMON_H

#include <pthread.h>
#include <stdbool.h>

#define MAX_PATH_LEN 108
#define MAX_CLIENTS 16
#define CLIENT_NAME_LEN 32

#define ERRNO_EXIT(S) do { perror(S); exit(EXIT_FAILURE); } while(1)

struct add_request {
    char client_name_buf[CLIENT_NAME_LEN];
    bool add;
};

struct op_request {
    char buf[64];
    int counter;
};

struct op_response {
    long response;
    int counter;
};

void acquire(pthread_mutex_t *mutex);
void release(pthread_mutex_t *mutex);

void sendn(int sockfd, void *buf, size_t len, int flags);
void recvn(int sockfd, void *buf, size_t len, int flags);

#endif //ZESTAW10_COMMON_H
