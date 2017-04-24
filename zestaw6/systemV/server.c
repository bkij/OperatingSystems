#define _XOPEN_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mq_server.h"
#include "util.h"

/*
 * Global because the MQ needs to be cleaned up in case of
 * a premature termination (e.g by signal) 
 */
static int msqid;

void cleanup_q();

int main(int argc, char **argv)
{
    CleanupFunction functions[] = {cleanup_q};
    msqid = create_public_queue();
    register_cleanup_functions(functions, ARRAY_LEN(functions));
    listen(msqid);
    return 0;
}

void cleanup_q()
{
    if(msgctl(msqid, IPC_RMID, NULL) < 0) {
        err_exit("Couldnt clean-up message queue");
    }
}