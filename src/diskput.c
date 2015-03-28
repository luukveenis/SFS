#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "util.h"

int next_free_sector(unsigned char *data, disk_info info, int current){
  int i, entry, index1, index2;
  for (i = 2; i < info.total_sectors-33+2; i++){
    if (i == current) continue;

    index1 = info.sector_size + (3*i/2);
    index2 = info.sector_size + 1 + (3*i/2);
    if (i % 2 == 0){
      entry = (data[index1]) + ((data[index2] & 0x0F) << 8);
    } else {
      entry = ((data[index1] & 0xF0) >> 4) + (data[index2] << 4);
    }
    if (entry == 0x00) return i;
  }
  printf("Error: No empty FAT entries\n");
  exit(EXIT_FAILURE);
}

int creation_time(){
  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);
  int h = timeinfo->tm_hour;
  int m = timeinfo->tm_min;
  int s = timeinfo->tm_sec;
  int time = ((h & 0x1f) << 11) | ((m & 0x3f) << 5) | (s & 0x1f);

  return time;
}

int creation_date(){
  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);
  int y = timeinfo->tm_year - 80; /* we want since 1980 instead of 1900 */
  int m = timeinfo->tm_mon + 1;
  int d = timeinfo->tm_mday;
  int date = ((y & 0x7f) << 9) | ((m & 0xf) << 5) | (d & 0x1f);

  return date;

}

void create_root_entry(unsigned char* data, disk_info info, char *fname, int size, int fclust){
  int i, j, basel, extl;
  char *name = strdup(fname); /* So we don't lose the original name with strtok */
  char *base = strtok(name, ".");
  char *ext = strtok(NULL, ".");
  basel = (int)strlen(base);
  extl = (int)strlen(ext);

  for (i = 19*(info.sector_size); i < (33*info.sector_size); i+=32){
    if ((int)data[i] == 0xE5 || (int)data[i] == 0x00){
      for(j = 0; j < 8; j++) data[i+j] = (j < basel) ? base[j] : ' ';
      for(j = 0; j < 3; j++) data[i+8+j] = (j < extl) ? ext[j] : ' ';
      data[i+11] = 0x00;
      write_num(data, size, i+28, 4);
      write_num(data, fclust, i+26, 2);
      write_num(data, creation_time(), i+14, 2);
      write_num(data, creation_date(), i+16, 2);
      return;
    }
  }
  printf("Error: No empty root dir entries\n");
  exit(EXIT_FAILURE);
}

void create_fat_entry(unsigned char *data, int index, int value, int secsize){
  int index1, index2;

  index1 = secsize + (3*index/2);
  index2 = secsize + 1 + (3*index/2);
  if (index % 2 == 0){
    data[index1] = value & 0xFF;
    data[index2] = (data[index2] & 0xF0) | ((value & 0xF00) >> 8);
  } else {
    data[index1] = (data[index1] & 0xF) | ((value & 0xF) << 4);
    data[index2] = (value & 0xFF0) >> 4;
  }
}

void write_file(unsigned char *data, disk_info info, char *fname){
  int fin, remain, i, next, pentry, towrite, last, temp;
  unsigned char *input;
  struct stat sf;
  int fcluster = next_free_sector(data, info, 0);

  if ((fin = open(fname, O_RDONLY))){
      stat(fname, &sf);
      input = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fin, 0);

      if (info.free_space < sf.st_size){
        printf("Not enough free space in the disk image.\n");
      } else {
        create_root_entry(data, info, fname, (int)sf.st_size, fcluster);
        remain = (int)sf.st_size;
        for (next = fcluster, i = 0;
             remain > 0;
             remain -= 512, i++){
          pentry = physical_entry(next);
          last = remain > 512 ? 0 : 1;
          towrite = last ? 512 : remain;
          memcpy(data+pentry, input+(i*512), towrite);
          if (last){
            create_fat_entry(data, next, 0xFFF, info.sector_size);
          } else {
            temp = next;
            next = next_free_sector(data, info, temp);
            create_fat_entry(data, temp, next, info.sector_size);
          }
        }
      }
  } else {
    printf("Failed to open file: %s\n", fname);
    exit(EXIT_FAILURE);
  }

}

int main(int argc, char **argv){
  int fd;
  unsigned char *data;
  struct stat sf;
  disk_info info;

  if (argc != 3){
    printf("Error: Usage: diskput <path-to-disk-image> <filename>\n");
    exit(EXIT_FAILURE);
  }
  if ((fd = open(argv[1], O_RDWR))){
    fstat(fd, &sf);
    data = mmap(NULL,sf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    process_disk(data, &info);

    write_file(data, info, argv[2]);
  } else {
    printf("Failed to open file '%s'\n", argv[1]);
  }

  return 0;
}
