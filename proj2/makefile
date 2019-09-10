CC=gcc
CFLAGS=-I.
DEPS = prodcomm.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellomake: prodcomm.o 
	$(CC) -o prodcomm prodcomm.o -lpthread 