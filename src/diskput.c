#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "util.h"

int main(int argc, char **argv){
  int fd;
  unsigned char *data;
  struct stat sf;
  disk_info info;

  if (argc != 3){
    printf("Error: Usage: diskput <path-to-disk-image> <filename>\n");
    exit(EXIT_FAILURE);
  }
  if ((fd = open(argv[1], O_RDONLY))){
    fstat(fd, &sf);
    data = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    process_disk(data, &info);
  } else {
    printf("Failed to open file '%s'\n", argv[1]);
  }

  return 0;
}
