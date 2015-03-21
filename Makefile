CC = gcc
CFLAGS = -c -ansi -Wall
VPATH = src

all: diskinfo

diskinfo: diskinfo.o
	$(CC) $< -o $@

diskinfo.o: diskinfo.c
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o diskinfo
