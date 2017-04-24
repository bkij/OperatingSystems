#ifndef MESSAGING_H
#define MESSAGING_H

#define PUBLIC_Q_ID 'A'
#define PUBLIC_Q_PATH "$HOME"
#define MSG_PERM 0600

#define CONN 1L
#define ECHO 2L
#define CAPITALIZE 3L
#define TIME 4L
#define CLOSE 5L

#define MAX_MSG_SIZE 256
#define MAX_REQUEST_SIZE 300

typedef struct ConnRequest {
    int msqid;
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