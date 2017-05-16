#define _XOPEN_SOURCE
#include <sys/types.h>
#include "barber_fun.h"
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <signal.h>
#include "shared.h"
#include "util.h"


int create_barber_semaphore()
{
    key_t sem_key = ftok(BARBER_SEM_PATH, BARBER_SEM_KEY);
    int sem_id = semget(sem_key, BARBER_NSEMS, IPC_CREAT | IPC_EXCL | BARBER_SEM_PERM);
    if(sem_id < 0) {
        err_exit("Couldnt create barber's semaphore");
    }
    union semun semaphore_init = {.val = 0};
    if(semctl(sem_id, BARBER_SEM_NUM, SETVAL, semaphore_init) < 0) {
        err_exit("Couldnt't initialize barber's semaphore");
    }
    return sem_id;
}

int create_customers_semaphore()
{
    key_t sem_key = ftok(CUSTOMERS_SEM_PATH, CUSTOMERS_SEM_KEY);
    int sem_id = semget(sem_key, CUSTOMERS_NSEMS, IPC_CREAT | IPC_EXCL | CUSTOMERS_SEM_PERM);
    if(sem_id < 0) {
        err_exit("Couldtn create customers semaphore");
    }
    union semun semaphore_init = {.val = 0};
    if(semctl(sem_id, CUSTOMERS_SEM_NUM, SETVAL, semaphore_init) < 0) {
        err_exit("Couldnt initialize customers semaphore");
    }
    return sem_id;
}

int create_shm_semaphore()
{
    key_t sem_key = ftok(BINARY_SEM_PATH, BINARY_SEM_KEY);
    int sem_id = semget(sem_key, BINARY_NSEMS, IPC_CREAT | IPC_EXCL | BINARY_SEM_PERM);
    if(sem_id < 0) {
        err_exit("Couldnt create shared memory semaphore");
    }
    union semun semaphore_init = {.val = 1};
    if(semctl(sem_id, BINARY_SEM_NUM, SETVAL, semaphore_init) < 0) {
        err_exit("Couldnt initialize shm semaphore");
    }
    return sem_id;
}

struct shared_memory *create_memory(const int num_chairs, int *shmem_id)
{
    key_t shm_key = ftok(SHARED_MEM_FTOK_PATH, SHARED_MEM_FTOK_ID);
    int shm_id = shmget(shm_key, sizeof(struct shared_memory), IPC_CREAT | IPC_EXCL | SHM_PERM);
    if(shm_id < 0) {
        err_exit("Couldnt initialize shared memory");
    }
    *shmem_id = shm_id;
    return initialize_memory(shm_id, num_chairs);
}

struct shared_memory *initialize_memory(const int shm_id, const int num_chairs)
{
    struct shared_memory *shm = (struct shared_memory *)shmat(shm_id, NULL, 0);
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

void barber_loop(const int barber_sem_id, const int shm_sem_id, const int haircut_sem_id, struct shared_memory *shm)
{
    struct circular_queue *client_q = &shm->clients;
    pid_t current_pid;
    while(1) {
        print_goto_sleep();
        barber_sem_take(barber_sem_id);
        binary_sem_take(shm_sem_id);
        current_pid = shm->first_pid;
        binary_sem_give(shm_sem_id);
        print_haircut_start(current_pid);
        haircut_sem_give(haircut_sem_id);
        print_haircut_end(current_pid);
        //perform_haircut(current_pid);
        while(1) {
            binary_sem_take(shm_sem_id);
            current_pid = pop(client_q);
            if(current_pid == -1) {
                //barber_sem_take(barber_sem_id);
                binary_sem_give(shm_sem_id);
                break;
            }
            binary_sem_give(shm_sem_id);
            print_haircut_start(current_pid);
            haircut_sem_give(haircut_sem_id);
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
