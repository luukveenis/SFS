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

int total_sectors(char *data){
  int totalsecs = read_num(data, 19, 2);
  if (totalsecs == 0){
    totalsecs = read_num(data, 32, 4);
  }

  return totalsecs;
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
    ++count;
  }
  return count;
}

int free_space(char *data, int totsecs, int secsize){
  int space, entry, i, index1, index2;


  for (space = 0, i = 2; i < totsecs-33+2; i++){
    index1 = secsize + (3*i/2);
    index2 = secsize + 1 + (3*i/2);
    if (i % 2 == 0){
      entry = (data[index1]) + ((data[index2] & 0x0F) << 8);
    } else {
      entry = ((data[index1] & 0xF0) >> 4) + (data[index2] << 8);
    }
    if (entry == 0x00) space += secsize;
  }
  return space;
}

int main(int argc, char **argv){
  int fd, secsize, totsecs, totsize, space, files;
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
    totsecs = total_sectors(data);
    totsize = totsecs * secsize;
    files = files_in_root(data, secsize);
    space = free_space(data, totsecs, secsize);
    read_str(os_name, data, 3, 8);
    get_label(data, vol_label, secsize);

    printf("OS Name: %s\n", os_name);
    printf("Label of the disk: %s\n", vol_label);
    printf("Total size of the disk: %d\n", totsize);
    printf("Free space: %d\n", space);
    printf("==============\n");
    printf("The number of files in the root directory"
        " (not including subdirectories): %d\n", files);
    printf("==============\n");
  } else {
    printf("Failed to open file '%s'\n", argv[1]);
  }

  return 0;
}
