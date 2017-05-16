#define _XOPEN_SOURCE
#include <stdio.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "shared.h"
#include "util.h"
#include "client_fun.h"

volatile sig_atomic_t current_haircuts = 0;
struct shared_memory *shm;

int main(int argc, char **argv)
{
    int max_haircuts = (int)*argv[1];
    shm = get_memory();
    sem_t *barber_sem_id = get_barber_sem();
    sem_t *shm_sem_id = get_shm_sem();
    sem_t *customers_sem_id = get_customers_sem();
    haircut_loop(shm, barber_sem_id, shm_sem_id, customers_sem_id, max_haircuts);
    return 0;
}

void inc_haircuts(int signo)
{
    current_haircuts++;
}