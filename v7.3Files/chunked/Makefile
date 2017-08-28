CC=clang
CFLAGS=-c -g -Wall

all: chunked

chunked: chunkedData.o fileHelper.o numberHelper.o queue.o
	$(CC) chunkedData.o fileHelper.o numberHelper.o queue.o -o chunked -lm

chunkedData.o: chunkedData.c
	$(CC) $(CFLAGS) chunkedData.c

fileHelper.o: fileHelper.c
	$(CC) $(CFLAGS) fileHelper.c

numberHelper.o: numberHelper.c
	$(CC) $(CFLAGS) numberHelper.c

queue.o: queue.c
	$(CC) $(CFLAGS) queue.c

clean: 
	rm *o chunked