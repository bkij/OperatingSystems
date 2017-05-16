#ifndef SHARED_DATA_H
#define SHARED_DATA_H
 
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_SEATS 256
#define MAX_CLIENTS 256
#define MAX_HAIRCUTS 128

// SHARED MEM
#define SHARED_MEM_FTOK_PATH "/homeA"
#define SHARED_MEM_FTOK_ID 'A'
#define SHM_PERM 0600


// SEMAPHORES
#define BARBER_SEM_PATH "/homeC"
#define BARBER_SEM_KEY 'C'
#define BARBER_SEM_PERM 0600

#define BARBER_NSEMS 1
#define BARBER_SEM_NUM 0
#define BARBER_WAIT -1
#define BARBER_SIGNAL 1
#define BARBER_WAIT_ZERO 0

#define BINARY_SEM_PATH "/homeB"
#define BINARY_SEM_KEY 'B'
#define BINARY_SEM_PERM 0600

#define BINARY_NSEMS 1
#define BINARY_SEM_NUM 0
#define SEM_WAIT_ZERO 0
#define SEM_TAKE -1
#define SEM_GIVE 1

#define CUSTOMERS_SEM_PATH "/homeD"
#define CUSTOMERS_SEM_KEY 'D'
#define CUSTOMERS_SEM_PERM 0600

#define CUSTOMERS_SEM_NUM 0
#define CUSTOMERS_NSEMS 1
#define CUSTOMERS_TAKE -1
#define CUSTOMERS_GIVE 1

#include <unistd.h>
#include <stdbool.h>

// Union declaration for semctl

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

// Circular queues

struct circular_queue {
    pid_t clients[MAX_SEATS];
    int capacity;
    int idx_first;
    int idx_last;
};

struct shared_memory {
    struct circular_queue clients;
    pid_t first_pid;
};

bool is_empty(struct circular_queue *q);
bool push(struct circular_queue *q, pid_t data);
pid_t pop(struct circular_queue *q);
bool barber_awake(sem_t *barber_sem);

#endif