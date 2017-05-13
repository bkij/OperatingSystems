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

void haircut_loop(struct shared_memory *shm, const int barber_sem_id, const int shm_sem_id, const int max_haircuts)
{
    while(current_haircuts < max_haircuts) {
        // Check barber status
        struct sembuf status_check = {BARBER_SEM_NUM, BARBER_WAIT, SEM_UNDO | IPC_NOWAIT};
        if(semop(barber_sem_id, &status_check, 1) < 0) {
            if(errno == EAGAIN) {
                enqueue(shm, shm_sem_id);
            }
            else {
                err_exit("Barber semaphore error");
            }
        }
        else {
            print_barber_woken();
            while(!haircut_finished) {}
            haircut_finished = 0;
            print_leaving_barbershop();
        }
    }
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

void enqueue(struct shared_memory *shm, const int shm_sem_id)
{
    struct circular_queue *client_q = &shm->clients;
    client_push(client_q, shm_sem_id);
}

void client_push(struct circular_queue *client_q, const int shm_sem_id)
{
    pid_t pid = getpid();
    if(binary_sem_take(shm_sem_id) < 0) {
        err_exit("Binary semaphore error");
    }
    if(push(client_q, pid)) {
        print_client_enqueued();
        if(binary_sem_give(shm_sem_id) < 0) {
            err_exit("Binary semaphore error");
        }
        while(!haircut_finished) {}
        haircut_finished = 0;
    }
    else {
        print_no_seats();
        if(binary_sem_give(shm_sem_id) < 0) {
            err_exit("Binary semaphore error");
        }
    }
}

void print_barber_woken()
{
    print_timestamp();
    printf("Barber woken up by PID %d\n", getpid());
}

void print_leaving_barbershop()
{
    print_timestamp();
    printf("PID %d leaving barber shop\n", getpid());
}

void print_client_enqueued()
{
    print_timestamp();
    printf("PID %d enqueued\n", getpid());
}

void print_no_seats()
{
    print_timestamp();
    printf("PID %d leaving barber shop - no seats\n", getpid());
}