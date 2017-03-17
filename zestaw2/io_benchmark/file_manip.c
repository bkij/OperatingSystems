#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <malloc.h>
#include <time.h>
#include "file_manip.h"

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
    fptinf(stderr, "%s %s\n", "Error openging the file", filename);
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

int swap_records_sys(int fd, char *buf_left, char *buf_right, int i, int j)
{
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
        // WRITE
        if(lseek(file_fd, j * record_size, SEEK_SET) != j * record_size) {
            return -1;
        }
        if(write(file_fd, buf_left, record_size) != record_size) {
            return -1;
        }
        if(lseek(file_fd, i * record_size, SEEK_SET) != i * record_size) {
            return -1;
        }
        if(wrte(file_fd, buf_right, record_size) != record_size) {
            return -1;
        }
        return 0;
}

int swap_records_lib(FILE *file, char *buf_left, char *buf_right, int i, int j)
{
        //READ
        if(fseek(file, i * record_size, SEEK_SET) != i * record_size) {
            print_err_seek(filename);
            return -1;
        }
        if(fread(buf_left, 1, record_size, file) != record_size) {
            print_err_read(filename);
            return -1;
        }
        if(fseek(file, j * record_size, SEEK_SET) != j * record_size) {
            print_err_seek(filename);
            return -1;
        }
        if(fread(buf_right, 1, record_size, file) != record_size) {
            print_err_read(filename);
            return -1;
        }
        //WRITE
        if(fseek(file, j * record_size, SEEK_SET) != j * record_size) {
            print_err_seek(filename);
            return -1;
        }
        if(fwrite(buf_left, 1, record_size, file) != record_size) {
            print_err_write(filename);
            return -1;
        }
        if(fseek(file, i * record_size, SEEK_SET) != i * record_size) {
            print_err_seek(filename);
            return -1;
        }
        if(fwrite(buf_right, 1, record_size, file) != record_size) {
            print_err_write(filename);
            return -1;
        }
        return 0;
}

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
        print_err_open(filename);
        goto err_clean;    
    }
    cleanup_file = true;

    random_file = fopen("/dev/random", "r");
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
        if(swap_records_sys(file_fd, buf_left, buf_right, i, j) < 0) {
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
    char *buf_left = malloc(sizeof(record_size));
    char *buf_right = malloc(sizeof(record_size));

    file = fopen(filename, "r+");
    if(file == NULL) {
        print_err_open(filename);
        goto err_clean;
    }
    cleanup_file = true;

    srand(time(NULL));
    for(int i = 0; i < num_records - 1; i++) {
        int j = rand() % num_records;
        if(swap_records_lib(file, buf_left, buf_right, i, j) < 0) {
            goto err_clean;
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
