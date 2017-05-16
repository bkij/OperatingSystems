#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include "util.h"
#include "shared.h"
#include "barber_fun.h"

void parse_args(char **argv, int *num_chairs);
void cleanup_exit(int signo);

int shm_id;
sem_t *barber_sem_id;
sem_t *shm_sem_id;
sem_t *haircut_sem_id;
struct shared_memory *shm;

int main(int argc, char **argv)
{
    int num_chairs;
    if(argc < 2) {
        fprintf(stderr, "%s\n", "Usage: ./sleepingbarber.out N");
        exit(EXIT_FAILURE);
    }
    parse_args(argv, &num_chairs);

    shm = create_memory(num_chairs, &shm_id);
    barber_sem_id = create_barber_semaphore();
    shm_sem_id = create_shm_semaphore();
    haircut_sem_id = create_customers_semaphore();
    signal(SIGINT, cleanup_exit);
    barber_loop(barber_sem_id, shm_sem_id, haircut_sem_id, shm);

    exit(EXIT_SUCCESS);
}

void cleanup_exit(int signo)
{
    if(shm_unlink(SHARED_MEM_FTOK_PATH) < 0) {
        err_exit("fatal error at cleanup");
    }
    if(sem_unlink(BARBER_SEM_PATH) < 0) {
        err_exit("fatal error");
    }
    if(sem_unlink(BINARY_SEM_PATH) < 0) {
        err_exit("fatal error");
    }
    if(sem_unlink(CUSTOMERS_SEM_PATH) < 0 ) {
        err_exit("fatal error");
    }
}

void parse_args(char **argv, int *num_chairs)
{
    long tmp = strtol(argv[1], NULL, 10);
    if(tmp <= 0 || tmp > MAX_SEATS || errno == ERANGE) {
        err_exit("Invalid N argument (integer; min 1 max 256)");
    }
    *num_chairs = (int)tmp;
}