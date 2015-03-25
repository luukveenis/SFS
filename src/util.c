#include <string.h>

/* Reads a number of 'size' bytes from the disk image copied into
 * 'data', starting at the given offset.
 * This will convert it from little endian to big endian. */
int read_num(char *data, size_t offset, size_t size){
  int retval, i;
  for(i=0, retval=0; i<size; i++){
    retval += data[offset+i] << (8*i);
  }
  return retval;
}

/* Copies the string of given length from data into buf,
 * starting at the offset */
void read_str(char *buf, char *data, size_t offset, size_t length){
  strncpy(buf, (data + offset), length);
  buf[length] = '\0';
}
