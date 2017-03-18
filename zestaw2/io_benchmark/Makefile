CC=gcc
CFLAGS=-Wall -std=c99 -g

.PHONY: clean

all: io_benchmark

io_benchmark: io_benchmark.c file_manip.c
	$(CC) $(CFLAGS) $^ -o $@.out

clean:
	rm -f *.o *.out
