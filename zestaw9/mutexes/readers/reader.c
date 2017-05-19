//
// Created by kveld on 18.05.17.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "reader.h"
#include "../commons/commons.h"

/*
 * Writer favorizing variant
 *
 * Readers wait until no writer
 * is waiting to access the shared array
 */
void *read_array(void *arg) {
    // TODO: Think about synchronization
    int divisor = rand() % 50 + 1;
    struct thread_arg *info = (struct thread_arg *)arg;
    while(true) {
        take_mutex(info->cond_mutex);
        if (pthread_cond_wait(info->no_writers_waiting, info->cond_mutex) < 0) {
            ERRNO_EXIT("Mutex error");
        }
        info->readers_active++;
        give_mutex(info->cond_mutex);
        find_divisible(info->shared_array, info->verbose, divisor);
        take_mutex(info->cond_mutex);
        info->readers_active--;
        if(info->readers_active == 0) {
            pthread_cond_signal(info->no_readers_active);
        }
        give_mutex(info->cond_mutex);
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
