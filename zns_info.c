#include <fcntl.h>
#include <linux/blkzoned.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int fd = open(argv[1], O_RDWR | O_DIRECT, 0644);
  if (fd == -1) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  unsigned int nr_zones, zone_size;
  int ret;

  ret = ioctl(fd, BLKGETZONESZ, &zone_size);
  if (ret) {
    perror("ioctl zone_size failed");
    exit(EXIT_FAILURE);
  }

  ret = ioctl(fd, BLKGETNRZONES, &nr_zones);
  if (ret) {
    perror("ioctl nr_zones failed");
    exit(EXIT_FAILURE);
  }

  printf("Device has %u zones of %u 512-Bytes sectors\n", nr_zones, zone_size);
}
