#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "util.h"

typedef struct file_info {
  int size;
  int fclust;
} finfo;

int fat_entry(unsigned char *data, int fclust, size_t secsize){
  int index1, index2, entry;

  index1 = secsize + (3*fclust/2);
  index2 = secsize + 1 + (3*fclust/2);
  if (fclust % 2 == 0){
    entry = (data[index1]) + ((data[index2] & 0x0F) << 8);
  } else {
    entry = ((data[index1] & 0xF0) >> 4) + (data[index2] << 4);
  }
  return entry;
}

int physical_entry(int logical_entry){
  return (logical_entry + 31) * 512;
}

void find_file(unsigned char *data, finfo *f, size_t secsize, char *file){
  int i;
  char *in_fname, *in_ext;
  char fname[9], ext[4];
  in_fname = strtok(file, ".");
  in_ext = strtok(NULL, ".");

  for(i = (19*secsize); i < (33*secsize); i+=32){
    read_str(fname, data, i, 8);
    nullify_spaces(fname, 8);
    read_str(ext, data, i+8, 3);

    if (!strcmp(in_fname, fname) && !strcmp(in_ext, ext)){
      f->size = read_num(data, i+28, 4);
      f->fclust = read_num(data, i+26, 2);
      return;
    }
  }
  printf("File not found.\n");
  exit(EXIT_FAILURE);
}

void copy_file(unsigned char *data, finfo f, size_t secsize, char *outfile){
  int entry, pentry, toread;
  int remaining = f.size;
  FILE *fout = fopen(outfile, "w");

  for (entry = f.fclust;
       entry < 0xFF8;
       entry = fat_entry(data, entry, secsize), remaining -= 512){
    pentry = physical_entry(entry);
    toread = remaining >= 512 ? 512 : remaining;
    fwrite(data+pentry, toread, 1, fout);
  }
  fclose(fout);
}

int main(int argc, char **argv){
  int fd;
  unsigned char *data;
  char *name = strdup(argv[2]);
  struct stat sf;
  finfo file;
  disk_info info;

  if (argc != 3){
    printf("Error: Usage: diskget <path-to-disk-image> <filename>\n");
    exit(EXIT_FAILURE);
  }
  if ((fd = open(argv[1], O_RDONLY))){
    fstat(fd, &sf);
    data = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    process_disk(data, &info);
    find_file(data, &file, info.sector_size, name);
    copy_file(data, file, info.sector_size, argv[2]);
  } else {
    printf("Failed to open file '%s'\n", argv[1]);
  }

  return 0;
}
