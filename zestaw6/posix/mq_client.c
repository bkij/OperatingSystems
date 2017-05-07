#define _XOPEN_SOURCE
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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
static void send_echo_request(EchoBuf *buf, const mqd_t public_msqid, const int client_id)
{
    buf->type = ECHO;
    buf->msgInfo.client_id = client_id;
    if(mq_send(public_msqid, (char *)buf, sizeof(EchoBuf), 0) < 0) {
        err_exit("couldnt send echo request");
    }
}

static void get_echo_response(MsgBuf *buf, const mqd_t private_q_id)
{
    if(mq_receive(private_q_id, (char *)buf, sizeof(MsgBuf), NULL) < 0) {
        err_exit("couldnt get echo response");
    }
}

static void send_capitalize_request(CapitalizeBuf *buf, const mqd_t public_msqid, const int client_id)
{
    buf->type = CAPITALIZE;
    buf->msgInfo.client_id = client_id;
    if(mq_send(public_msqid, (char *)buf, sizeof(CapitalizeBuf), 0) < 0) {
        err_exit("couldnt send capitalize request");
    }
} 

static void get_capitalize_response(MsgBuf *buf, const mqd_t private_q_id)
{
    if(mq_receive(private_q_id, (char *)buf, sizeof(MsgBuf), NULL) < 0) {
        err_exit("Couldnt get capitalize response");
    }
}

static void send_time_request(TimeRequestBuf *buf, const mqd_t public_msqid, const int client_id)
{
    buf->type = TIME;
    buf->client_id = client_id;
    if(mq_send(public_msqid, (char *)buf, sizeof(TimeRequestBuf), 0) < 0) {
        err_exit("Couldnt send time response");
    }
}

static void get_time_response(MsgBuf *buf, const mqd_t private_q_id)
{
    if(mq_receive(private_q_id, (char *)buf, sizeof(MsgBuf), NULL) < 0) {
        err_exit("Couldnt get time response");
    }
}

static void send_close_request(CloseConnBuf *buf, const mqd_t public_msqid, const int client_id)
{
    buf->type = CLOSE;
    buf->client_id = client_id;
    if(mq_send(public_msqid, (char *)buf, sizeof(CloseConnBuf), 0) < 0) {
        err_exit("Couldnt send close request");
    }
}

mqd_t get_public_q(void)
{
    mqd_t public_q_id = mq_open(PUBLIC_Q_PATHNAME, O_WRONLY);
    if(public_q_id < 0) {
        err_exit("Couldnt open public queue");
    }
    return public_q_id;
}

mqd_t create_private_q(char *private_q_name)
{
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 40;
    attr.mq_msgsize = sizeof(EchoBuf);
    attr.mq_curmsgs = 0;
    mqd_t private_q_id;
    char name_buf[] = PRIVATE_Q_TEMPLATE;
    for(char c = 'a'; c < 'z'; c++) {
        name_buf[strlen(name_buf) - 1] = c;
        private_q_id = mq_open(name_buf, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);
        if(private_q_id > 0) {
            strcpy(private_q_name, name_buf);
            break;
        }
    }
    if(private_q_id < 0) {
        err_exit("Couldnt create private queue");
    }
    return private_q_id;
}

void send_conn_request(const mqd_t public_msqid, const char *private_q_name)
{
    ConnRequestBuf request = {.type = CONN, .conn_request = {.pid = getpid()}};
    strcpy(request.conn_request.q_name, private_q_name);
    if(mq_send(public_msqid, (char *)&request, sizeof(ConnRequestBuf), 0) < 0) {
        err_exit("Couldnt send conn request");
    }
}

int get_id(const mqd_t private_q_id)
{
    MsgBuf response;
    if(mq_receive(private_q_id, (char *)&response, sizeof(MsgBuf), NULL) < 0) {
        err_exit("Couldnt receive conn response");
    }
    ConnResponseBuf buf = response.data.connResponseBuf;
    return buf.id;
}

void request_loop(const mqd_t public_msqid, const mqd_t private_q_id, const int client_id)
{
    EchoBuf echoBuf = {.type = ECHO, .msgInfo = {.client_id = client_id}};
    CapitalizeBuf capitalizeBuf = {.type = CAPITALIZE, .msgInfo = {.client_id = client_id}};
    TimeRequestBuf timeRequestBuf = {.type = TIME, .client_id = client_id};
    CloseConnBuf closeBuf = {.type = CLOSE, .client_id = client_id};
    MsgBuf responseBuf;
    int choice = 0;
    do {
        print_menu();
        choice = get_choice();
        switch(choice) {
            case 1:
                get_user_input(echoBuf.msgInfo.msg);
                send_echo_request(&echoBuf, public_msqid, client_id);
                get_echo_response(&responseBuf, private_q_id);
                printf("%s", responseBuf.data.msgInfo.msg);
                break;
            case 2:
                get_user_input(capitalizeBuf.msgInfo.msg);
                send_capitalize_request(&capitalizeBuf, public_msqid, client_id);
                get_capitalize_response(&responseBuf, private_q_id);
                printf("%s", responseBuf.data.msgInfo.msg);
                break;
            case 3:
                send_time_request(&timeRequestBuf, public_msqid, client_id);
                get_time_response(&responseBuf, private_q_id);
                printf("%s", responseBuf.data.time);
                break;
            case 4:
                send_close_request(&closeBuf, public_msqid, client_id);
                break;
            default:
                err_exit("Fatal error, unknown choice in switch statement");
        }
    } while(choice != 4);
}