CC=gcc
CFLAGS=-Wall -std=c99

.PHONY: clean

all: file_locker

file_locker: file_locker.c lock_manip.c
	$(CC) $(CFLAGS) $^ -o $@.out

clean:
	rm -f *.o *.out