#define _XOPEN_SOURCE 
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/shm.h>
#include "client_fun.h"
#include "util.h"
#include "shared.h"

extern volatile sig_atomic_t haircut_finished;
extern volatile sig_atomic_t current_haircuts;

void haircut_loop(struct shared_memory *shm, const int barber_sem_id, const int shm_sem_id, const int haircut_sem_id, const int max_haircuts)
{
    struct circular_queue *client_q = &shm->clients;
    while(current_haircuts < max_haircuts) {
        binary_sem_take(shm_sem_id);
        if(barber_awake(barber_sem_id)) {
            if(push(client_q, getpid())) {
                print_client_enqueued();
                binary_sem_give(shm_sem_id);
                haircut_sem_take(haircut_sem_id);
                current_haircuts++;
                print_leaving_barbershop();
            }
            else {
                print_no_seats();
                binary_sem_give(shm_sem_id);
            }
        }
        else {
            barber_sem_give(barber_sem_id);
            print_barber_woken();
            shm->first_pid = getpid();
            binary_sem_give(shm_sem_id);
            haircut_sem_take(haircut_sem_id);
            current_haircuts++;
            print_leaving_barbershop();
        }
    }
}

void wait_for_cut()
{
    sigset_t mask;
    if(sigfillset(&mask) < 0) {
        err_exit("Couldnt set mask value");
    }
    if(sigdelset(&mask, SIGUSR1) < 0) {
        err_exit("Coultdnt set mask value");
    }
    sigsuspend(&mask);
    print_leaving_barbershop();
}

struct shared_memory *get_memory()
{
    key_t shm_key = ftok(SHARED_MEM_FTOK_PATH, SHARED_MEM_FTOK_ID);
    int shm_id = -1;
    do {
        shm_id = shmget(shm_key, sizeof(struct shared_memory), 0);
    } while(shm_id < 0);
    struct shared_memory *shm = shmat(shm_id, NULL, 0);
    if(shm == (void *)-1) {
        err_exit("Couldnt map shared memory segment");
    }
    return shm;
}

int get_barber_sem()
{
    key_t sem_key = ftok(BARBER_SEM_PATH, BARBER_SEM_KEY);
    int sem_id = -1;
    do {
        sem_id = semget(sem_key, BARBER_NSEMS, 0);
    } while(sem_id < 0);
    return sem_id;
}

int get_shm_sem()
{
    key_t sem_key = ftok(BINARY_SEM_PATH, BINARY_SEM_KEY);
    int sem_id = -1;
    do {
        sem_id = semget(sem_key, BINARY_NSEMS, 0);
    } while(sem_id < 0);
    return sem_id;
}

int get_customers_sem()
{
    key_t sem_key = ftok(CUSTOMERS_SEM_PATH, CUSTOMERS_SEM_KEY);
    int sem_id = -1;
    do {
        sem_id = semget(sem_key, CUSTOMERS_NSEMS, 0);
    } while(sem_id < 0);
    return sem_id;
}

void enqueue(struct shared_memory *shm, const int shm_sem_id)
{
    struct circular_queue *client_q = &shm->clients;
    client_push(client_q, shm_sem_id);
}

void client_push(struct circular_queue *client_q, const int shm_sem_id)
{
    pid_t pid = getpid();
    if(push(client_q, pid)) {
        print_client_enqueued();
        binary_sem_give(shm_sem_id);
        while(!haircut_finished) {}
        haircut_finished = 0;
    }
    else {
        print_no_seats();
        binary_sem_give(shm_sem_id);
    }
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