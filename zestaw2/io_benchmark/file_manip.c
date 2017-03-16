#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <malloc.h>
#include "file_manip.h"

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

int generate_sys(char *filename, int num_records, int record_size)
{
    int file_fd, random_fd;
    bool cleanup_random = false;
    bool cleanup_file = false;
    char *buf = malloc(num_records * record_size);
        
    file_fd = open(filename, O_CREAT, S_IRUSR | S_IWUSR);
    if(file_fd < 0) {
        goto err_clean;
    }
    cleanup_file = true;

    random_fd = open("/dev/random", O_RDONLY);
    if(random_fd < 0) {
        goto err_clean;
    }
    cleanup_random = true;
    
    ssize_t read_bytes = read(random_fd, buf, sizeof(buf));
    if(read_bytes < sizeof(buf)) {
        if(read_bytes > 0) {
            errno = EAGAIN;
        }
        goto err_clean;
    }

    ssize_t written_bytes = write(file_fd, buf, sizeof(buf));
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
        goto err_clean;    
    }
    cleanup_file = true;

    random_file = fopen("/dev/random", "r");
    if(random_file == NULL) {
        goto err_clean;
    }
    cleanup_random = true;

    size_t read_bytes = fread(buf, sizeof(char), sizeof(buf), random_file);
    if(read_bytes < sizeof(buf)) {
        fprintf(stderr, "%s\n", "Error while reading /dev/random");
        goto err_clean;
    }

    size_t written_bytes = fwrite(buf, sizeof(char), sizeof(buf), file);
    if(written_bytes < sizeof(buf)) {
        fprintf(stderr, "%s\n", "Error while writing to file");
        goto err_clean;
    }

    if(fclose(file) < 0 || fclose(random_file) < 0) {
        fprintf(stderr, "%s\n", "Error closing a file");
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
    if(type == sys) {
        return generate_sys(filename, num_records, record_size);
    }
    else {
        return generate_lib(filename, num_records, record_size);
    }
}
