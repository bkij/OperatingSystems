#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "barber_fun.h"

void parse_args(char **argv, int *num_chairs);

int main(int argc, char **argv)
{
    int num_chairs;
    if(argc < 2) {
        fprintf(stderr, "%s\n", "Usage: ./sleepingbarber.out N");
        exit(EXIT_FAILURE);
    }
    parse_args(**argv, &num_chairs);

    struct shared_memory *shm = create_memory(num_chairs);
    int barber_sem_id = create_barber_semaphore();
    int shm_sem_id = create_shm_semaphore();
    barber_loop(barber_sem_id, shm_sed_id, shm);

    exit(EXIT_SUCCESS);
}

void parse_args(char **argv, int *num_chairs)
{
    long tmp = strtol(argv[1], NULL, 10);
    if(tmp <= 0 || tmp > MAX_SEATS || errno == ERANGE) {
        err_exit("Invalid N argument (integer; min 1 max 256)");
    }
    *num_chairs = (int)tmp;
}