#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "util.h"

/* Simply print the disk info in the specified output format */
void print_info(disk_info info){
    printf("OS Name: %s\n", info.os_name);
    printf("Label of the disk: %s\n", info.volume_label);
    printf("Total size of the disk: %d\n", info.total_size);
    printf("Free space: %d\n", info.free_space);
    printf("==============\n");
    printf("The number of files in the root directory"
        " (not including subdirectories): %d\n", info.files_in_root);
    printf("==============\n");
    printf("Number of FAT copies: %d\n", info.num_fats);
    printf("Sectors per FAT: %d\n", info.sectors_per_fat);
}

int main(int argc, char **argv){
  int fd;
  unsigned char *data;
  struct stat sf;
  disk_info info;

  if (argc != 2){
    printf("Error: Usage: diskinfo <path-to-disk-images>\n");
    exit(EXIT_FAILURE);
  }
  if ((fd = open(argv[1], O_RDONLY))){
    fstat(fd, &sf);
    data = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    process_disk(data, &info);
    print_info(info);
  } else {
    printf("Failed to open file '%s'\n", argv[1]);
  }

  return 0;
}
