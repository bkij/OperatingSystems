//
// Created by kveld on 19.05.17.
//

#ifndef ZESTAW9_SEM_COMMONS_H
#define ZESTAW9_SEM_COMMONS_H

#include <stdbool.h>
#include <semaphore.h>

#define SHARED_ARR_LEN 256

#define ERRNO_EXIT(S) do {perror(S); exit(EXIT_FAILURE);} while(0)

struct thread_arg {
    int *shared_array;
    bool verbose;
    sem_t *writer_sem;
    sem_t *reader_sem;
    sem_t *rw_sem;
};

void take_semaphore(sem_t *sem);
void give_semaphore(sem_t *sem);
int sem_val(sem_t *sem);

#endif //ZESTAW9_SEM_COMMONS_H
