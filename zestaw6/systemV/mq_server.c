#include "mq_server.h"
#include "messaging.h"

// Cleanup
void register_cleanup_functions(const CleanupFunction functions[], const int len)
{
    if(len > sysconf(_SC_ATEXIT_MAX)) {
        err_exit("Too many atexit functions");
    }
    for(int i = 0; i < len; i++) {
        if(atexit(functions[i]) < 0) {
            err_exit("Error: Couldnt set atexit function")
        }
    }
}


// MQ Functions
int create_public_queue(void)
{
    key_t public_q_key = ftok(PUBLIC_Q_PATH, PUBLIC_Q_ID);
    int msqid = msgget(public_q_key, IPC_CREAT);
    if(msqid < 0) {
        err_exit("Error creating public message queue");
    }
    return msqid;
}

void listen(const int msqid)
{
    
}
