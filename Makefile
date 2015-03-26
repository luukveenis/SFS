CC = gcc
CFLAGS = -c -ansi -Wall
VPATH = src

all: diskinfo disklist diskget

diskinfo: diskinfo.o util.o
	$(CC) $^ -o $@

disklist: disklist.o util.o
	$(CC) $^ -o $@

diskget: diskget.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o diskinfo disklist diskget
