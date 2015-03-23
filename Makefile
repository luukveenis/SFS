CC = gcc
CFLAGS = -c -ansi -Wall
VPATH = src

all: diskinfo disklist

diskinfo: diskinfo.o
	$(CC) $< -o $@

disklist: disklist.o
	$(CC) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o diskinfo disklist
