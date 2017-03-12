CC=gcc
CFLAGS=-Wall

all: rbtree_test list_test

rbtree_test: rbtree_test.c ../library/libcontact_static.a
	$(CC) $^ -o $@

list_test: list_test.c ../library/libcontact_static.a
	$(CC) $^ -o $@

runAll: all
	./rbtree_test
	./list_test
