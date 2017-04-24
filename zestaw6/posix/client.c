#define _XOPEN_SOURCE
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include "mq_client.h"
#include "util.h"

/*
 * Global because the MQ needs to be cleaned up in case of
 * a premature termination (e.g by signal) 
 */
static int private_msqid;
static int public_msqid;

void cleanup_private(void);

int main(int argc, char **argv)
{
    public_msqid = get_public_q();
    private_msqid = create_private_q();

    CleanupFunction functions[] = {cleanup_private};
    register_cleanup_functions(functions, ARRAY_LEN(functions));
    
    send_conn_request(public_msqid, private_msqid);
    int client_id = get_id(private_msqid);
    request_loop(public_msqid, private_msqid, client_id);
    return 0;
}

void cleanup_private(void)
{
    if(msgctl(private_msqid, IPC_RMID, NULL) < 0) {
        err_exit("Couldnt cleanup private MQ");
    }
}