CC=gcc
LIBS=-lasound

all: asub

clean:
	rm -f asub.o asub

asub: asub.o
	$(CC) asub.o -o asub $(LIBS)

asub.o: asub.c
	$(CC) -c asub.c -o asub.o

.PHONY: clean
