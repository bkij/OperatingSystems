//
// Created by kveld on 19.05.17.
//

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "sem_readers.h"
#include "../sem_commons/sem_commons.h"

void *read_array(void *arg)
{
    /*
     * Think about semaphores here
     * Writer is prioritized, he waits on
     * semaphore which is given as soon as another writer
     * finishes work
     */
    int divisor = rand() % 50 + 1;
    struct thread_arg *info = (struct thread_arg *)arg;
    while(true) {
        take_semaphore(info->rw_sem);
        int wr_sem_val = sem_val(info->writer_sem);
        if(wr_sem_val != 1) {
            give_semaphore(info->rw_sem);
            continue;
        }
        give_semaphore(info->reader_sem);
        give_semaphore(info->rw_sem);
        find_divisible(info->shared_array, info->verbose, divisor);
        take_semaphore(info->rw_sem);
        take_semaphore(info->reader_sem);
        give_semaphore(info->rw_sem);
    }
    return NULL;
}

void find_divisible(int *shared_array, bool verbose, int divisor)
{
    int divisible_cnt = 0;
    bool divisible[SHARED_ARR_LEN];
    for(int i = 0; i < SHARED_ARR_LEN; i++) {
        if(shared_array[i] % divisor == 0) {
            divisible_cnt++;
            divisible[i] = true;
        }
    }
    printf("Reader TID: %lu:\nFound %d numbers divisible by %d\n", pthread_self(), divisible_cnt, divisor);
    if(verbose) {
        printf("%s", "Numbers found: ");
        for(int i = 0; i < SHARED_ARR_LEN; i++) {
            if(divisible[i]) {
                printf("a[%d] - %d", i, shared_array[i]);
            }
        }
        printf("\n");
    }
}