#ifndef READER_H
#define READER_H

#include <stdio.h>
#include <pthread.h>

#define MAX_THREADS 20
#define MAX_RECORDS 10

#define RECORD_SIZE 1024

#define PATTERN_FOUND 0
#define NO_PATTERN 1

#define MAX_PATTERN_SIZE 1020
#define MAX_FILENAME_SIZE 256

struct search_info {
    FILE *fp;
    int num_records;
    char *pattern;
};

struct thread_info {
    struct search_info *info;
    pthread_t *tids;
    int num_threads;
    pthread_mutex_t *mutex;
};

// HELPER
int parse_int(char *str, int min, int max);

// STRUCT MANAGEMENT
struct search_info *init_search_info(FILE *input_file, int num_records, char *pattern);
void free_search_info(struct search_info *info);

struct thread_info *init_thread_info(struct search_info *info, pthread_t *tids, int num_threads, pthread_mutex_t *mutex);
void free_thread_info(struct thread_info *info);

// MUTEX WRAPPERS
void init_mutex(pthread_mutex_t *mutex);
void free_mutex(pthread_mutex_t *mutex);
void acquire(pthread_mutex_t *mutex);
void release(pthread_mutex_t *mutex);

// CANCELLATION WRAPPERS
void enable_cancel();
void disable_cancel();
void test_cancel();

// ATTRIBUTE WRAPPERS
void init_detached(pthread_attr_t *attr);

// THREAD FUNCTIONS
void dispatch_threads(int num_threads, char *filename, int num_records, char *pattern, int type);

void threads_read_async(int num_threads, FILE *input_file, int num_records, char *pattern);
void *async_search(void *arg);
void cancel_threads(pthread_t *tids, int num_threads);

void threads_read_sync(int num_threads, FILE *input_file, int num_records, char *pattern);
void *sync_search(void *arg);

void threads_read_detached(int num_threads, FILE *input_file, int num_records, char *pattern);
void *detached_search(void *arg);

// SEARCHING
int search_pattern(char *data_buffer, int bufsize, char *pattern);
int search_in_slice(char *slice, char *end, char *pattern, int i);
int check_match(char *slice, char *pattern, int i, int j);

#endif