#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include "util.h"
#include "reader_fun.h"

int parse_int(char *str, int min, int max)
{
    long tmp = strtol(str, NULL, 10);
    if(errno == ERANGE || tmp < min || tmp > max) {
        errno_exit("Invalid argument");
    }
    return (int)tmp;
}

void dispatch_threads(int num_threads, char *filename, int num_records, char *pattern, int type)
{
    FILE *input_file = fopen(filename, "r");
    if(input_file == NULL) {
        errno_exit("Couldnt open file");
    }
    switch(type) {
        case 1:
            threads_read_async(num_threads, input_file, num_records, pattern);
            break;
        case 2:
            threads_read_sync(num_threads, input_file, num_records, pattern);
            break;
        case 3:
            threads_read_detached(num_threads, input_file, num_records, pattern);
            break;
        default:
            fprintf(stderr, "%s\n", "FATAL ERROR! Unkown argument in switch");
            exit(EXIT_FAILURE);
    }
}

struct search_info *init_search_info(FILE *input_file, int num_records, char *pattern)
{
    struct search_info *info = malloc(sizeof(struct search_info));
    if(!info) {
        errno_exit("Malloc error");
    }
    info->fp = input_file;
    info->num_records = num_records;
    info->pattern = pattern;
    return info;
}

void free_search_info(struct search_info *info)
{
    free(info);
}

struct thread_info *init_thread_info(struct search_info *info, pthread_t *tids, int num_threads, pthread_mutex_t *mutex)
{
    struct thread_info *thread_info = malloc(sizeof(struct thread_info));
    if(!thread_info) {
        errno_exit("Malloc error");
    }
    thread_info->info = info;
    thread_info->tids = tids;
    thread_info->num_threads = num_threads;
    thread_info->mutex = mutex;
    return thread_info; 
}
void free_thread_info(struct thread_info *info)
{
    free(info);
}

void threads_read_async(int num_threads, FILE *input_file, int num_records, char *pattern)
{
    pthread_t *tids = malloc(MAX_THREADS * sizeof(pthread_t));
    memset(tids, 0, MAX_THREADS * sizeof(pthread_t));

    pthread_mutex_t *async_mutex = malloc(sizeof(pthread_mutex_t));
    init_mutex(async_mutex);

    struct search_info *search_info = init_search_info(input_file, num_records, pattern);
    struct thread_info *thread_info = init_thread_info(search_info, tids, num_threads, async_mutex);
    
    for(int i = 0; i < num_threads; i++) {
        int ret_value = pthread_create(&tids[i], NULL, async_search, (void *)thread_info);
        if(ret_value < 0) {
            errno_exit("Error creating thread");
        }
    }
    for(int i = 0; i < num_threads; i++) {
        if(pthread_join(tids[i], NULL)) {
            errno_exit("Error joining with a thread");
        }
    }
}

void *async_search(void *arg)
{
    struct thread_info *info = (struct thread_info *)arg;
    struct search_info *search_info = info->info;

    const int bufsize = search_info->num_records * RECORD_SIZE;
    char *data_buffer = malloc(bufsize);
    
    while(fread(data_buffer, bufsize, 1, search_info->fp) != 0) {
        if(search_pattern(data_buffer, bufsize, search_info->pattern) == PATTERN_FOUND) {
            acquire(info->mutex);
            cancel_threads(info->tids, info->num_threads);
            release(info->mutex);
            pthread_exit((void *)PATTERN_FOUND);
        }
    }
    printf("Thread %lu - no pattern found\n", pthread_self());
    pthread_exit((void *)NO_PATTERN);
}

void cancel_threads(pthread_t *tids, int num_threads)
{
    for(int i = 0; i < num_threads; i++) {
        if(tids[i] != pthread_self()) {
            if(pthread_cancel(tids[i]) < 0) {
                errno_exit("Error cancelling thread");
            }
        }
    }
}

void init_mutex(pthread_mutex_t *mutex)
{
    if(pthread_mutex_init(mutex, NULL) < 0) {
        errno_exit("Couldnt initialize mutex");
    }
}

void free_mutex(pthread_mutex_t *mutex)
{
    if(pthread_mutex_destroy(mutex) < 0) {
        errno_exit("Couldnt free mutex");
    }
    free(mutex);
}

void acquire(pthread_mutex_t *mutex)
{
    if(pthread_mutex_lock(mutex) < 0) {
        errno_exit("Mutex error on lock");
    }
}

void release(pthread_mutex_t *mutex)
{
    if(pthread_mutex_unlock(mutex) < 0) {
        errno_exit("Mutex error on unlock");
    }
}

int search_pattern(char *data_buffer, int bufsize, char *pattern)
{
    const int text_size = RECORD_SIZE - sizeof(int);
    if(strlen(pattern) > text_size) {
        return NO_PATTERN;
    }
    int status = NO_PATTERN;
    int multiplier = 1;
    int i = 0;
    char *text_slice = data_buffer;
    while(text_slice < data_buffer + bufsize) {
        text_slice = text_slice + sizeof(int);
        status = search_in_slice(text_slice, data_buffer + RECORD_SIZE * multiplier, pattern, i);
        text_slice += text_size;
        if(status == PATTERN_FOUND) {
            printf("Pattern %s found by thread %lu\n", pattern, pthread_self());
            break;
        }
        multiplier++;
    }
    return status;
}

int search_in_slice(char *slice, char *end, char *pattern, int i)
{
    int status = NO_PATTERN;
    int j = strlen(pattern) - 1;
    while(slice + j != end) {
        if(slice[i] == pattern[0] && slice[j] == pattern[strlen(pattern) - 1]) {
            if(check_match(slice, pattern, i + 1, j - 1) == PATTERN_FOUND) {
                status = PATTERN_FOUND;
                break;
            }
        }
        i++;
        j++;
    }
    return status;
}

int check_match(char *slice, char *pattern, int i, int j)
{
    int status = PATTERN_FOUND;
    int k = 0;
    while(i <= j) {
        if(slice[i] != pattern[k]) {
            status = NO_PATTERN;
            break;
        }
        i++;
        k++;
    }
    return status;
}

void threads_read_sync(int num_threads, FILE *input_file, int num_records, char *pattern)
{
    pthread_t *tids = malloc(MAX_THREADS * sizeof(pthread_t));
    memset(tids, 0, MAX_THREADS * sizeof(pthread_t));

    pthread_mutex_t *async_mutex = malloc(sizeof(pthread_mutex_t));
    init_mutex(async_mutex);

    struct search_info *search_info = init_search_info(input_file, num_records, pattern);
    struct thread_info *thread_info = init_thread_info(search_info, tids, num_threads, async_mutex);
    
    for(int i = 0; i < num_threads; i++) {
        int ret_value = pthread_create(&tids[i], NULL, sync_search, (void *)thread_info);
        if(ret_value < 0) {
            errno_exit("Error creating thread");
        }
    }
    for(int i = 0; i < num_threads; i++) {
        if(pthread_join(tids[i], NULL)) {
            errno_exit("Error joining with a thread");
        }
    }
}

void *sync_search(void *arg)
{
    enable_cancel();
    struct thread_info *info = (struct thread_info *)arg;
    struct search_info *search_info = info->info;

    const int bufsize = search_info->num_records * RECORD_SIZE;
    char *data_buffer = malloc(bufsize);
    while(1) {
        disable_cancel();
        if(fread(data_buffer, bufsize, 1, search_info->fp) == 0) {
            break;
        }
        int search_status = search_pattern(data_buffer, bufsize, search_info->pattern);
        enable_cancel();
        test_cancel();
        if(search_status == PATTERN_FOUND) {
            acquire(info->mutex);
            cancel_threads(info->tids, info->num_threads);
            release(info->mutex);
            pthread_exit((void *)PATTERN_FOUND);
        }
    }
    printf("Thread %lu - no pattern found\n", pthread_self());
    pthread_exit((void *)NO_PATTERN);
}


void enable_cancel()
{
    if(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) {
        errno_exit("Error setting cancel state");
    }
}

void disable_cancel()
{
    if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL)) {
        errno_exit("Error setting cancel state");
    }
}

void test_cancel()
{
    pthread_testcancel();
}

void threads_read_detached(int num_threads, FILE *input_file, int num_records, char *pattern)
{
    pthread_t *tids = malloc(MAX_THREADS * sizeof(pthread_t));
    memset(tids, 0, MAX_THREADS * sizeof(pthread_t));

    pthread_mutex_t *async_mutex = malloc(sizeof(pthread_mutex_t));
    init_mutex(async_mutex);

    pthread_attr_t *attr = malloc(sizeof(pthread_attr_t));
    init_detached(attr);

    struct search_info *search_info = init_search_info(input_file, num_records, pattern);
    struct thread_info *thread_info = init_thread_info(search_info, tids, num_threads, async_mutex);
    
    for(int i = 0; i < num_threads; i++) {
        int ret_value = pthread_create(&tids[i], NULL, detached_search, (void *)thread_info);
        if(ret_value < 0) {
            errno_exit("Error creating thread");
        }
    }
    while(1) {}
}

void *detached_search(void *arg)
{
    int status = NO_PATTERN;

    struct thread_info *info = (struct thread_info *)arg;
    struct search_info *search_info = info->info;

    const int bufsize = search_info->num_records * RECORD_SIZE;
    char *data_buffer = malloc(bufsize);
    
    while(fread(data_buffer, bufsize, 1, search_info->fp) != 0) {
        if(search_pattern(data_buffer, bufsize, search_info->pattern) == PATTERN_FOUND) {
            status = PATTERN_FOUND;
        }
    }
    if(status == NO_PATTERN) {
        printf("Thread %lu - no pattern found\n", pthread_self());
    }
    pthread_exit((void *)0);
}


void init_detached(pthread_attr_t *attr)
{
    if(pthread_attr_init(attr)) {
        errno_exit("Couldnt initialize thread attribute");
    }
    if(pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED)) {
        errno_exit("Couldnt initialize thread attribute");
    }
}