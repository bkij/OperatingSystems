#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/wait.h>
#include "lock_manip.h"

int get_file_size(int fd)
{
    struct stat file_info;
    if(fstat(fd, &file_info) < 0) {
        perror("Error getting file size");
        return -1;
    }
    return file_info.st_size;
}

void init_read_lock(struct flock *lock_info, int pos)
{
    lock_info->l_type = F_RDLCK;
    lock_info->l_whence = SEEK_SET;
    lock_info->l_start = pos;
    lock_info->l_len = 1;
    lock_info->l_pid = getpid();
}

void init_write_lock(struct flock *lock_info, int pos)
{
    lock_info->l_type = F_WRLCK;
    lock_info->l_whence = SEEK_SET;
    lock_info->l_start = pos;
    lock_info->l_len = 1;
    lock_info->l_pid = getpid();
}

void init_unlock(struct flock *lock_info, int pos)
{
    lock_info->l_type = F_UNLCK;
    lock_info->l_whence = SEEK_SET;
    lock_info->l_start = pos;
    lock_info->l_len = 1;
    lock_info->l_pid = getpid();
}

char *lock_type_to_string(short lock_type)
{
    if(lock_type == F_RDLCK) {
        return "RDLCK";
    }
    else if(lock_type == F_WRLCK) {
        return "WRLCK";
    }
    else {
        return "ERROR!";
    }
}

void print_lock_info(struct flock *lock_info)
{
    printf("%d\t\t%s\t%ld\n", lock_info->l_pid, lock_type_to_string(lock_info->l_type), lock_info->l_start);
}

void read_byte(int fd, int pos)
{
    struct flock lock_info;
    init_read_lock(&lock_info, pos);
    if(fcntl(fd, F_GETLK, &lock_info) < 0) {
        perror("Error checking lock");
        return;
    }
    if(lock_info.l_type != F_UNLCK) {
        printf("Byte %d is locked - can't read\n", pos);
        return;
    }

    unsigned char byte;
    if(lseek(fd, pos, SEEK_SET) != pos) {
        perror("Seek error while reading from file");
        return;
    }
    if(read(fd, &byte, 1) < 0) {
        perror("Read error");
        return;
    }
    printf("Byte: %x read at pos: %d\n", byte, pos);
}

void write_byte(int fd, int pos)
{
    struct flock lock_info;
    init_write_lock(&lock_info, pos);
    if(fcntl(fd, F_GETLK, &lock_info) < 0) {
        perror("Error checking lock");
        return;
    }
    if(lock_info.l_type != F_UNLCK) {
        printf("Byte %d is locked - can't write\n", pos);
        return;
    }

    unsigned char byte = rand();
    if(lseek(fd, pos, SEEK_SET) != pos) {
        perror("Seek error while writing to a file");
        return;
    }
    if(write(fd, &byte, 1) < 0) {
        perror("Write error");
        return;
    }
    printf("Byte: %x written at pos: %d\n", byte, pos);
}

void create_read_lock(int fd, int pos, bool blocking)
{
    struct flock lock_info;
    init_read_lock(&lock_info, pos);
    if(blocking) {
        printf("%s\n", "WARNING: This call can potentially block.");
        if(fcntl(fd, F_SETLKW, &lock_info) < 0) {
            perror("Error acquiring read lock");
        }
        printf("%s\n", "Lock acquired successfully");
    }
    else {
        if(fcntl(fd, F_SETLK, &lock_info) < 0) {
            perror("Error acquiring read lock");
        }
        else {
            printf("%s\n", "Lock acquired successfully");
        }
    }
}

void create_write_lock(int fd, int pos, bool blocking)
{
    struct flock lock_info;
    init_write_lock(&lock_info, pos);
    if(blocking) {
        printf("%s\n", "WARNING: This call can potentially block.");
        if(fcntl(fd, F_SETLKW, &lock_info) < 0) {
            perror("Error acquiring write lock");
        }
        printf("%s\n", "Lock acquired successfully");
    }
    else {
        if(fcntl(fd, F_SETLK, &lock_info) < 0) {
            perror("Error acquiring write lock");
        }
        else {
            printf("%s\n", "Lock acquired successfully");
        }
    }
}

void list_locks(int fd)
{
    struct flock lock_info;
    int file_size = get_file_size(fd);
    printf("%s\n", "LOCK LIST");
    printf("%s\t%s\t%s\n", "OwnerPID:", "Type:", "ByteNum:");

    int status;
    pid_t pid = fork();
    if(pid == 0) {
        for(int i = 0; i < file_size; i++) {
            init_write_lock(&lock_info, i);
            if(fcntl(fd, F_GETLK, &lock_info) < 0) {
                fprintf(stderr, "ERR AT BYTE %d\n", i);
                perror("Error reading lock info.");
            }
            else {
                if(lock_info.l_type != F_UNLCK) {
                    print_lock_info(&lock_info);   
                }
            }
        }
        if(close(fd) < 0) {
            perror("Error closing file in child process");
        }
        _exit(0);
    }
    else if(pid > 0) {
        wait(&status);
        if(WIFEXITED(status)) {
            assert(WEXITSTATUS(status) == 0);    
        }
    }
    else {
        perror("Fork error");
    }
    printf("\n");
}

void lift_lock(int fd, int pos)
{
    struct flock lock_info;
    init_unlock(&lock_info, pos);
    if(fcntl(fd, F_SETLK, &lock_info) < 0) {
        perror("Error unlocking lock");
    }
}