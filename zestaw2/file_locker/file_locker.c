#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "lock_manip.h"

bool parse_arguments(char **argv, char **filename, bool *blocking);
void menu(int fd, bool blocking);
void print_options();
void read_byte(int fd, int pos);
void write_byte(int fd, int pos);

int main(int argc, char **argv)
{
    char *filename;
    bool blocking;
    srand(time(NULL));
    
    if(argc < 3) {
        printf("%s\n", "Not enough arguments. Please provide the filename and [blocking/nonblocking] argument");
    }
    if(!parse_arguments(argv, &filename, &blocking)) {
        fprintf(stderr, "%s\n", "Error parsing arguments");
        return -1;
    }

    int fd = open(filename, O_RDWR);
    if(fd < 0) {
        perror("open");
        return -1;
    }

    menu(fd, blocking);
    
    if(close(fd) < 0) {
        fprintf(stderr, "%s %s\n", "Couldn't close file:", filename);
    }

    return 0;
}

bool parse_arguments(char **argv, char **filename, bool *blocking)
{
    *filename = argv[1];
    if(!strcmp("blocking", argv[2])) {
        *blocking = true;
    }
    else if(!strcmp("nonblocking", argv[2])) {
        *blocking = false;
    }
    else {
        return false;
    }
    return true;
}

void menu(int fd, bool blocking)
{
    int choice;
    int pos;
    do {
        print_options();
        scanf("%d", &choice);
        switch(choice) {
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                printf("%s\n", "Please enter the byte position:");
                scanf("%d", &pos);
                read_byte(fd, pos);
                break;
            case 6:
                printf("%s\n", "Please enter the byte position:");
                scanf("%d", &pos);
                write_byte(fd, pos);
                break;
            case 7:
                printf("%s\n", "Exiting...");
                break;
            default:
                printf("%s\n", "Unknown option, please try again...");
                break;
        }
    } while(choice != 7);
}

void print_options()
{
    printf("%s\n", "1) Create a read lock");
    printf("%s\n", "2) Create a write lock");
    printf("%s\n", "3) Show lock list");
    printf("%s\n", "4) Lift a lock");
    printf("%s\n", "5) Read a byte from file");
    printf("%s\n", "6) Write a byte to file");
    printf("%s\n", "7) Quit");
}

void read_byte(int fd, int pos)
{
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