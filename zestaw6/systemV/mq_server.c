#define _XOPEN_SOURCE
#include <sys/msg.h>
#include <sys/ipc.h>
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

// TODO: Get messages from public queue

// Helper
static void init_clients(ClientInfo *clients)
{
    clients->size = 0;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        clients->clients_msqids[i] = -1;
        clients->clients_pids[i] = -1;
    }
}

static int add_client(ClientInfo *clients, const int msqid, const pid_t client_pid)
{
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients->clients_msqids[i] == - 1) {
            clients->clients_msqids[i] = msqid;
            clients->clients_pids[i] = client_pid;
            return i;
        }
    }
    return -1;
}

static void remove_client(ClientInfo *clients, const int idx)
{
    clients->clients_msqids[idx] = -1;
    clients->clients_pids[idx] = -1;
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
    int client_id = add_client(clients, connRequestBuf->conn_request.msqid, connRequestBuf->conn_request.pid);
    ConnResponseBuf responseBuf = {.type = CONN,.id = client_id};
    if(msgsnd(clients->clients_msqids[client_id], &responseBuf, sizeof(ConnResponseBuf), IPC_NOWAIT) < 0 && errno != EAGAIN) {
        err_exit("MQ error");
    }
}

static void handle_echo_request(const EchoBuf *echoBuf, ClientInfo *clients)
{
    const int client_msqid = clients->clients_msqids[echoBuf->msgInfo.client_id];
    const pid_t client_pid = clients->clients_pids[echoBuf->msgInfo.client_id];
    printf("Got an echo request from PID: %d, owning message queue with ID: %d\n", client_pid, client_msqid);
    if(msgsnd(client_msqid, echoBuf, sizeof(EchoBuf), IPC_NOWAIT) < 0 && errno != EAGAIN) {
        err_exit("MQ error");
    }
}
static void handle_capitalize_request(CapitalizeBuf *capitalizeBuf, ClientInfo *clients)
{
    const int client_msqid = clients->clients_msqids[capitalizeBuf->msgInfo.client_id];
    const pid_t client_pid = clients->clients_pids[capitalizeBuf->msgInfo.client_id];
    printf("Got a capitalize request from PID: %d, owning message queue with ID: %d\n", client_pid, client_msqid);
    for(int i = 0; capitalizeBuf->msgInfo.msg[i] != '\0'; i++) {
        capitalizeBuf->msgInfo.msg[i] = toupper(capitalizeBuf->msgInfo.msg[i]);
    }
    if(msgsnd(client_msqid, capitalizeBuf, sizeof(CapitalizeBuf), IPC_NOWAIT) < 0 && errno != EAGAIN) {
        err_exit("MQ error");
    }
}
static void handle_time_request(const int client_id, ClientInfo *clients)
{
    const int client_msqid = clients->clients_msqids[client_id];
    const pid_t client_pid = clients->clients_pids[client_id];
    printf("Got a datetime request from PID: %d, owning message queue with ID: %d\n", client_pid, client_msqid);

    TimeResponseBuf response = {.type = TIME};
    time_t timer;
    struct tm *time_info;

    time(&timer);
    time_info = localtime(&timer);

    strftime(response.time, sizeof(response.time), "%Y-%m-%d %H:%M:%S\n", time_info);

    if(msgsnd(client_msqid, &response, sizeof(TimeResponseBuf), IPC_NOWAIT) < 0 && errno != EAGAIN) {
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
                "Got a close request from PID: %d with message queue ID: %d\n",
                clients->clients_pids[closeConnBuf.client_id], clients->clients_msqids[closeConnBuf.client_id]
            );
            remove_client(clients, closeConnBuf.client_id);
            *shutdown = true;
            break;
        default:
            err_exit("Fatal error, unknown message identifier");
    }
}

int create_public_queue(void)
{
    wordexp_t path_container;
    if(wordexp(PUBLIC_Q_PATH, &path_container, WRDE_UNDEF) < 0) {
        err_exit("Couldn't expand $HOME path");
    }
    key_t public_q_key = ftok(path_container.we_wordv[0], PUBLIC_Q_ID);
    int msqid = msgget(public_q_key, MSG_PERM | IPC_CREAT);
    if(msqid < 0) {
        err_exit("Error creating public message queue");
    }
    wordfree(&path_container);
    return msqid;
}

void listen(const int msqid)
{
    RequestBuf requestBuf;
    ClientInfo clients;
    int msg_ret_val;
    bool shutdown = false;

    init_clients(&clients);

    while(!shutdown || clients_connected(&clients)) {
        memset(&requestBuf, 0, sizeof(RequestBuf));
        msg_ret_val = msgrcv(msqid, &requestBuf, MAX_REQUEST_SIZE, 0L, IPC_NOWAIT);
        if(msg_ret_val > 0) {
            dispatch_request(&requestBuf, &clients, &shutdown);
        }
        else if(msg_ret_val < 0 && errno != ENOMSG) {
            err_exit("MQ error");
        }
    }
}

