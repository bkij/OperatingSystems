#ifndef CLIENT_FUN_H
#define CLIENT_FUN_H

#include <unistd.h>
#include "shared.h"

void haircut_loop(struct shared_memory *shm, const int barber_sem_id, const int shm_sem_id, const int max_haircuts);
struct shared_memory *get_memory();
int get_barber_sem();
int get_shm_sem();

void enqueue(struct shared_memory *shm, const int shm_sem_id);
void client_push(struct circular_queue *client_q, const int shm_sem_id);

void print_barber_woken();
void print_leaving_barbershop();
void print_client_enqueued();
void print_no_seats();

#endif