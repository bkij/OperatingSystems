#include <sys/types.h>
#include <sys/ipc.h>
#include "barber_fun.h"
#include "shared_data.h"

int initialize_memory()
{
    key_t shm_key = ftok(SHARED_MEM_FTOK_PATH, SHARED_MEM_FTOK_ID);
    int shm_id = shmget(shm_key, ?, IPC_CREAT);
    if(shm_id < 0) {
        err_exit("Couldnt initialize shared memory");
    }
}

void barber_loop(const int shm_id, const int num_chairs)
{
    
}