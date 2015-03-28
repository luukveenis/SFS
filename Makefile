CC = gcc
CFLAGS = -c -Wall
VPATH = src

all: diskinfo disklist diskget diskput

diskinfo: diskinfo.o util.o
	$(CC) $^ -o $@

disklist: disklist.o util.o
	$(CC) $^ -o $@

diskget: diskget.o util.o
	$(CC) $^ -o $@

diskput: diskput.o util.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o diskinfo disklist diskget diskput
