#include <fcntl.h>
#include <linux/blkzoned.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define

struct checkpoint {
  __u64 zone_start;
  __u64 offset;
  __u64 size;
};

int main(int argc, char **argv) {
  int fd = open(argv[1], O_RDWR | O_DIRECT, 0644);
  if (fd == -1) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  __u32 nr_zones;
  if (ioctl(fd, BLKGETNRZONES, &nr_zones) < 0) {
    perror("Failed to get number of zones");
    close(fd);
    return 1;
  }

  struct blk_zone_report *report = malloc(sizeof(struct blk_zone_report) +
                                          nr_zones * sizeof(struct blk_zone));
  if (!report) {
    perror("Failed to allocate memory");
    close(fd);
    return 1;
  }

  report->nr_zones = nr_zones;
  report->sector = 0;

  struct checkpoint ckpt = {0};

  while (report->sector < nr_zones) {
    if (ioctl(fd, BLKREPORTZONE, report) < 0) {
      perror("Failed to get zone report");
      break;
    }

    for (int i = 0; i < report->nr_zones; i++) {
      if (report->zones[i].wp - report->zones[i].start >=
          report->zones[i].len / 2) {
        // Update checkpoint information
        ckpt.zone_start = report->zones[i].start;
        ckpt.offset = report->zones[i].wp - report->zones[i].start;
        ckpt.size = report->zones[i].wp - report->zones[i].start;

        struct blk_zone_range range = {.sector = report->zones[i].start,
                                       .nr_sectors = report->zones[i].len};

        if (ioctl(fd, BLKRESETZONE, &range) < 0) {
          perror("Failed to reset zone");
          continue;
        }

        // Move data from the checkpoint to the first offset of the zone
        off_t offset = lseek(fd, range.sector * 512, SEEK_SET);
        if (offset == -1) {
          perror("Failed to seek to zone start");
          continue;
        }

        ssize_t bytes_written =
            write(fd, (void *)((char *)report + ckpt.offset), ckpt.size);
        if (bytes_written != ckpt.size) {
          perror("Failed to write data to zone start");
          continue;
        }
      }
    }

    report->sector += report->nr_zones;
  }

  free(report);
  close(fd);

  return 0;
}
