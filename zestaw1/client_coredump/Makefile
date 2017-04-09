CC=gcc
CFLAGS=-Wall -std=c99

.PHONY: clean

all: client_static client_dynamic client_shared

client_static: client_coredump.c ../library/libcontact_static.a
	$(CC) $(CFLAGS) $^ -o $@.out

client_dynamic: client_coredump.c 
	$(CC) $(CFLAGS) $^ -o $@.out -L../library -lcontact_dynamic -Wl,-rpath=../library

client_shared: client_coredump.c
	$(CC) $(CFLAGS) $^ -o $@.out -ldl -D DYNAMIC

runAll: all
	./client_static.out -tree -list
	./client_dynamic.out -tree -list
	./client_shared.out -tree -list

runResults: all
	echo "STATIC LINKED\n" > results.txt
	./client_static.out -tree -list >> results.txt
	echo "DYNAMIC LINKED\n" >> results.txt
	./client_dynamic.out -tree -list>> results.txt
	echo "DYNAMIC SHARED\n" >> results.txt
	./client_shared.out -tree -list >> results.txt

runValgrind: all
	valgrind ./client_static.out -tree -list
	valgrind ./client_dynamic.out -tree -list
	valgrind ./client_shared.out -tree -list

o1: CFLAGS += -O1
o1: all

o2: CFLAGS += -O2
o2: all

o3: CFLAGS += -O3
o3: all

clean:
	rm -f *.out *.txt
