CC=gcc
CFLAGS=-c -g -Wall -std=c99

all: mapping

mapping: userTest.o mman.o mapping.o fileHelper.o numberHelper.o queue.o
	$(CC) userTest.o mman.o mapping.o fileHelper.o numberHelper.o queue.o -o mapping -lm

userTest.o: userTest.c
	$(CC) $(CFLAGS) userTest.c

mman.o: mman.c
	$(CC) $(CFLAGS) mman.c

mapping.o: mapping.c
	$(CC) $(CFLAGS) mapping.c

fileHelper.o: fileHelper.c
	$(CC) $(CFLAGS) fileHelper.c

numberHelper.o: numberHelper.c
	$(CC) $(CFLAGS) numberHelper.c

queue.o: queue.c
	$(CC) $(CFLAGS) queue.c

clean: 
	rm *o mapping