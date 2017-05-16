#define _XOPEN_SOURCE
#include <stdio.h>
#include <signal.h>
#include <sys/shm.h>
#include <stdlib.h>
#include "shared.h"
#include "util.h"
#include "client_fun.h"

volatile sig_atomic_t current_haircuts = 0;
volatile sig_atomic_t haircut_finished = 0;

void inc_haircuts(int signo);
void cleanup(void);

struct shared_memory *shm;

int main(int argc, char **argv)
{
    int max_haircuts = (int)*argv[1];
    shm = get_memory();
    int barber_sem_id = get_barber_sem();
    int shm_sem_id = get_shm_sem();
    int customers_sem_id = get_customers_sem();
    signal(SIGUSR1, inc_haircuts);
    sigset_t blocked;
    if(sigemptyset(&blocked) < 0) {
        err_exit("error");
    }
    if(sigaddset(&blocked, SIGUSR1) < 0) {
        err_exit("error");
    }
    if(sigprocmask(SIG_BLOCK, &blocked, NULL) < 0) {
        err_exit("error");
    }
    atexit(cleanup);
    haircut_loop(shm, barber_sem_id, shm_sem_id, customers_sem_id, max_haircuts);
    return 0;
}

void cleanup(void)
{
    if(shmdt((void *)shm) < 0) {
        err_exit("fatal error");
    }
}

void inc_haircuts(int signo)
{
    current_haircuts++;
}