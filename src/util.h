#ifndef UTIL_H_
#define UTIL_H_

/* Data structure we can read all the disk information into
 * so that it can be used in other places in the program */
typedef struct disk_info {
  char os_name[9];
  char volume_label[9];
  int sector_size;
  int total_sectors;
  int total_size;
  int free_space;
  int files_in_root;
  int num_fats;
  int sectors_per_fat;
} disk_info;

int read_num(unsigned char*, size_t, size_t);
void write_num(unsigned char*, int, size_t, size_t);
void read_str(char*, unsigned char*, size_t, size_t);
int physical_entry(int);
void process_disk(unsigned char*, disk_info*);
void nullify_spaces(char*, size_t);

#endif
