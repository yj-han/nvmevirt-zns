#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int main(int argc, char **argv) {
  int fd;
  char *buffer;
  ssize_t bytes_read, bytes_written;

  // Allocate memory for the buffer (aligned to 512-byte boundary)
  posix_memalign((void **)&buffer, 512, BUFFER_SIZE);

  // Open the file for reading and writing with O_DIRECT flag
  fd = open(argv[1], O_RDWR | O_DIRECT, 0644);
  if (fd == -1) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  // Write some values to the buffer
  strcpy(buffer, "Hello, World!\n");

  // Write the buffer to the file using O_DIRECT
  bytes_written = write(fd, buffer, BUFFER_SIZE);
  if (bytes_written == -1) {
    perror("Error writing file");
    exit(EXIT_FAILURE);
  }

  // Reset the file offset to the beginning
  lseek(fd, 0, SEEK_SET);

  // Read from the file using O_DIRECT
  bytes_read = read(fd, buffer, BUFFER_SIZE);
  if (bytes_read == -1) {
    perror("Error reading file");
    exit(EXIT_FAILURE);
  }

  // Print the contents of the buffer
  printf("Read from file:\n%s", buffer);

  // Close the file
  close(fd);

  // Free the allocated memory
  free(buffer);

  return 0;
}
