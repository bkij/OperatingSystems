//
// Created by kveld on 23.05.17.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "common.h"

void acquire(pthread_mutex_t *mutex)
{
    if(pthread_mutex_lock(mutex) < 0) {
        ERRNO_EXIT("Mutex error");
    }
}
void release(pthread_mutex_t *mutex)
{
    if(pthread_mutex_unlock(mutex) < 0) {
        ERRNO_EXIT("Mutex error");
    }
}

void sendn(int sockfd, void *buf, size_t len, int flags)
{
    size_t total_bytes_sent = 0;
    size_t n = len;
    ssize_t bytes_sent;
    while(total_bytes_sent < len) {
        bytes_sent = send(sockfd, buf, n, flags);
        if(bytes_sent == -1) {
            ERRNO_EXIT("Send error");
        }
        else {
            total_bytes_sent += bytes_sent;
            buf += bytes_sent;
            n -= bytes_sent;
        }
    }

}

void recvn(int sockfd, void *buf, size_t len, int flags)
{
    size_t total_bytes_received = 0;
    size_t n = len;
    ssize_t bytes_received;
    while(total_bytes_received < len) {
        bytes_received = recv(sockfd, buf, n, flags);
        if(bytes_received == -1) {
            ERRNO_EXIT("Recv error");
        }
        else {
            total_bytes_received += bytes_received;
            buf += bytes_received;
            n -= bytes_received;
        }
    }
}
