#ifndef MQ_CLIENT_H
#define MQ_CLIENT_H

#include <mqueue.h>
#include "messaging.h"

// Typedefs
typedef void (*CleanupFunction)(void);
typedef struct MsgBuf {
    long type;
    union {
        ConnResponseBuf connResponseBuf;
        MsgInfo msgInfo;
        char time[21];
        int client_id;
    } data;
    char padding;
} MsgBuf;

// Cleanup
void register_cleanup_functions(const CleanupFunction functions[], const int len);

// MQ functions
mqd_t get_public_q(void);
mqd_t create_private_q(char *private_q_name);
void send_conn_request(const mqd_t public_msqid, const char *private_q_name);
int get_id(const mqd_t private_msqid);

void request_loop(const mqd_t public_msqid, const mqd_t private_msqid, const int client_id);
#endif