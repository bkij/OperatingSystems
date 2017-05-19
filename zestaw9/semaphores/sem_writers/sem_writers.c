//
// Created by kveld on 19.05.17.
//

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "sem_writers.h"
#include "../sem_commons/sem_commons.h"

void *write_to_array(void *arg)
{
    int modification_cnt = rand() % SHARED_ARR_LEN + 1;
    struct thread_arg *info = (struct thread_arg *)arg;
    while(true) {
        take_semaphore(info->rw_sem);
        int wr_cnt = sem_val(info->reader_sem);
        if(wr_cnt != 0) {
            give_semaphore(info->rw_sem);
            continue;
        }
        take_semaphore(info->writer_sem);
        give_semaphore(info->rw_sem);
        modify(info->shared_array, modification_cnt, info->verbose);
        take_semaphore(info->rw_sem);
        give_semaphore(info->writer_sem);
        give_semaphore(info->rw_sem);
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