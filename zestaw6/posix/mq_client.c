#define _XOPEN_SOURCE
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include "mq_client.h"
#include "util.h"
#include "messaging.h"

// Helper
static void print_menu()
{
    printf("%s\n", "1) ECHO");
    printf("%s\n", "2) CAPITALIZE");
    printf("%s\n", "3) DATETIME");
    printf("%s\n", "4) CLOSE");
}

static int get_choice()
{
    int s;
    char buf[sizeof(int)];
    do {
        printf("%s ", "Please enter a number from 1 to 4:");
        fgets(buf, sizeof(buf), stdin);
        if(sscanf(buf, "%d", &s) != 1) {
            s = -1;
        }
    } while(s < 1 || s > 4);
    return s;
}

static void get_user_input(char *s)
{
    fgets(s, MAX_MSG_SIZE, stdin);
}

// Cleanup
void register_cleanup_functions(const CleanupFunction functions[], const int len)
{
    for(int i = 0; i < len; i++) {
        atexit(functions[i]);
    }
}

// MQ functions
static void send_echo_request(EchoBuf *buf, const int public_msqid, const int client_id)
{
    buf->type = ECHO;
    buf->msgInfo.client_id = client_id;
    int msg_ret_val = 0;
    do {
        msg_ret_val = msgsnd(public_msqid, buf, sizeof(EchoBuf), 0);
        if(errno == EIDRM) {
            err_exit("Public MQ destroyed while trying to send a message");
        }
    } while(msg_ret_val < 0);
}

static void get_echo_response(EchoBuf *buf, const int private_msqid)
{
    int msg_ret_val = 0;
    do {
        msg_ret_val = msgrcv(private_msqid, buf, sizeof(EchoBuf), ECHO, 0);
    } while(msg_ret_val < 0);
}

static void send_capitalize_request(CapitalizeBuf *buf, const int public_msqid, const int client_id)
{
    buf->type = CAPITALIZE;
    buf->msgInfo.client_id = client_id;
    int msg_ret_val = 0;
    do {
        msg_ret_val = msgsnd(public_msqid, buf, sizeof(CapitalizeBuf), 0);
        if(errno == EIDRM) {
            err_exit("Public MQ destroyed while trying to send a message");
        }
    } while(msg_ret_val < 0);
}

static void get_capitalize_response(CapitalizeBuf *buf, const int private_msqid)
{
    int msg_ret_val = 0;
    do {
        msg_ret_val = msgrcv(private_msqid, buf, sizeof(CapitalizeBuf), CAPITALIZE, 0);
    } while(msg_ret_val < 0);
}

static void send_time_request(TimeRequestBuf *buf, const int public_msqid, const int client_id)
{
    buf->type = TIME;
    buf->client_id = client_id;
    int msg_ret_val = 0;
    do {
        msg_ret_val = msgsnd(public_msqid, buf, sizeof(TimeRequestBuf), 0);
        if(errno == EIDRM) {
            err_exit("Public MQ destroyed while trying to send a message");
        }
    } while(msg_ret_val < 0);
}

static void get_time_response(TimeResponseBuf *buf, const int private_msqid)
{
    int msg_ret_val = 0;
    do {
        msg_ret_val = msgrcv(private_msqid, buf, sizeof(TimeResponseBuf), TIME, 0);
    } while(msg_ret_val < 0);
}

static void send_close_request(CloseConnBuf *buf, const int public_msqid, const int client_id)
{
    buf->type = CLOSE;
    buf->client_id = client_id;
    int msg_ret_val = 0;
    do {
        msg_ret_val = msgsnd(public_msqid, buf, sizeof(CloseConnBuf), 0);
        if(errno == EIDRM) {
            err_exit("Public MQ destroyed while trying to send a message");
        }
    } while(msg_ret_val < 0);
}

int get_public_q(void)
{
    wordexp_t path_container;
    if(wordexp(PUBLIC_Q_PATH, &path_container, WRDE_UNDEF) < 0) {
        err_exit("Couldn't expand $HOME path");
    }
    key_t public_q_key = ftok(path_container.we_wordv[0], PUBLIC_Q_ID);
    int public_msqid = msgget(public_q_key, MSG_PERM);
    if(public_msqid < 0) {
        err_exit("Error opening public queue");
    }
    return public_msqid;
}

int create_private_q(void)
{
    int private_msqid = msgget(IPC_PRIVATE, MSG_PERM);
    if(private_msqid < 0) {
        err_exit("Error creating private queue");
    }
    return private_msqid;
}

void send_conn_request(const int public_msqid, const int private_msqid)
{
    ConnRequestBuf request = {.type = CONN, .conn_request = {.msqid = private_msqid, .pid = getpid()}};
    int msg_ret_val = 0;
    do {
        msg_ret_val = msgsnd(public_msqid, &request, sizeof(ConnRequestBuf), 0);
        if(errno == EIDRM) {
            err_exit("Public queue destroyed while trying to send a message");
        }
    } while(msg_ret_val < 0);
}
int get_id(const int private_msqid)
{
    ConnResponseBuf response;
    int msg_ret_val = 0;
    do {
        msg_ret_val = msgrcv(private_msqid, &response, sizeof(ConnResponseBuf), CONN, 0);
    } while(msg_ret_val < 0);
    return response.id;
}

void request_loop(const int public_msqid, const int private_msqid, const int client_id)
{
    EchoBuf echoBuf = {.type = ECHO, .msgInfo = {.client_id = client_id}};
    CapitalizeBuf capitalizeBuf = {.type = CAPITALIZE, .msgInfo = {.client_id = client_id}};
    TimeRequestBuf timeRequestBuf = {.type = TIME, .client_id = client_id};
    TimeResponseBuf timeResponseBuf = {.type = TIME};
    CloseConnBuf closeBuf = {.type = CLOSE, .client_id = client_id};
    int choice = 0;
    do {
        print_menu();
        choice = get_choice();
        switch(choice) {
            case 1:
                get_user_input(echoBuf.msgInfo.msg);
                send_echo_request(&echoBuf, public_msqid, client_id);
                get_echo_response(&echoBuf, private_msqid);
                printf("%s", echoBuf.msgInfo.msg);
                break;
            case 2:
                get_user_input(capitalizeBuf.msgInfo.msg);
                send_capitalize_request(&capitalizeBuf, public_msqid, client_id);
                get_capitalize_response(&capitalizeBuf, private_msqid);
                printf("%s", capitalizeBuf.msgInfo.msg);
                break;
            case 3:
                send_time_request(&timeRequestBuf, public_msqid, client_id);
                get_time_response(&timeResponseBuf, private_msqid);
                printf("%s", timeResponseBuf.time);
                break;
            case 4:
                send_close_request(&closeBuf, public_msqid, client_id);
                break;
            default:
                err_exit("Fatal error, unknown choice in switch statement");
        }
    } while(choice != 4);
}