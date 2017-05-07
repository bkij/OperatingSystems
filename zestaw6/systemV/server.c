#define _XOPEN_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "mq_server.h"
#include "util.h"

/*
 * Global because the MQ needs to be cleaned up in case of
 * a premature termination (e.g by signal) 
 */
static int msqid;

void cleanup_q();
void handle_signals();
void cleanup_on_sig(int signo);

int main(int argc, char **argv)
{
    CleanupFunction functions[] = {cleanup_q};
    msqid = create_public_queue();
    register_cleanup_functions(functions, ARRAY_LEN(functions));
    handle_signals();
    listen(msqid);
    printf("%s\n", "No more clients, closing...");
    return 0;
}

void cleanup_q()
{
    if(msgctl(msqid, IPC_RMID, NULL) < 0) {
        err_exit("Couldnt clean-up message queue");
    }
}

void handle_signals()
{
    for(int i = 0; i < 32; i++) {
        signal(i, cleanup_on_sig);
    }
}

void cleanup_on_sig(int signo)
{
    cleanup_q();
    _exit(-1);
}