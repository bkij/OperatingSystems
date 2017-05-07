#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <stdio.h>
#include "mq_server.h"
#include "util.h"

void cleanup_queues(void);
void free_pathnames(void);

int main(int argc, char **argv)
{
    CleanupFunction functions[] = {cleanup_queues};
    const mqd_t public_q_id = create_public_queue();
    register_cleanup_functions(functions, ARRAY_LEN(functions));
    listen(public_q_id);
    printf("%s\n", "No more clients, exiting...");
    return 0;
}

void cleanup_queues(void)
{
    if(mq_unlink(PUBLIC_Q_PATHNAME) < 0) {
        fprintf(stderr, "Coudlnt clean up message queue: %s\n", PUBLIC_Q_PATHNAME);
    }
}