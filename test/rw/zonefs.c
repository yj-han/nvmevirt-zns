#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char **argv) {
  FILE *file;
  char read_buffer[BUFFER_SIZE];

  file = fopen(argv[1], "wb");
  if (file == NULL) {
    printf("Error: Unable to open the file, %s\n", strerror(errno));
    return -1;
  }

  int ret = fputs("Hello World\n", file);
  if (ret == EOF) {
    printf("Error: Failed to write, %s\n", strerror(errno));
    fclose(file);
    return 1;
  }

  fclose(file);

  file = fopen(argv[1], "rb");
  if (file == NULL) {
    printf("Error: Unable to open the file, %s\n", strerror(errno));
    return -1;
  }

  while (fgets(read_buffer, sizeof read_buffer, file) != NULL) {
    printf("%s", read_buffer);
  }

  // ssize_t bytes_read = fread(read_buffer, sizeof(char), BUFFER_SIZE, file);
  // if (bytes_read == -1) {
  //   printf("Error: Failed to read, %s\n", strerror(errno));
  //   fclose(file);
  //   return 1;
  // }
  //
  // printf("Data read from the file : %.*s", (int)bytes_read, read_buffer);
  fclose(file);
  return 0;
}
