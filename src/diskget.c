#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "util.h"

int first_cluster(unsigned char *data, size_t secsize, char *file){
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
      return read_num(data, i+26, 2);
    }
  }
  printf("File not found.\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv){
  int fd, fclust;
  unsigned char *data;
  struct stat sf;
  disk_info info;

  if (argc != 3){
    printf("Error: Usage: diskget <path-to-disk-image> <filename>\n");
    exit(EXIT_FAILURE);
  }
  if ((fd = open(argv[1], O_RDONLY))){
    fstat(fd, &sf);
    data = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    process_disk(data, &info);
    fclust = first_cluster(data, info.sector_size, argv[2]);
    printf("First cluster: %d\n", fclust);
  } else {
    printf("Failed to open file '%s'\n", argv[1]);
  }

  return 0;
}
