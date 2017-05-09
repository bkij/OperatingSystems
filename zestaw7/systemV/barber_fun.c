#include <sys/types.h>
#include <sys/ipc.h>
#include "barber_fun.h"
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared.h"


int create_barber_semaphore()
{
    key_t sem_key = ftok(BARBER_SEM_PATH, BARBER_SEM_KEY);
    int sem_id = semget(sem_key, BARBER_NSEMS, IPC_CREAT | IPC_EXCL | BARBER_SEM_PERM);
    if(sem_id < 0) {
        err_exit("Couldnt create barber's semaphore");
    }
    union semun semaphore_init = {.val = 0};
    if(semop(sem_id, BARBER_SEM_NUM, semaphore_init) < 0) {
        err_exit("Couldnt't initialize barber's semaphore");
    }
    return sem_id;
}

int create_shm_semaphore()
{
    key_t sem_key = ftok(BINARY_SEM_PATH, BINARY_SEM_KEY);
    int sem_id = semget(sem_key, SHM_NSEMS, IPC_CREAT | IPC_EXCL | BINARY_SEM_PERM);
    if(sem_id < 0) {
        err_exit("Couldnt create shared memory semaphore");
    }
    union semun semaphore_init = {.val = 0};
    if(semop(sem_id, BINARY_SEM_NUM, semaphore_init) < 0) {
        err_exit("Couldnt initialize shm semaphore");
    }
    return sem_id;
}

int create_memory(const int num_chairs)
{
    key_t shm_key = ftok(SHARED_MEM_FTOK_PATH, SHARED_MEM_FTOK_ID);
    int shm_id = shmget(shm_key, sizeof(struct shared_memory), IPC_CREAT | IPC_EXCL | SHM_PERM);
    if(shm_id < 0) {
        err_exit("Couldnt initialize shared memory");
    }
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
    shm_>clients.idx_last = -1;
    return shm;
}

void barber_loop(const int barber_sem_id, const int shm_sem_id, struct shared_memory *shm)
{
    struct circular_queue *client_q = &shm->clients;
    pid_t current_pid;
    while((current_pid = barber_sem_take_with_pid(barber_sem_id)) != -1) {
        print_wakeup();
        // Cut client who woke barber up
        print_haircut_start(current_pid);
        perform_haircut(current_pid);
        print_haircut_end(current_pid);
        // Get clients from Q in a loop
        while((current_pid = atomic_pop(client_q)) != -1) {
            // If client gotten successfully, cut hair
            print_client_gotten(current_pid);
            print_haircut_start(current_pid);
            perform_haircut(current_pid);
            print_haircut_end(current_pid);
        }
        print_goto_sleep();
    }
}

void perform_haircut(const pid_t client_pid)
{
    kill(client_pid, SIGUSR1);
}

void print_haircut_start(const pid_t client_pid)
{
    print_timestamp();
    printf("Starting haricut on PID %d\n", client_pid);
}
void print_haircut_end(const pid_t client_pid)
{
    print_timestamp();
    printf("Finishing haircut on PID %d\n", client_pid);
}
void print_wakeup()
{
    print_timestamp();
    printf("%s\n","Waking up");
}
void print_goto_sleep()
{
    print_timestamp();
    printf("%s\n", "Going to sleep");
}
void print_client_gotten(const pid_t client_pid)
{
    print_timestamp();
    printf("Client PID %d gotten from queue\n", client_pid);
}
