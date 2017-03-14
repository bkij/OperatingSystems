CC=gcc
CFLAGS=-Wall -std=c99

all: client_static client_dynamic client_shared

client_static: client.c ../library/libcontact_static.a
	$(CC) $(CFLAGS) $^ -o $@.out

client_dynamic: client.c 
	$(CC) $(CFLAGS) $^ -o $@.out -L../library -lcontact_dynamic -Wl,-rpath=../library

client_shared: client.c
	$(CC) $(CFLAGS) $^ -o $@.out -ldl -D DYNAMIC

runAll:
	./client_static.out -tree -list
	./client_dynamic.out -tree -list
	./client_shared.out -tree -list

runResults:
	echo "STATIC LINKED\n" > results.txt
	./client_static.out -tree -list >> results.txt
	echo "DYNAMIC LINKED\n" >> results.txt
	./client_dynamic.out -tree -list>> results.txt
	echo "DYNAMIC SHARED\n" >> results.txt
	./client_shared.out -tree -list >> results.txt

runValgrind:
	valgrind ./client_static.out
	valgrind ./client_dynamic.out
	valgrind ./client_shared.out

clean:
	rm -f *.out *.txt
