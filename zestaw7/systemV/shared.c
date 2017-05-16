#include "shared.h"
#include "util.h"

/*
 *  CIRCULAR QUEUE FUNCTIONS
 */

bool is_empty(struct circular_queue *q)
{
    return q->idx_first == -1;
}

bool push(struct circular_queue *q, pid_t data)
{
    int next_idx = (q->idx_last + 1) % q->capacity;
    if(q->clients[next_idx] != -1) {
        return false;
    }
    q->idx_last = next_idx;
    if(q->idx_first == -1) {
        q->idx_first = q->idx_last;
    }
    q->clients[q->idx_last] = data;
    return true;
}

pid_t pop(struct circular_queue *q)
{
    if(q->idx_first == -1) {
        return -1;
    }
    pid_t return_data = q->clients[q->idx_first];
    q->clients[q->idx_first] = -1;
    q->idx_first = (q->idx_first + 1) % q->capacity;
    if(q->clients[q->idx_first] == -1) {
        q->idx_first = -1;
    }
    if(q->clients[q->idx_last] == -1) {
        q->idx_last = -1;
    }
    return return_data;
}

bool atomic_push(struct circular_queue *q, pid_t data, int binary_sem_id)
{
    binary_sem_take(binary_sem_id);
    bool push_ret_val = push(q, data);
    binary_sem_give(binary_sem_id);
    return push_ret_val;
}

pid_t atomic_pop(struct circular_queue *q, int binary_sem_id)
{
    binary_sem_take(binary_sem_id);
    pid_t pop_ret_val = pop(q);
    binary_sem_give(binary_sem_id);
    return pop_ret_val;
}

/*
 *  BINARY SEMAPHORE FUNCTIONS
 */

 void binary_sem_take(const int binary_sem_id)
 {
    struct sembuf take = {BINARY_SEM_NUM, SEM_TAKE, SEM_UNDO};
    if(semop(binary_sem_id, &take, 1) < 0) {
        err_exit("Binary semaphore error");
    }
 }

 void binary_sem_give(const int binary_sem_id)
 {
     struct sembuf give = {BINARY_SEM_NUM, SEM_GIVE, SEM_UNDO};
     if(semop(binary_sem_id, &give, 1) < 0) {
         err_exit("Binary semaphore error");
     }
 }

/*
 *  BARBER SEMAPHORE FUNCTIONS
 */
void barber_sem_take(const int barber_sem_id)
{
    struct sembuf take = {BARBER_SEM_NUM, BARBER_WAIT, SEM_UNDO};
    if(semop(barber_sem_id, &take, 1) < 0) {
        err_exit("Barber semaphore error");
    }
}
void barber_sem_give(const int barber_sem_id)
{
    struct sembuf give = {BARBER_SEM_NUM, BARBER_SIGNAL, SEM_UNDO};
    if(semop(barber_sem_id, &give, 1) < 0) {
        err_exit("Barber semaphore error");
    }
}

bool barber_awake(const int barber_sem_id)
{
    return semctl(barber_sem_id, BARBER_SEM_NUM, GETVAL) > 0;
}

void haircut_sem_take(const int cli_sem_id)
{
    struct sembuf take = {CUSTOMERS_SEM_NUM, CUSTOMERS_TAKE, SEM_UNDO};
    if(semop(cli_sem_id, &take, 1) < 0) {
        err_exit("Customers semaphore error");
    }
}

void haircut_sem_give(const int cli_sem_id)
{
    struct sembuf give = {CUSTOMERS_SEM_NUM, CUSTOMERS_GIVE, SEM_UNDO};
    if(semop(cli_sem_id, &give, 1) < 0) {
        err_exit("Customers semaphore error");
    }
}