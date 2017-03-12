CC=gcc
CFLAGS=-Wall

all: libcontact_static_object libcontact_dynamic_object libcontact_static libcontact_dynamic

libcontact_static_object: contact_book.c contact_info.h
	$(CC) $(CFLAGS) -c $^

libcontact_static: contact_book.o contact_info.o
	ar crs $@.a $^

libcontact_dynamic_object: contact_book.c contact_info.c
	$(CC) $(CFLAGS) -fPIC -c $^

libcontact_dynamic: contact_book.o contact_info.o
	$(CC) -shared $^ -o $@.so

clean:
	rm -f *.a *.o *.so
