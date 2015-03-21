#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* Copies the string of given length from data into buf,
 * starting at the offset */
void read_str(char *buf, char *data, size_t offset, size_t length){
  strncpy(buf, (data + offset), length);
  buf[length] = '\0';
}

int main(int argc, char **argv){
  int fd;
  char *data;
  char os_name[9]; /* Leave room for terminating '\0' */
  struct stat sf;

  if (argc != 2){
    printf("Error: Usage: diskinfo <path-to-disk-images>\n");
    exit(EXIT_FAILURE);
  }
  if ((fd = open(argv[1], O_RDONLY))){
    fstat(fd, &sf);
    data = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    read_str(os_name, data, 3, 8);
    printf("OS Name: %s\n", os_name);
  } else {
    printf("Failed to open file '%s'\n", argv[1]);
  }

  return 0;
}
