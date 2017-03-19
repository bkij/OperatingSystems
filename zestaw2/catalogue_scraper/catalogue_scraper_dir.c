#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

int parse_args(char **argv, char **root_dir);
int scrape(char *root_dir);
int scrape_internal(char *absolute_root_path, char *current_dir);
void print_file_info(char *absolute_path, char *filename, struct stat *file_info);
void print_permissions(mode_t file_mode);
void print_modification_date(time_t modified_time);
void print_r();
void print_w();
void print_x();
void print_dash();
void close_dir(DIR *dir_stream);
void create_absolute_path(char *path_buffer, int buf_size, char *root_path, char *filename);

static int FILE_SIZE;

int main(int argc, char **argv)
{
    char *root_dir;
    int FILE_SIZE;
    if(argc < 3) {
        fprintf(stderr, "%s\n", "Not enough arguments.");
        return -1;
    }
    if(!parse_args(argv, &root_dir)) {
        return scrape(root_dir);
    }
    else {
        fprintf(stderr, "%s\n", "Error parsing arguments.");
        return -1;
    }
    return 0;
}

void print_r()
{
    printf("%s", "r");
}

void print_w()
{
    printf("%s", "w");
}

void print_x()
{
    printf("%s", "x");
}

void print_dash()
{
    printf("%s", "-");
}

void close_dir(DIR *dir_stream)
{
    if(closedir(dir_stream) < 0) {
        fprintf(stderr, "%s\n", "Error closing directory stream");
    }
}

int parse_args(char **argv, char **root_dir)
{
    *root_dir = argv[1];
    long tmp;
    tmp = strtol(argv[2], NULL, 10);
    if(errno == ERANGE || tmp <= 0 || tmp >= INT_MAX) {
        return -1;
    }
    FILE_SIZE = (int)tmp;
    return 0;
}


void create_absolute_path(char *path_buffer, int buf_size, char *root_path, char *filename)
{
    memset(path_buffer, 0, buf_size);
    strcat(path_buffer, root_path);
    strcat(path_buffer, "/");
    strcat(path_buffer, filename);
}

int scrape(char *root_dir)
{
    char path_buffer[1024];

    char *absolute_path = realpath(root_dir, NULL);
    if(absolute_path == NULL) {
        return -1;
    }

    DIR *dir_stream = opendir(absolute_path);
    if(dir_stream == NULL) {
        perror("opendir");
        free(absolute_path);
        return -1;
    }

    struct stat file_info;
    struct dirent *current_dir_info;
    int prev_errno = errno;
    while((current_dir_info = readdir(dir_stream))) {
        if(!strcmp(current_dir_info->d_name, ".") || !strcmp(current_dir_info->d_name, "..")) {
                continue;
        }
        create_absolute_path(path_buffer, sizeof(path_buffer), absolute_path, current_dir_info->d_name);
        int ret_code = stat(path_buffer, &file_info);
        if(!ret_code || errno == EACCES) {
            if(S_ISDIR(file_info.st_mode) && !S_ISLNK(file_info.st_mode)) {
                if(scrape(current_dir_info->d_name)) {
                    fprintf(stderr, "%s\n", "Error occurred in a nested directory");
                    close_dir(dir_stream);
                    free(absolute_path);
                    return -1;
                }
            }
            if(S_ISREG(file_info.st_mode) && !S_ISLNK(file_info.st_mode) && file_info.st_size >= FILE_SIZE) {
                print_file_info(absolute_path, current_dir_info->d_name, &file_info);
            }
        }
        else {
            perror("stat");
            close_dir(dir_stream);
            free(absolute_path);
            return -1;
        }
    }
    if(prev_errno != errno) {
        perror("readdir");
        close_dir(dir_stream);
        free(absolute_path);
        return -1;
    }
    
    close_dir(dir_stream);
    free(absolute_path);
    return 0;
}


void print_file_info(char *absolute_path, char *filename, struct stat *file_info)
{
    printf("File path: %s%s%s\n", absolute_path, "/", filename);
    printf("File size: %ld\n", file_info->st_size);
    print_permissions(file_info->st_mode);
    print_modification_date(file_info->st_mtime);
}

void print_permissions(mode_t file_mode)
{
    printf("%s: ", "Permissions");
    if(file_mode & S_IRUSR) {
        print_r();
    }
    else {
        print_dash();
    }
    if(file_mode & S_IWUSR) {
        print_w();
    }
    else {
        print_dash();
    }
    if(file_mode & S_IXUSR) {
        print_x();
    }
    else {
        print_dash();
    }

    if(file_mode & S_IRGRP) {
        print_r();
    }
    else {
        print_dash();
    }
    if(file_mode & S_IWGRP) {
        print_w();
    }
    else {
        print_dash();
    }
    if(file_mode & S_IXGRP) {
        print_x();
    }
    else {
        print_dash();
    }

    if(file_mode & S_IROTH) {
        print_r();
    }
    else {
        print_dash();
    }
    if(file_mode & S_IWOTH) {
       print_w();
    }
    else {
        print_dash();
    }
    if(file_mode & S_IXOTH) {
        print_x();
    }
    else {
        print_dash();
    }
    printf("\n");
}

void print_modification_date(time_t modified_time)
{
    struct tm *time_info = localtime(&modified_time);
    printf("%s %s\n", "Last modified: ", asctime(time_info));
}