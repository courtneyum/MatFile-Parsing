CC=clang
CFLAGS=-c -g -Wall

all: mapping

mapping: userTest.o mapping.o fileHelper.o numberHelper.o queue.o
	$(CC) userTest.o mapping.o fileHelper.o numberHelper.o queue.o -o mapping -lm

userTest.o: userTest.c
	$(CC) $(CFLAGS) userTest.c

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