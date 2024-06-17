#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define NUM_ITERATIONS 10000

int main(int argc, char **argv) {
  int fd;
  int *buffer;
  ssize_t bytes_read, bytes_written;
  clock_t start, end;
  double cpu_time_used;
  double write_throughput, read_throughput;

  // Allocate memory for the buffer (aligned to 512-byte boundary)
  posix_memalign((void **)&buffer, 512, BUFFER_SIZE);

  // Open the file for reading and writing with O_DIRECT flag
  fd = open(argv[1], O_RDWR | O_DIRECT, 0644);
  if (fd == -1) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  // Write numbers to the buffer
  for (int i = 0; i < BUFFER_SIZE / sizeof(int); i++) {
    buffer[i] = i;
  }

  // Perform write benchmark
  start = clock();
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    bytes_written = write(fd, buffer, BUFFER_SIZE);
    if (bytes_written == -1) {
      perror("Error writing file");
      exit(EXIT_FAILURE);
    }
  }
  end = clock();
  cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000;
  write_throughput = (BUFFER_SIZE * NUM_ITERATIONS) /
                     (cpu_time_used / 1000000) / (1024 * 1024);
  printf("Write benchmark: %.2f microseconds, Throughput: %.2f MB/s\n",
         cpu_time_used, write_throughput);

  // Reset the file offset to the beginning
  lseek(fd, 0, SEEK_SET);

  // Perform read benchmark
  start = clock();
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read == -1) {
      perror("Error reading file");
      exit(EXIT_FAILURE);
    }
  }
  end = clock();
  cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000;
  read_throughput = (BUFFER_SIZE * (double)NUM_ITERATIONS) /
                    (cpu_time_used / 1000000) / (1024 * 1024);
  printf("Read benchmark: %.2f microseconds, Throughput: %.2f MB/s\n",
         cpu_time_used, read_throughput);

  // Close the file
  close(fd);

  // Free the allocated memory
  free(buffer);

  return 0;
}
