#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "util.h"

struct date {
  int day;
  int month;
  int year;
};

struct time {
  int hours;
  int mins;
  int secs;
};

void parse_date(struct date *fdate, unsigned char *data, int offset){
  int date = (data[offset+17] << 8) + data[offset+16];
  fdate->year = 1980 + ((date & 0xFE00) >> 9);
  fdate->month = (date & 0x01E0) >> 5;
  fdate->day = (date & 0x001F);
}

void parse_time(struct time *ftime, unsigned char *data, int offset){
  int time = (data[offset+15] << 8) + data[offset+14];
  ftime->hours = (time & 0xF800) >> 11;
  ftime->mins = (time & 0x07E0) >> 5;
  ftime->secs = (time & 0x001F);
}

void list_root(unsigned char *data, int secsize){
  char fname[9];
  struct date fdate;
  struct time ftime;
  int i, is_file, fsize;

  for (i=(19*secsize); i<(33*secsize); i+=32){
    if ((int)data[i] == 0xE5
        || (int)data[i+11] == 0x0F
        || data[i+11] & 0x08) continue;
    if (!data[i]) return; /* No more files */

    is_file = (data[i+11] & 0x10) ? 0 : 1;
    fsize = read_num(data, (i+28), 4);
    read_str(fname, data, i, 8);
    parse_date(&fdate, data, i);
    parse_time(&ftime, data, i);
    printf("Filename: %s\n", fname);
    printf("Year: %d, Month: %d, Day: %d\n", fdate.year, fdate.month, fdate.day);
    printf("Time: %02d:%02d:%02d\n", ftime.hours, ftime.mins, ftime.secs);
  }
}

int main(int argc, char **argv){
  int fd, secsize;
  unsigned char *data;
  struct stat sf;

  if (argc != 2){
    printf("Error: Usage: diskinfo <path-to-disk-images>\n");
    exit(EXIT_FAILURE);
  }
  if ((fd = open(argv[1], O_RDONLY))){
    fstat(fd, &sf);
    data = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    /* Implement root directory listing here */
    secsize = read_num(data, 11, 2);
    list_root(data, secsize);
  } else {
    printf("Failed to open file '%s'\n", argv[1]);
  }

  return 0;
}
