CC = gcc
CFLAGS = -c -ansi -Wall

all: diskinfo

diskinfo: diskinfo.o
	$(CC) diskinfo.o -o diskinfo

diskinfo.o: diskinfo.c
	$(CC) $(CFLAGS) diskinfo.c

clean:
	rm -rf *.o diskinfo
