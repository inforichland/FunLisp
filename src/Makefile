CC=gcc
CFLAGS=-O3 -g -Wall -Werror -std=c11
OBJECTS= main.o reader.o writer.o eval.o types.o primitives.o

all: compiler

clean:
	rm -f *.o *.~ *#

%.o : %.c
	$(CC) $(CFLAGS) -c $<

compiler: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o ../bin/compiler

