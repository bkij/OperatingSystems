#ifndef MESSAGING_H
#define MESSAGING_H

#define PUBLIC_Q_PATHNAME "/public_q_1"
#define PRIVATE_Q_TEMPLATE "/private_q_x"
#define MSG_PERM 0600

#define CONN 1L
#define ECHO 2L
#define CAPITALIZE 3L
#define TIME 4L
#define CLOSE 5L

#define MAX_MSG_SIZE 256
#define MAX_REQUEST_SIZE 300
#define MAX_Q_NAME 64

typedef struct ConnRequest {
    char q_name[MAX_Q_NAME];
    pid_t pid;
} ConnRequest;

typedef struct ConnRequestBuf {
    long type;
    ConnRequest conn_request;
} ConnRequestBuf;

typedef struct ConnResponseBuf {
    long type;
    int id;
} ConnResponseBuf;

typedef struct MsgInfo {
    int client_id;
    char msg[MAX_MSG_SIZE];
} MsgInfo;

typedef struct EchoBuf {
    long type;
    MsgInfo msgInfo;
} EchoBuf;

typedef struct CapitalizeBuf {
    long type;
    MsgInfo msgInfo;
} CapitalizeBuf;

typedef struct TimeRequestBuf {
    long type;
    int client_id;
} TimeRequestBuf;

typedef struct TimeResponseBuf {
    long type;
    char time[21];
} TimeResponseBuf;

typedef struct CloseConnBuf {
    long type;
    int client_id;
} CloseConnBuf;

#endif