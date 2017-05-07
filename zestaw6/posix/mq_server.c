#define _XOPEN_SOURCE 500
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <wordexp.h>
#include "mq_server.h"
#include "messaging.h"
#include "util.h"


// Helper
static void init_clients(ClientInfo *clients)
{
    clients->size = 0;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        clients->clients_msqids[i] = -1;
    }
}

static int add_client(ClientInfo *clients, const q_name[MAX_Q_NAME], const pid_t pid)
{
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients->clients_msqids[i] == - 1) {
            clients->clients_msqids[i] = mq_open(q_name, O_RDONLY);
            if(clients->clients_msqids[i] < 0) {
                err_exit("Couldnt't open client's queue");
            }
            clients->clients_pids[i] = pid;
            return i;
        }
    }
    return -1;
}

static void remove_client(ClientInfo *clients, const int idx)
{
    clients->clients_msqids[idx] = -1;
}

static bool clients_connected(const ClientInfo *clients)
{
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients->clients_msqids[i] != -1) {
            return true;
        }
    }
    return false;
}

// Cleanup
void register_cleanup_functions(const CleanupFunction functions[], const int len)
{
    if(len > sysconf(_SC_ATEXIT_MAX)) {
        err_exit("Too many atexit functions");
    }
    for(int i = 0; i < len; i++) {
        if(atexit(functions[i]) < 0) {
            err_exit("Error: Couldnt set atexit function");
        }
    }
}


// MQ Functions
static void handle_conn_request(const ConnRequestBuf *connRequestBuf, ClientInfo *clients)
{
    printf("Got a connection request from PID: %d\n", connRequestBuf->conn_request.pid);
    int client_id = add_client(clients, connRequestBuf->conn_request.q_name, connRequestBuf->conn_request.pid);
    ConnResponseBuf responseBuf = {.type = CONN,.id = client_id};
    if(mq_send(clients->client_msqids[client_id], (char *)responseBuf, sizeof(ResponseBuf), 0L) < 0) {
        err_exit("MQ error");
    }
}

static void handle_echo_request(const EchoBuf *echoBuf, ClientInfo *clients)
{
    const mqd_t client_mq_id = clients->clients_msqids[echoBuf->msgInfo.client_id];
    const pid_t client_pid = clients->clients_pids[echoBuf->msgInfo.client_id];
    printf("Got an echo request from PID: %d, owning message queue with ID: %d\n", client_pid, client_msqid);
    if(mq_send(client_mq_id, (char *)echoBuf, sizeof(EchoBuf) 0) < 0) {
        err_exit("MQ error");
    }
}

static void handle_capitalize_request(CapitalizeBuf *capitalizeBuf, ClientInfo *clients)
{
    const mqd_t client_mq_id = clients->clients_msqids[capitalizeBuf->msgInfo.client_id];
    const pid_t client_pid = clients->clients_pids[capitalizeBuf->msgInfo.client_id];
    printf("Got a capitalize request from PID: %d, owning message queue with ID: %d\n", client_pid, client_msqid);
    for(int i = 0; capitalizeBuf->msgInfo.msg[i] != '\0'; i++) {
        capitalizeBuf->msgInfo.msg[i] = toupper(capitalizeBuf->msgInfo.msg[i]);
    }
    if(mq_send(client_mq_id, capitalizeBuf, sizeof(CapitalizeBuf), 0) < 0) {
        err_exit("MQ error");
    }
}

static void handle_time_request(const int client_id, ClientInfo *clients)
{
    const mqd_t client_mq_id = clients->clients_msqids[client_id];
    const pid_t client_pid = clients->clients_pids[client_id];
    printf("Got a datetime request from PID: %d, owning message queue with ID: %d\n", client_pid, client_msqid);

    TimeResponseBuf response = {.type = TIME};
    time_t timer;
    struct tm *time_info;

    time(&timer);
    time_info = localtime(&timer);

    strftime(response.time, sizeof(response.time), "%Y-%m-%d %H:%M:%S\n", time_info);

    if(mq_send(client_mq_id, &response, sizeof(TimeResponseBuf), 0) < 0) {
        err_exit("MQ error");
    }
}

static void dispatch_request(RequestBuf *buf, ClientInfo *clients, bool *shutdown)
{
    EchoBuf echoBuf;
    CapitalizeBuf capitalizeBuf;
    TimeRequestBuf timeRequestBuf;
    CloseConnBuf closeConnBuf;
    ConnRequestBuf connRequestBuf;
    int client_msqid;

    switch(buf->type) {
        case CONN:
            connRequestBuf.type = CONN;
            connRequestBuf.conn_request = buf->data.connRequest;
            handle_conn_request(&connRequestBuf, clients);
            break;
        case ECHO:
            echoBuf.type = ECHO;
            echoBuf.msgInfo = buf->data.msgInfo;
            handle_echo_request(&echoBuf, clients);
            break;
        case CAPITALIZE:
            capitalizeBuf.type = CAPITALIZE;
            capitalizeBuf.msgInfo = buf->data.msgInfo;
            handle_capitalize_request(&capitalizeBuf, clients);
            break;
        case TIME:
            timeRequestBuf.type = TIME;
            timeRequestBuf.client_id = buf->data.client_id;
            handle_time_request(timeRequestBuf.client_id, clients);
            break;
        case CLOSE:
            closeConnBuf.type = CLOSE;
            closeConnBuf.client_id = buf->data.client_id;
            printf(
                "Got a close request from PID: %d, owning message queue with ID %d\n",
                clients->clients_pids[closeConnBuf.client_id],
                clients->clients_msqids[closeConnBuf.client_id]    
            );
            if(mq_close(client->clients_msqids[closeConnBuf.client_id])) {
                err_exit("Couldnt close client queue");
            }
            remove_client(clients, closeConnBuf.client_id);
            *shutdown = true;
            break;
        default:
            err_exit("Fatal error, unknown message identifier");
    }
}

mqd_t create_public_queue(const char *public_q_pathname)
{
    mqd_t public_q_id = mq_open(public_q_pathname, O_RDONLY | O_CREAT | O_EXCL);
    if(public_q_id < 0) {
        err_exit("Couldnt create public MQ");
    }
    return public_q_id;
}

void listen(const mqd_t public_q_id)
{
    RequestBuf requestBuf;
    ClientInfo clients;
    ssize_t msg_ret_val;
    unsigned int msg_prio;
    bool shutdown = false;

    init_clients(&clients);

    while(!shutdown || clients_connected(&clients)) {
        memset(&requestBuf, 0, sizeof(RequestBuf));
        msg_ret_val = mq_receive(public_q_id, (char *)&requestBuf, sizeof(RequestBuf), &msg_prio);
        if(msg_ret_val > 0) {
            dispatch_request(&requestBuf, &clients, &shutdown);
        }
        else if(msg_ret_val < 0) {
            err_exit("MQ error");
        }
    }
}

