#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "util.h"

int next_free_sector(unsigned char *data, disk_info info){
  int i, entry, index1, index2;
  for (i = 2; i < info.total_sectors-33+2; i++){
    index1 = info.sector_size + (3*i/2);
    index2 = info.sector_size + 1 + (3*i/2);
    if (i % 2 == 0){
      entry = (data[index1]) + ((data[index2] & 0x0F) << 8);
    } else {
      entry = ((data[index1] & 0xF0) >> 4) + (data[index2] << 4);
    }
    if (entry == 0x00) return entry;
  }
  printf("Error: No empty FAT entries\n");
  exit(EXIT_FAILURE);
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
      return;
    }
  }
  printf("Error: No empty root dir entries\n");
  exit(EXIT_FAILURE);
}

void write_file(unsigned char *data, disk_info info, char *fname){
  FILE *fin;
  struct stat sf;
  int fcluster = next_free_sector(data, info);

  if ((fin = fopen(fname, "r"))){
      stat(fname, &sf);

      if (info.free_space < sf.st_size){
        printf("Not enough free space in the disk image.\n");
      } else {
        create_root_entry(data, info, fname, (int)sf.st_size, fcluster);
      }
  } else {
    printf("Failed to open file: %s\n", fname);
    exit(EXIT_FAILURE);
  }

}

void write_to_disk(unsigned char *data, int fd, size_t size){
  FILE *disk;
  int i;
  if ((disk = fdopen(fd, "w"))){
    for (i = 0; i < size; i++){
      fputc(data[i], disk);
    }
    fclose(disk);
  } else {
    printf("Error writing to disk image.\n");
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
    /* write_to_disk(data, fd, sf.st_size); */
  } else {
    printf("Failed to open file '%s'\n", argv[1]);
  }

  return 0;
}
