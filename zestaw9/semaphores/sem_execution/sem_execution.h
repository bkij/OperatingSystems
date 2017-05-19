//
// Created by kveld on 19.05.17.
//

#ifndef ZESTAW9_SEM_EXECUTION_H
#define ZESTAW9_SEM_EXECUTION_H

#include <stdbool.h>
#include "../sem_commons/sem_commons.h"

void execute_threads(int *shared_array, int num_readers, int num_writers, bool verbose);
void init_rng();
void init_thread_arg(struct thread_arg *arg, int *shared_array, bool verbose);

#endif //ZESTAW9_SEM_EXECUTION_H
