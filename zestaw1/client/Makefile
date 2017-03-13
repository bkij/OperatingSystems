CC=gcc
CFLAGS=-Wall -std=c99

all: client_static client_dynamic client_shared

client_static: client.c ../library/libcontact_static.a
	$(CC) $(CFLAGS) $^ -o $@.out

client_dynamic: client.c 
	$(CC) $(CFLAGS) $^ -o $@.out -L../library -llibcontact_dynamic

client_shared: client.c
	$(CC) $(CFLAGS) $^ -o $@.out -ldl -D DYNAMIC

runAll:
	./client_static
	./client_dynamic
	./client_shared

runResults:
	echo -e "STATIC LINKED\n" > results.txt
	./client_static >> results.txt
	echo -e "DYNAMIC LINKED\n" > results.txt
	./client_dynamic >> results.txt
	echo -e "DYNAMIC SHARED\n" > results.txt
	./client_shared >> results.txt

runValgrind:
	valgrind ./client_static
	valgrind ./client_dynamic
	valgrind ./client_shared

clean:
	rm -f *.out
