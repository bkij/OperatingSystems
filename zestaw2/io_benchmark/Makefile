CC=gcc
CFLAGS=-Wall -std=c99

.PHONY: clean

all: io_benchmark

io_benchmark: io_benchmark.c
	$(CC) $(CFLAGS) $^ -o $@.out

clean:
	rm -f *.o *.out
