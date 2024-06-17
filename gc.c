#include <fcntl.h>
#include <linux/blkzoned.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE (128 * 1024 * 1024) // 128 MB

int main(int argc, char **argv) {
  // Allocate memory for the buffer (aligned to 512-byte boundary)
  int *buffer;
  posix_memalign((void **)&buffer, 512, BUFFER_SIZE);
  for (int i = 0; i < BUFFER_SIZE / sizeof(int); i++) {
    buffer[i] = i;
  }

  // Open the file for reading and writing with O_DIRECT flag
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

  struct blk_zone_report *report = malloc(sizeof(struct blk_zone_report) +
                                          nr_zones * sizeof(struct blk_zone));
  if (!report) {
    perror("Failed to allocate memory");
    close(fd);
    return 1;
  }

  report->nr_zones = nr_zones;
  report->sector = 0;

  while (report->sector < nr_zones) {
    if (ioctl(fd, BLKREPORTZONE, report) < 0) {
      perror("Failed to get zone report");
      break;
    }

    for (ssize_t i = 0; i < report->nr_zones; i++) {
      printf("Zone %ld: before gc\n", i);
      printf("  Start: %llu\n", report->zones[i].start);
      printf("  Length: %llu\n", report->zones[i].len);
      printf("  Write Pointer: %llu\n", report->zones[i].wp);
      printf("\n");

      struct blk_zone_range range = {.sector = report->zones[i].start,
                                     .nr_sectors = report->zones[i].len};

      if (ioctl(fd, BLKRESETZONE, &range) < 0) {
        perror("Failed to reset zone");
        continue;
      }
    }
    report->sector += report->nr_zones;
  }

  struct blk_zone_report *report_gc = malloc(
      sizeof(struct blk_zone_report) + nr_zones * sizeof(struct blk_zone));
  if (!report_gc) {
    perror("Failed to allocate memory");
    close(fd);
    return 1;
  }
  report_gc->nr_zones = nr_zones;
  report_gc->sector = 0;

  while (report_gc->sector < nr_zones) {
    if (ioctl(fd, BLKREPORTZONE, report_gc) < 0) {
      perror("Failed to get zone report");
      break;
    }

    for (ssize_t i = 0; i < report_gc->nr_zones; i++) {
      printf("Zone %ld: after gc\n", i);
      printf("  Start: %llu\n", report_gc->zones[i].start);
      printf("  Write Pointer Prev: %llu\n", report->zones[i].wp);
      printf("  Write Pointer Now: %llu\n", report_gc->zones[i].wp);
      printf("\n");
    }
    report_gc->sector += report_gc->nr_zones;
  }

  // Close the file
  close(fd);

  // Free the allocated memory
  free(buffer);
  free(report);
  free(report_gc);

  return 0;
}
