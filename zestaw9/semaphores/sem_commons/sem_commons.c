//
// Created by kveld on 19.05.17.
//

#include <stdio.h>
#include <stdlib.h>
#include "sem_commons.h"


void take_semaphore(sem_t *sem)
{
    if(sem_wait(sem) < 0) {
        ERRNO_EXIT("Semaphore error (wait)");
    }
}

void give_semaphore(sem_t *sem)
{
    if(sem_post(sem) < 0) {
        ERRNO_EXIT("Semaphore error (post)");
    }
}

int sem_val(sem_t *sem)
{
    int val;
    if(sem_getvalue(sem, &val) < 0) {
        ERRNO_EXIT("Couldnt get semaphore value");
    }
    return val;
}