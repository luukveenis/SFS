#include <string.h>

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

/* Replace spaces with \0 */
void nullify_spaces(char *buf, size_t length){
  int i;
  for (i=0; i<length; i++){
    if (buf[i] == ' ') buf[i] = '\0';
  }
}
