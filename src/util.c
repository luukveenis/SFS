#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/* Reads a number of 'size' bytes from the disk image copied into
 * 'data', starting at the given offset.
 * This will convert it from little endian to big endian. */
int read_num(unsigned char *data, size_t offset, size_t size){
  int retval, i;
  for(i=0, retval=0; i<size; i++){
    retval += data[offset+i] << (8*i);
  }
  return retval;
}

/* Copies the string of given length from data into buf,
 * starting at the offset */
void read_str(char *buf, unsigned char *data, size_t offset, size_t length){
  strncpy(buf, ((char*)data + offset), length);
  buf[length] = '\0';
}

/* Returns the total number of sectors in the disk */
int total_sectors(unsigned char *data){
  int totalsecs = read_num(data, 19, 2);
  if (totalsecs == 0){
    totalsecs = read_num(data, 32, 4);
  }
  return totalsecs;
}

/* Searches the root directory for the volume label.
 * The label should always be present, so we exit with failure if it is not. */
int get_label(unsigned char *data, char *buf, int secsize){
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
int files_in_root(unsigned char *data, int secsize){
  int i, count;
  for(count = 0, i = (19*secsize); i < (33*secsize); i+=32){
    if ((int)data[i] == 0xE5 || (int)data[i+11] == 0x0F
        || data[i+11] & 0x08 || data[i+11] & 0x10) continue;
    if (!data[i]) return count;
    ++count;
  }
  return count;
}

/* Calculate the free space in the disk by looking for empty entries in the
 * FAT table, each of which corresponds to an empty sector. */
int free_space(unsigned char *data, int totsecs, int secsize){
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

/* Populates the disk_info struct pointed to by info with the correct
 * values for the disk image copied into data. */
void process_disk(unsigned char *data, disk_info *info){
    int secsize = read_num(data, 11, 2);
    read_str(info->os_name, data, 3, 8);
    get_label(data, info->volume_label, secsize);
    info->sector_size = secsize;
    info->total_sectors = total_sectors(data);
    info->total_size = info->total_sectors * secsize;
    info->files_in_root = files_in_root(data, secsize);
    info->free_space = free_space(data, info->total_sectors, secsize);
    info->num_fats = read_num(data, 16, 1);
    info->sectors_per_fat = read_num(data, 22, 2);
}


/* Replace spaces with \0 */
void nullify_spaces(char *buf, size_t length){
  int i;
  for (i=0; i<length; i++){
    if (buf[i] == ' ') buf[i] = '\0';
  }
}
