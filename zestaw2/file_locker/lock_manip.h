#ifndef LOCK_MANIP_H
#define LOCK_MANIP_H

#include <stdbool.h>

void create_read_lock(int fd, bool blocking);
void create_write_lock(int fd, bool blocking);
void list_locks(int fd);
void lift_lock();

#endif