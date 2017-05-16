#ifndef CLIENT_FUN_H
#define CLIENT_FUN_H

#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "shared.h"

void haircut_loop(struct shared_memory *shm, sem_t *barber_sem_id, sem_t *shm_sem_id, sem_t *customers_sem_id, const int max_haircuts);
struct shared_memory *get_memory();
sem_t *get_barber_sem();
sem_t *get_shm_sem();
sem_t *get_customers_sem();

void print_barber_woken();
void print_leaving_barbershop();
void print_client_enqueued();
void print_no_seats();

#endif