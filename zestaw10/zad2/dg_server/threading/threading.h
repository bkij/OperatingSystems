//
// Created by kveld on 30.05.17.
//

#ifndef ZESTAW10_THREADING_H
#define ZESTAW10_THREADING_H

#include <pthread.h>
#include "../sockets/sockets.h"
#include "../../common/common.h"

pthread_t create_pinging_thread(struct thread_shared_data *data);
pthread_t create_requests_thread(struct thread_shared_data *data);

#endif //ZESTAW10_THREADING_H
