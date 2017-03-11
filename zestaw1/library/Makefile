CC=gcc
CFLAGS=-Wall

all: libcontact_static libcontact_dynamic

libcontact_static: contact_book.c
	$(CC) $(CFLAGS) -c $^ -o $@.o
	ar crs $@.a $A.o

libcontact_dynamic: contact_book.c
	$(CC) $(CFLAGS) -fPIC -c $^ -o $@.o
	$(CC) -shared $@.o -o $@.so

clean:
	rm -f *.a *.o *.so
