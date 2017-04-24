#ifndef MQ_CLIENT_H
#define MQ_CLIENT_H

// Typedefs
typedef void (*CleanupFunction)(void);

// Cleanup
void register_cleanup_functions(const CleanupFunction functions[], const int len);

// MQ functions
int get_public_q(void);
int create_private_q(void);
void send_conn_request(const int public_msqid, const int private_msqid);
int get_id(const int private_msqid);

void request_loop(const int public_msqid, const int private_msqid, const int client_id);
#endif