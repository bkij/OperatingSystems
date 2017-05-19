//
// Created by kveld on 19.05.17.
//

#include <stdio.h>
#include <stdlib.h>
#include "commons.h"

void take_mutex(pthread_mutex_t *mutex) {
    if(pthread_mutex_lock(mutex) < 0) {
        ERRNO_EXIT("Error locking no_writers_waiting");
    }
}

void give_mutex(pthread_mutex_t *mutex) {
    if(pthread_mutex_unlock(mutex) < 0) {
        ERRNO_EXIT("Error unlocking no_writers_waiting");
    }
}