//
// Created by kveld on 18.05.17.
//

#ifndef ZESTAW9_EXECUTION_H
#define ZESTAW9_EXECUTION_H

#include <stdbool.h>

void execute_threads(int *shared_array, int num_readers, int num_writers, bool verbose);
void init_thread_arg(struct thread_arg *arg, int *shared_array, bool verbose);
void init_rng();

#endif //ZESTAW9_EXECUTION_H
