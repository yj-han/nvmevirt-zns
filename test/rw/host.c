#include <fcntl.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#define BUFFERS (1)
#define BUFFER_SIZE (1 * 1024 * 1024) // 64 MB
#define DEVICE_FILE "/dev/nvme2"

int main() {
  int fd = open(DEVICE_FILE, O_RDWR);
  if (fd == -1) {
    perror("Error with open");
    exit(EXIT_FAILURE);
  }

  ssize_t page_size;
  char *buffers_unaligned[BUFFERS];
  struct iovec iov[BUFFERS];
  page_size = sysconf(_SC_PAGESIZE);

  srand(time(NULL)); // Seed the random number generator

  for (int i = 0; i < BUFFERS; i++) {
    buffers_unaligned[i] = (char *)malloc(BUFFER_SIZE + page_size);
    if (buffers_unaligned[i] == NULL) {
      perror("Unable to allocate memory");
      exit(EXIT_FAILURE);
    } else {
      iov[i].iov_base = (char *)(((long)buffers_unaligned[i] + page_size) &
                                 (~(page_size - 1)));
      int *data = (int *)iov[i].iov_base;
      for (int j = 0; j < BUFFER_SIZE / sizeof(int); j++) {
        data[j] = rand(); // Generate random numbers
      }
      iov[i].iov_len = BUFFER_SIZE;
    }
  }

  ssize_t nwritten = pwritev2(fd, iov, BUFFERS, 0, RWF_SYNC);
  fsync(fd);
  printf("Written bytes: %zu\n", nwritten);

  // Read the data back from the device
  ssize_t nread = preadv2(fd, iov, BUFFERS, 0, 0);
  printf("Read bytes: %zu\n", nread);

  // Print the read data
  for (int i = 0; i < BUFFERS; i++) {
    int *data = (int *)iov[i].iov_base;
    // Print the first 10 numbers from each buffer
    printf("Buffer %d:\n", i);
    for (int j = 0; j < 10; j++) {
      printf("%d ", data[j]);
    }
    printf("\n");
  }

  close(fd);
  for (int i = 0; i < BUFFERS; i++) {
    free(buffers_unaligned[i]);
    buffers_unaligned[i] = NULL;
  }
  return 0;
}
