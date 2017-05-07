#define _XOPEN_SOURCE
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include "mq_client.h"
#include "messaging.h"
#include "util.h"

/*
 * Global because the MQ needs to be cleaned up in case of
 * a premature termination (e.g by signal) 
 */
static char private_q_name[MAX_Q_NAME];

void cleanup_private(void);

int main(int argc, char **argv)
{
    const mqd_t public_msqid = get_public_q();
    const mqd_t private_msqid = create_private_q(private_q_name);

    CleanupFunction functions[] = {cleanup_private};
    register_cleanup_functions(functions, ARRAY_LEN(functions));
    
    send_conn_request(public_msqid, private_q_name);
    int client_id = get_id(private_msqid);
    request_loop(public_msqid, private_msqid, client_id);
    return 0;
}

void cleanup_private(void)
{
    if(mq_unlink(private_q_name) < 0) {
        err_exit("Couldnt cleanup private MQ");
    }
}