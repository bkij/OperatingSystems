#define _XOPEN_SOURCE
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "mq_client.h"
#include "util.h"

/*
 * Global because the MQ needs to be cleaned up in case of
 * a premature termination (e.g by signal) 
 */
static int private_msqid;
static int public_msqid;

void cleanup_private(void);
void handle_signals(void);
void cleanup_on_sig(int signo);

int main(int argc, char **argv)
{
    public_msqid = get_public_q();
    private_msqid = create_private_q();

    CleanupFunction functions[] = {cleanup_private};
    register_cleanup_functions(functions, ARRAY_LEN(functions));
    handle_signals();

    send_conn_request(public_msqid, private_msqid);
    int client_id = get_id(private_msqid);
    if(client_id == -1) {
        fprintf(stderr, "%s\n", "Max connections to server reached, aborting...");
        exit(-1);
    }
    request_loop(public_msqid, private_msqid, client_id);
    return 0;
}

void cleanup_private(void)
{
    if(msgctl(private_msqid, IPC_RMID, NULL) < 0) {
        err_exit("Couldnt cleanup private MQ");
    }
}

void handle_signals(void)
{
    for(int i = 0; i < 32; i++) {
        signal(i, cleanup_on_sig);
    }
}

void cleanup_on_sig(int signo)
{
    cleanup_private();
    _exit(-1);
}