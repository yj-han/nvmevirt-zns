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

  __u64 *bytes_written = malloc(sizeof(__u64) * nr_zones);
  if (!report || !bytes_written) {
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

    for (int i = 0; i < report->nr_zones; i++) {
      printf("Zone %d:\n", i);
      printf("  Start: %llu\n", report->zones[i].start);
      printf("  Length: %llu\n", report->zones[i].len);
      printf("  Write Pointer: %llu\n", report->zones[i].wp);
      printf("\n");

      // Move to the wptr of each zone
      off_t offset = lseek(fd, report->zones[i].wp * 512, SEEK_SET);
      if (offset == -1) {
        perror("Failed to seek to zone wp");
        continue;
      }

      bytes_written[i] = write(fd, buffer, BUFFER_SIZE);
    }
    report->sector += report->nr_zones;
  }

  struct blk_zone_report *report_w = malloc(sizeof(struct blk_zone_report) +
                                            nr_zones * sizeof(struct blk_zone));
  if (!report_w) {
    perror("Failed to allocate memory");
    close(fd);
    return 1;
  }

  report_w->nr_zones = nr_zones;
  report_w->sector = 0;

  while (report_w->sector < nr_zones) {
    if (ioctl(fd, BLKREPORTZONE, report_w) < 0) {
      perror("Failed to get zone report");
      break;
    }

    for (int i = 0; i < report_w->nr_zones; i++) {
      printf("Zone %d: after write %llu\n", i, bytes_written[i]);
      printf("  Write Pointer: %llu\n", report_w->zones[i].wp);
      printf("  Write Pointer Diff: %llu\n",
             report_w->zones[i].wp - report->zones[i].wp);
      printf("\n");
    }
    report_w->sector += report_w->nr_zones;
  }

  // Close the file
  close(fd);

  // Free the allocated memory
  free(buffer);
  free(report);
  free(report_w);
  return 0;
}
