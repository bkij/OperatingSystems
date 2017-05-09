#include "shared.h"

/*
 *  CIRCULAR QUEUE FUNCTIONS
 */

bool is_empty(struct circular_queue *q)
{
    return q->idx_first == -1;
}

bool push(struct circular_queue *q, pid_t data)
{
    if((q->idx_last + 1) % q->capacity != -1) {
        return false;
    }
    q->idx_last = (q->idx_last + 1) % q->capacity;
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
    q->clients[idx_first] = -1;
    q->idx_first = (q->idx_first + 1) % q->capacity;
    if(q->clients[idx_first] == -1) {
        q->idx_first = -1;
    }
    if(q->clients[idx_last] == -1) {
        q->idx_last = -1;
    }
    return return_data;
}

bool atomic_push(struct circular_queue *q, pid_t data, int binary_sem_id)
{
    if(binary_sem_take(binary_sem_id) < 0) {
        err_exit("Binary semaphore error");
    }
    bool push_ret_val = push(q, data);
    if(binary_sem_give(binar_sem_id) < 0) {
        err_exit("Binary semaphore error");
    }
    return push_ret_val;
}

pid_t atomic_pop(struct circular_q *q, int binary_sem_id)
{
    if(binary_sem_take(binary_sem_id) < 0) {
        err_exit("Binary semaphore error");
    }
    pid_t pop_ret_val = pop(q);
    if(binary_sem_give(binar_sem_id) < 0) {
        err_exit("Binary semaphore error");
    }
    return pop_ret_val;
}

/*
 *  BINARY SEMAPHORE FUNCTIONS
 */

 int binary_sem_take(const int binary_sem_id)
 {
    struct sembuf wait_and_take[2];
    wait_and_take[0] = {BINARY_SEM_NUM, SEM_WAIT_ZERO, SEM_UNDO};
    wait_and_take[1] = {BINARY_SEM_NUM, SEM_TAKE, SEM_UNDO};
    return semop(binary_sem_id, &wait_and_take, ARR_LEN(wait_and_take));
 }

 int binary_sem_give(const int binary_sem_id)
 {
     struct sembuf give = {BINARY_SEM_NUM, SEM_GIVE, SEM_UNDO};
     return semop(binary_sem_id, &give, 1);
 }