#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include "file_manip.h"

/*
 * HELPER FUNCTIONS
 */

void print_err_seek(char *filename)
{
    fprintf(stderr, "%s %s\n", "Error seeking in the file", filename);
}

void print_err_read(char *filename)
{
    fprintf(stderr, "%s %s\n", "Error reading from the file", filename);
}

void print_err_write(char *filename)
{
    fprintf(stderr, "%s %s\n", "Error writing to the file", filename);
}

void print_err_close(char *filename)
{
    fprintf(stderr, "%s %s\n", "Error closing the file", filename);
}

void print_err_open(char *filename)
{
    fprintf(stderr, "%s %s\n", "Error openging the file", filename);
}

bool enough_entropy_available(int entropy_needed)
{
    char *buf = malloc(256);
    FILE *file = fopen("/proc/sys/kernel/random/entropy_avail", "r");
    if(file == NULL) {
        fprintf(stderr, "%s\n", "Couldn't open entropy_avail");
        free(buf);
        return false;
    }
    if(fgets(buf, 256, file) < 0) {
        fprintf(stderr, "%s\n", "Couldn't read from entropy_avail");
        free(buf);
        return false;
    }
    long tmp = strtol(buf, NULL, 10);
    if(errno == ERANGE || tmp < 0 || tmp >= INT_MAX) {
        return false;
    }
    if(entropy_needed > (int)tmp) {
        fprintf(stderr, "%s\n", "Not enough entropy in /dev/random");
        free(buf);
        return false;
    }
    return true;
}

int execute(char *filename, int num_records, int record_size, Command command, FunctionType type)
{
    switch(command) {
        case do_generate:
            return generate(filename, num_records, record_size, type);
        case do_shuffle:
            return shuffle(filename, num_records, record_size, type);
        case do_sort:
            return sort(filename, num_records, record_size, type);
        default:
            fprintf(stderr, "%s\n", "Fatal error - unknown command type");
            return -1;
    }
}

int swap_records_sys(int file_fd, char *buf_left, char *buf_right, int i, int j, int record_size)
{
        if(lseek(file_fd, j * record_size, SEEK_SET) != j * record_size) {
            return -1;
        }
        if(write(file_fd, buf_left, record_size) != record_size) {
            return -1;
        }
        if(lseek(file_fd, i * record_size, SEEK_SET) != i * record_size) {
            return -1;
        }
        if(write(file_fd, buf_right, record_size) != record_size) {
            return -1;
        }
        return 0;
}

int swap_records_lib(FILE *file, char *buf_left, char *buf_right, int i, int j, int record_size, char *filename)
{
        if(fseek(file, j * record_size, SEEK_SET) < 0) {
            print_err_seek(filename);
            return -1;
        }
        if(fwrite(buf_left, 1, record_size, file) != record_size) {
            print_err_write(filename);
            return -1;
        }
        if(fseek(file, i * record_size, SEEK_SET) < 0) {
            print_err_seek(filename);
            return -1;
        }
        if(fwrite(buf_right, 1, record_size, file) != record_size) {
            print_err_write(filename);
            return -1;
        }
        return 0;
}

int cmp_records(char *record_left, char *record_right)
{
    // Compare first byte
    if((unsigned char)(*record_left) == (unsigned char)(*record_right)) {
        return 0;
    }
    else {
        return (unsigned char)(*record_left) > (unsigned char)(*record_right);
    }
}

/*
 * API and related functions
 */

int generate_sys(char *filename, int num_records, int record_size)
{
    int file_fd, random_fd;
    bool cleanup_random = false;
    bool cleanup_file = false;
    char *buf = malloc(num_records * record_size);
 
    file_fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if(file_fd < 0) {
        goto err_clean;
    }
    cleanup_file = true;

    random_fd = open("/dev/urandom", O_RDONLY);
    if(random_fd < 0) {
        goto err_clean;
    }
    cleanup_random = true;
    
    ssize_t read_bytes = read(random_fd, buf, num_records * record_size);
    if(read_bytes < sizeof(buf)) {
        if(read_bytes > 0) {
            errno = EAGAIN;
        }
        goto err_clean;
    }

    ssize_t written_bytes = write(file_fd, buf, num_records * record_size);
    if(written_bytes < sizeof(buf)) {
        if(written_bytes > 0) {
            errno = EIO;
        }
        goto err_clean;
    }

    if(close(file_fd) < 0 || close(random_fd) < 0) {
        perror("Error closing file");
    }
    free(buf);
    return 0;

err_clean:
    perror("Error in function generate");
    if(cleanup_file) {
        if(close(file_fd) < 0) {
            perror("Error closing file with given filename");
        }
    }
    if(cleanup_random) {
        if(close(random_fd) < 0) {
            perror("Error closing /dev/random");
        }
    }
    free(buf);
    return -1;
}

int generate_lib(char *filename, int num_records, int record_size)
{
    FILE *file, *random_file;
    bool cleanup_file = false;
    bool cleanup_random = false;
    char *buf = malloc(num_records * record_size);

    file = fopen(filename, "w+");
    if(file == NULL) {
        print_err_open(filename);
        goto err_clean;    
    }
    cleanup_file = true;

    random_file = fopen("/dev/urandom", "r");
    if(random_file == NULL) {
        goto err_clean;
    }
    cleanup_random = true;

    size_t read_bytes = fread(buf, sizeof(char), num_records * record_size, random_file);
    if(read_bytes < num_records * record_size) {
        print_err_read(filename);
        goto err_clean;
    }

    size_t written_bytes = fwrite(buf, sizeof(char), num_records * record_size, file);
    if(written_bytes < num_records * record_size) {
        print_err_write(filename);
        goto err_clean;
    }

    if(fclose(file) < 0 || fclose(random_file) < 0) {
        print_err_close(filename);
    }
    free(buf);
    return 0;

err_clean:
    if(cleanup_file) {
        if(fclose(file) < 0) {
            perror("Error closing file");
        }
    }
    if(cleanup_random) {
        if(fclose(random_file) < 0) {
            perror("Error closing file");
        }
    }
    free(buf);
    return -1;
}

int generate(char *filename, int num_records, int record_size, FunctionType type)
{
    //if(!enough_entropy_available(num_records * record_size)) {
    //    return -1;
    //}
    if(type == sys) {
        return generate_sys(filename, num_records, record_size);
    }
    else {
        return generate_lib(filename, num_records, record_size);
    }
}

int shuffle_sys(char *filename, int num_records, int record_size)
{
    int file_fd;
    bool cleanup_file = false;
    char *buf_left = malloc(record_size);
    char *buf_right = malloc(record_size);

    file_fd = open(filename, O_RDWR);
    if(file_fd < 0) {
        goto err_clean;
    }
    cleanup_file = true;

    srand(time(NULL));

    for(int i = 0; i < num_records - 1; i++) {
        int j = rand() % num_records;
        // READ
        if(lseek(file_fd, i * record_size, SEEK_SET) != i * record_size) {
            return -1;
        }
        if(read(file_fd, buf_left, record_size) != record_size) {
            return -1;
        }
        if(lseek(file_fd, j * record_size, SEEK_SET) != j * record_size) {
            return -1;
        }
        if(read(file_fd, buf_right, record_size) != record_size) {
            return -1;
        }
        //WRITE
        if(swap_records_sys(file_fd, buf_left, buf_right, i, j, record_size) < 0) {
            goto err_clean;
        }
    }

    free(buf_left);
    free(buf_right);
    if(close(file_fd) < 0) {
        perror("Error closing file");
    }
    return 0;

err_clean:
    perror("Error in function shuffle");
    free(buf_left);
    free(buf_right);
    if(cleanup_file && close(file_fd) < 0) {
        perror("Error closing file");
    }
    return -1;
}

int shuffle_lib(char *filename, int num_records, int record_size)
{
    FILE *file;
    bool cleanup_file = false;
    char *buf_left = malloc(record_size);
    char *buf_right = malloc(record_size);

    file = fopen(filename, "r+");
    if(file == NULL) {
        print_err_open(filename);
        goto err_clean;
    }
    cleanup_file = true;

    srand(time(NULL));
    for(int i = 0; i < num_records - 1; i++) {
        int j = rand() % num_records; 
        //READ
        if(fseek(file, i * record_size, SEEK_SET) < 0) {
            print_err_seek(filename);
            goto err_clean;
        }
        if(fread(buf_left, record_size, 1, file) != 1) {
            print_err_read(filename);
            goto err_clean;
        }
        if(fseek(file, j * record_size, SEEK_SET) < 0) {
            print_err_seek(filename);
            goto err_clean;
        }
        if(fread(buf_right, record_size, 1, file) != 1) {
            print_err_read(filename);
            goto err_clean;
        }
        //WRITE
        if(swap_records_lib(file, buf_left, buf_right, i, j, record_size, filename) < 0) {
            goto err_clean;
        }
    }

    if(cleanup_file && fclose(file) < 0) {
        print_err_close(filename);
    }
    free(buf_left);
    free(buf_right);
    return 0;

err_clean:
    perror("Encountered error while shuffling");
    if(cleanup_file && fclose(file) < 0) {
        print_err_close(filename);
    }
    free(buf_left);
    free(buf_right);
    return -1;
}

int shuffle(char *filename, int num_records, int record_size, FunctionType type)
{
    if(type == sys) {
        return shuffle_sys(filename, num_records, record_size);
    }
    else {
        return shuffle_lib(filename, num_records, record_size);
    }
}

int sort_sys(char *filename, int num_records, int record_size)
{
    int file_fd;
    bool cleanup_file = true;
    char *buf_left = malloc(record_size);
    char *buf_right = malloc(record_size);

    file_fd = open(filename, O_RDWR);
    if(file_fd < 0) {
        goto err_clean;
    }
    cleanup_file = true;

    for(int cnt = num_records; cnt > 1; cnt--) {
       for(int i = 0; i < cnt - 1; i++) {
           int j = i + 1;
           if(lseek(file_fd, i * record_size, SEEK_SET) != i * record_size) {
               return -1;
           }
           if(read(file_fd, buf_left, record_size) != record_size) {
               return -1;
           }
           if(lseek(file_fd, j * record_size, SEEK_SET) != j * record_size) {
               return -1;
           }
           if(read(file_fd, buf_right, record_size) != record_size) {
               return -1;
           }
           if(cmp_records(buf_left, buf_right) > 0) {
               if(swap_records_sys(file_fd, buf_left, buf_right, i, j, record_size)) {
                   goto err_clean;
               }
           }
       }
    }

    if(close(file_fd) < 0) {
        perror("Error closing file");
    }
    free(buf_left);
    free(buf_right);
    return 0;
err_clean:
    perror("Error in function sort");
    if(cleanup_file && close(file_fd) < 0) {
        perror("Error closing file");
    }
    free(buf_left);
    free(buf_right);
    return -1;
}

int sort_lib(char *filename, int num_records, int record_size)
{
    FILE *file;
    bool cleanup_file = false;
    char *buf_left = malloc(record_size);
    char *buf_right = malloc(record_size);

    file = fopen(filename, "r+");
    if(file == NULL) {
        goto err_clean;
    }
    cleanup_file = true;

    for(int cnt = num_records; cnt > 1; cnt--) {
        for(int i = 0; i < cnt - 1; i++) {
            int j = i + 1;
            if(fseek(file, i * record_size, SEEK_SET) < 0) {
                print_err_seek(filename);
                return -1;
            }
            if(fread(buf_left, 1, record_size, file) != record_size) {
                print_err_read(filename);
                return -1;
            }
            if(fseek(file, j * record_size, SEEK_SET) < 0) {
                print_err_seek(filename);
                return -1;
            }
            if(fread(buf_right, 1, record_size, file) != record_size) {
                print_err_read(filename);
                return -1;
            }
            if(cmp_records(buf_left, buf_right) > 0) {
                if(swap_records_lib(file, buf_left, buf_right, i, j, record_size, filename) < 0) {
                    goto err_clean;
                }
            }
        }
    }

    if(fclose(file) < 0) {
        print_err_close(filename);
    }
    free(buf_left);
    free(buf_right);
    return 0;
err_clean:
    if(cleanup_file && fclose(file) < 0) {
        print_err_close(filename);
    }
    free(buf_left);
    free(buf_right);
    return -1;
}

int sort(char *filename, int num_records, int record_size, FunctionType type)
{
    // Bubble sort
    if(type == sys) {
        return sort_sys(filename, num_records, record_size);
    }
    else {
        return sort_lib(filename, num_records, record_size);
    }
}
