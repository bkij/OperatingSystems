//
// Created by kveld on 18.05.17.
//

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "writer.h"
#include "../commons/commons.h"

void *write_to_array(void *arg) {
    int modification_cnt = rand() % SHARED_ARR_LEN + 1;
    struct thread_arg *info = (struct thread_arg *)arg;
    while(1) {
        take_mutex(info->cond_mutex);
        info->writers_waiting++;
        while(info->readers_active != 0) {
            if(pthread_cond_wait(info->no_readers_active, info->cond_mutex) < 0) {
                ERRNO_EXIT("Error waiting on condition");
            }
        }
        give_mutex(info->cond_mutex);
        take_mutex(info->writer_mutex);
        modify(info->shared_array, modification_cnt, info->verbose);
        give_mutex(info->writer_mutex);
        take_mutex(info->cond_mutex);
        info->writers_waiting--;
        if(info->writers_waiting == 0) {
            if(pthread_cond_broadcast(info->no_writers_waiting) < 0) {
                ERRNO_EXIT("Error signalling condition");
            }
        }
        give_mutex(info->cond_mutex);
    }
}

void modify(int *shared_array, int cnt, bool verbose)
{
    printf("Writer TID: %lu modifying shared array\n", pthread_self());
    for(int i = 0; i < cnt; i++) {
        int rand_idx = rand() % SHARED_ARR_LEN;
        int rand_val = rand();
        shared_array[rand_idx] = rand_val;
        if(verbose) {
            printf("Writer changed a[%d] to %d\n", rand_idx, rand_val);
        }
    }
}
