CC=gcc
LIBS=-lasound

all: asub

clean:
	rm -f asub.o asub

asub: asub.o
	$(CC) asub.o -o asub $(LIBS)

asub.o: asub.c
	$(CC) -pedantic -std=c99 -c asub.c -o asub.o

.PHONY: clean
