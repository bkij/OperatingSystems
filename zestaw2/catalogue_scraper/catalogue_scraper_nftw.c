#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ftw.h>
#include <time.h>
#include <errno.h>

int parse_args(char **argv, char **root_dir);
int scrape(char *root_dir);
int print_file_info(const char *pathname, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
void print_permissions(mode_t file_mode);
void print_modification_date(time_t modified_time);
void print_r();
void print_w();
void print_x();
void print_dash();

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

int scrape(char *root_dir)
{
    char *absolute_path = realpath(root_dir, NULL);
    if(absolute_path == NULL) {
        free(absolute_path);
        return -1;
    }
    if(nftw(absolute_path, print_file_info, 4, FTW_PHYS) < 0) {
        perror("nftw");
        free(absolute_path);
        return -1;
    }

    free(absolute_path);
    return 0;
}

int print_file_info(const char *pathname, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    if(typeflag == FTW_F && sb->st_size >= FILE_SIZE) {
        printf("File path: %s\n", pathname);
        printf("File size: %ld\n", sb->st_size);
        print_permissions(sb->st_mode);
        print_modification_date(sb->st_mtime);
    }
    return 0;
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