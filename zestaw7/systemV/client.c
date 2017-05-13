#include <stdio.h>
#include <signal.h>
#include "shared.h"
#include "client_fun.h"

volatile sig_atomic_t current_haircuts = 0;
volatile sig_atomic_t haircut_finished = 0;

void inc_haircuts(int signo);

int main(int argc, char **argv)
{
    int max_haircuts = (int)*argv[1];
    struct shared_memory *shm = get_memory();
    int barber_sem_id = get_barber_sem();
    int shm_sem_id = get_shm_sem();
    signal(SIGUSR1, inc_haircuts);
    haircut_loop(shm, barber_sem_id, shm_sem_id, max_haircuts);
    return 0;
}

void inc_haircuts(int signo)
{
    current_haircuts++;
    haircut_finished = 1;
}