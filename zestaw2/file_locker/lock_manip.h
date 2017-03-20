#ifndef LOCK_MANIP_H
#define LOCK_MANIP_H

#include <stdbool.h>

void create_read_lock(int fd, int pos, bool blocking);
void create_write_lock(int fd, int pos, bool blocking);
void list_locks(int fd);
void lift_lock(int fd, int pos);
void read_byte(int fd, int pos);
void write_byte(int fd, int pos);

#endif