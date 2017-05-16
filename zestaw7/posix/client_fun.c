#define _XOPEN_SOURCE 
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/shm.h>
#include "client_fun.h"
#include "util.h"
#include "shared.h"

extern volatile sig_atomic_t current_haircuts;

void haircut_loop(struct shared_memory *shm, sem_t *barber_sem_id, sem_t *shm_sem_id, sem_t *haircut_sem_id, const int max_haircuts)
{
    struct circular_queue *client_q = &shm->clients;
    while(current_haircuts < max_haircuts) {
        sem_wait(shm_sem_id);
        if(barber_awake(barber_sem_id)) {
            if(push(client_q, getpid())) {
                print_client_enqueued();
                sem_post(shm_sem_id);
                sem_wait(haircut_sem_id);
                current_haircuts++;
                print_leaving_barbershop();
            }
            else {
                print_no_seats();
                sem_post(shm_sem_id);
            }
        }
        else {
            sem_post(barber_sem_id);
            print_barber_woken();
            shm->first_pid = getpid();
            sem_post(shm_sem_id);
            sem_wait(haircut_sem_id);
            current_haircuts++;
            print_leaving_barbershop();
        }
    }
}

struct shared_memory *get_memory()
{
    int shm_id = shm_open(SHARED_MEM_FTOK_PATH, O_RDWR, 0600);
    struct shared_memory *shm = (struct shared_memory *)mmap(NULL, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    if(shm == (void *)-1) {
        err_exit("Couldnt open shared memory");
    }
    return shm;
}

sem_t *get_barber_sem()
{
    sem_t *sem = sem_open(BARBER_SEM_PATH, 0, 0600, 0);
    if(sem == SEM_FAILED) {
        err_exit("error creating semaphore");
    }
    return sem;
}

sem_t *get_shm_sem()
{
    sem_t *sem = sem_open(BINARY_SEM_PATH, 0, 0600, 1);
    if(sem == SEM_FAILED) {
        err_exit("error creating semaphore");
    }
    return sem;
}

sem_t *get_customers_sem()
{
    sem_t *sem = sem_open(CUSTOMERS_SEM_PATH, 0, 0600, 0);
    if(sem == SEM_FAILED) {
        err_exit("error creating semaphore");
    }
    return sem;
}

void print_barber_woken()
{
    printf("%lld : Barber woken up by PID %d\n", timestamp(), getpid());
}

void print_leaving_barbershop()
{
    printf("%lld : PID %d leaving barber shop after cut\n", timestamp(), getpid());
}

void print_client_enqueued()
{
    printf("%lld : PID %d enqueued\n", timestamp(), getpid());
}

void print_no_seats()
{
    printf("%lld : PID %d leaving barber shop - no seats\n", timestamp(), getpid());
}