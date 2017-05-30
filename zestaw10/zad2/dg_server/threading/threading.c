//
// Created by kveld on 30.05.17.
//

#include "threading.h"

pthread_t create_remote_listening_thread(struct socket_info *socket_info) {
    int sockfd = mk_nonblocking_remote_sock(socket_info->port_num)
}
