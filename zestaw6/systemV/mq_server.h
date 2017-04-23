#ifndef MQ_SERVER_H
#define MQ_SERVER_H

typedef void (*CleanupFunction)(void);

// Cleanup
void register_cleanup_functions(const CleanupFunction functions[], const int len);

// MQ functions
int create_public_queue(void);
void listen(const int msqid);

#endif