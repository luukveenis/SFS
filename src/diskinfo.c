#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

int read_num(char *data, size_t offset, size_t size){
  int retval, i;

  for(i=0, retval=0; i<size; i++){
    retval += data[offset+i] << (8*i);
  }
  return retval;
}

int get_total_size(char *data, int secsize){
  int totalsecs;

  totalsecs = read_num(data, 19, 2);
  if (totalsecs == 0){
    totalsecs = read_num(data, 32, 4);
  }

  return (totalsecs * secsize);
}

/* Copies the string of given length from data into buf,
 * starting at the offset */
void read_str(char *buf, char *data, size_t offset, size_t length){
  strncpy(buf, (data + offset), length);
  buf[length] = '\0';
}

int get_label(char *data, char *buf, int secsize){
  int i;
  for(i = (19*secsize); i < (33*secsize); i+=32){
    if ((int)data[i] == 0xE5 || (int)data[i+11] == 0x0F) continue;
    if (!data[i]){
      printf("Error: No volume label found\n");
      exit(EXIT_FAILURE);
    }
    if (data[i+11] & 0x08){
      read_str(buf, data, i, 8);
      return 0;
    }
  }
  return 1;
}

/* Counts the number of files in the root directory
 * 0xE5 in entry i means the entry is empty, but there are still more files
 * 0x0F in entry i+11 corresponds to a long file name and we ignore it
 * 0x08 in entry i+11 corresponds to a volume label
 * 0x10 in entry i+11 corresponds to a subdirectory
 * !data[i] tests if the rest of the directory is emtpy */
int files_in_root(char *data, int secsize){
  int i, count;
  for(count = 0, i = (19*secsize); i < (33*secsize); i+=32){
    if ((int)data[i] == 0xE5 || (int)data[i+11] == 0x0F
        || data[i+11] & 0x08 || data[i+11] & 0x10) continue;
    if (!data[i]) return count;
    printf("Data[i]: %x\n", data[i]);
    printf("Data[i+11]: %x\n", data[i+11]);
    ++count;
  }
  return count;
}

int main(int argc, char **argv){
  int fd, secsize, totsize, files;
  char *data;
  char os_name[9]; /* Leave room for terminating '\0' */
  char vol_label[9];
  struct stat sf;

  if (argc != 2){
    printf("Error: Usage: diskinfo <path-to-disk-images>\n");
    exit(EXIT_FAILURE);
  }
  if ((fd = open(argv[1], O_RDONLY))){
    fstat(fd, &sf);
    data = mmap(NULL,sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    secsize = read_num(data, 11, 2);
    totsize = get_total_size(data, secsize);
    files = files_in_root(data, secsize);
    read_str(os_name, data, 3, 8);
    get_label(data, vol_label, secsize);

    printf("OS Name: %s\n", os_name);
    printf("Label of the disk: %s\n", vol_label);
    printf("Total size of the disk: %d\n", totsize);
    printf("==============\n");
    printf("The number of files in the root directory"
        " (not including subdirectories): %d\n", files);
    printf("==============\n");
  } else {
    printf("Failed to open file '%s'\n", argv[1]);
  }

  return 0;
}
