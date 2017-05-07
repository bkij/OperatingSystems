#ifndef MQ_SERVER_H
#define MQ_SERVER_H

#include <stdbool.h>
#include <unistd.h>
#include <mqueue.h>
#include "messaging.h"

#define MAX_CLIENTS 10

// Data structures and typedefs
typedef void (*CleanupFunction)(void);
typedef struct ClientInfo {
    mqd_t clients_msqids[MAX_CLIENTS];
    pid_t clients_pids[MAX_CLIENTS];
    int size;
} ClientInfo;
typedef struct RequestBuf {
    long type;
    union {
        ConnRequest connRequest;
        MsgInfo msgInfo;
        int client_id;
    } data;
    char padding;
} RequestBuf;

// Cleanup
void register_cleanup_functions(const CleanupFunction functions[], const int len);

// MQ functions
int create_public_queue(void);
void listen(const mqd_t public_q_id);


#endif