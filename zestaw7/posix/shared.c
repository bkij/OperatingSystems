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

bool barber_awake(sem_t *barber_sem)
{
    int val;
    if(sem_getvalue(barber_sem, &val) < 0) {
        err_exit("error");
    }
    return val > 0;
}