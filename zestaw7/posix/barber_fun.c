#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include "barber_fun.h"
#include <semaphore.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include "shared.h"
#include "util.h"


sem_t *create_barber_semaphore()
{
    sem_t *sem = sem_open(BARBER_SEM_PATH, O_CREAT | O_EXCL, 0600, 0);
    if(sem == SEM_FAILED) {
        err_exit("error creating semaphore");
    }
    return sem;
}

sem_t *create_customers_semaphore()
{
    sem_t *sem = sem_open(CUSTOMERS_SEM_PATH, O_CREAT | O_EXCL, 0600, 0);
    if(sem == SEM_FAILED) {
        err_exit("error creating semaphore");
    }
    return sem;
}

sem_t *create_shm_semaphore()
{
    sem_t *sem = sem_open(BINARY_SEM_PATH, O_CREAT | O_EXCL, 0600, 1);
    if(sem == SEM_FAILED) {
        err_exit("error creating semaphore");
    }
    return sem;
}

struct shared_memory *create_memory(const int num_chairs, int *shmem_id)
{
    int shm_id = shm_open(SHARED_MEM_FTOK_PATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if(shm_id < 0) {
        err_exit("Couldnt initialize shared memory");
    }
    *shmem_id = shm_id;
    return initialize_memory(shm_id, num_chairs);
}

struct shared_memory *initialize_memory(const int shm_id, const int num_chairs)
{
    if(ftruncate(shm_id, sizeof(struct shared_memory)) < 0) {
        err_exit("ftruncate");
    }
    struct shared_memory *shm = (struct shared_memory *)mmap(NULL, sizeof(struct shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    if(shm == (void *)-1) {
        err_exit("Couldnt open shared memory");
    }
    shm->clients.capacity = num_chairs;
    shm->clients.idx_first = -1;
    shm->clients.idx_last = -1;
    for(int i = 0; i < num_chairs; i++) {
        shm->clients.clients[i] = -1;
    }
    return shm;
}

void barber_loop(sem_t *barber_sem_id, sem_t *shm_sem_id, sem_t *haircut_sem_id, struct shared_memory *shm)
{
    struct circular_queue *client_q = &shm->clients;
    pid_t current_pid;
    while(1) {
        print_goto_sleep();
        sem_wait(barber_sem_id);
        sem_wait(shm_sem_id);
        current_pid = shm->first_pid;
        sem_post(shm_sem_id);
        print_haircut_start(current_pid);
        sem_post(haircut_sem_id);
        print_haircut_end(current_pid);
        while(1) {
            sem_wait(shm_sem_id);
            current_pid = pop(client_q);
            if(current_pid == -1) {
                sem_post(shm_sem_id);
                break;
            }
            sem_post(shm_sem_id);
            print_haircut_start(current_pid);
            sem_post(haircut_sem_id);
            print_haircut_end(current_pid);
        }
    }
}

void perform_haircut(const pid_t client_pid)
{
    print_haircut_start(client_pid);
    kill(client_pid, SIGUSR1);
    print_haircut_end(client_pid);
}

void print_haircut_start(const pid_t client_pid)
{
    printf("%lld : Starting haircut on PID %d\n", timestamp(), client_pid);
}
void print_haircut_end(const pid_t client_pid)
{
    printf("%lld : Finishing haircut on PID %d\n", timestamp(), client_pid);
}
void print_wakeup()
{
    printf("%lld : %s\n", timestamp(), "Waking up");
}
void print_goto_sleep()
{
    printf("%lld : %s\n", timestamp(), "Going to sleep");
}
void print_client_gotten(const pid_t client_pid)
{
    printf("%lld : Client PID %d gotten from queue\n", timestamp(), client_pid);
}
