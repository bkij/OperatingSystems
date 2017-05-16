#ifndef BARBER_FUN_H
#define BARBER_FUN_H

#include <sys/types.h>
#include <semaphore.h>

struct shared_memory *create_memory(const int num_chairs, int *shmem_id);
struct shared_memory *initialize_memory(const int shm_id, const int num_chairs);
sem_t *create_barber_semaphore();
sem_t *create_shm_semaphore();
sem_t *create_customers_semaphore();
void barber_loop(sem_t *barber_sem_id, sem_t *shm_sem_id, sem_t *customers_sem_id, struct shared_memory *shm);

void perform_haircut(const pid_t client_pid);

void print_haircut_start(const pid_t client_pid);
void print_haircut_end(const pid_t client_pid);
void print_wakeup();
void print_goto_sleep();
void print_client_gotten(const pid_t client_pid);

#endif