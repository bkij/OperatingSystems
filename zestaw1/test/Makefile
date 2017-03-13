CC=gcc
CFLAGS=-Wall -std=c99

all: rbtree_test list_test

rbtree_test: rbtree_test.c ../library/libcontact_static.a
	$(CC) $(CFLAGS) $^ -o $@.out

list_test: list_test.c ../library/libcontact_static.a
	$(CC) $(CFLAGS) $^ -o $@.out

runAll: all
	./rbtree_test.out
	./list_test.out

runValgrind: all
	valgrind ./rbtree_test.out
	valgrind ./list_test.out

clean:
	rm -f *.out
