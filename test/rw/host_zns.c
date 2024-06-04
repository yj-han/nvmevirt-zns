#define _GNU_SOURCE

#include <dirent.h>
#include <fcntl.h>
#include <ftw.h>
#include <linux/nvme_ioctl.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PAGE_SIZE 4096
#define NUM 1

int main(int argc, char **argv) {
  int fd;
  int i;
  int num;
  int ret;
  int buffer_size = PAGE_SIZE * NUM;
  __u32 write_offset = 4096;
  __u32 read_offset = 8192;

  fd = open(argv[1], O_RDWR | O_DIRECT | O_SYNC);
  if (fd == -1) {
    printf("Error\n");
    return -1;
  }

  char *read_data = (char *)memalign(PAGE_SIZE, buffer_size);
  char *write_data = (char *)memalign(PAGE_SIZE, buffer_size);
  char *buffer = (char *)memalign(PAGE_SIZE, buffer_size);

  memset(read_data, 0, buffer_size);
  memset(write_data, 0, buffer_size);
  memset(buffer, 0, buffer_size);

  for (i = 0; i < buffer_size; i++) {
    read_data[i] = 'R';
  }
  for (i = 0; i < buffer_size; i++) {
    write_data[i] = 'W';
  }

  /* Initialize pages to read */
  ret = pwrite(fd, read_data, buffer_size, read_offset);
  printf("Initialize: data filled with %d 'R's to offset(%d)\n", ret,
         read_offset);

  /* Initalize READ command */
  struct nvme_passthru_cmd nvme_cmd_r;
  struct nvme_passthru_cmd *m_nvme_cmd_r = &nvme_cmd_r;
  memset(m_nvme_cmd_r, 0, sizeof(struct nvme_passthru_cmd));

  m_nvme_cmd_r->nsid = 1;
  m_nvme_cmd_r->opcode = 0x02;
  m_nvme_cmd_r->addr = (__u64)buffer;
  m_nvme_cmd_r->data_len = buffer_size;
  m_nvme_cmd_r->cdw10 = read_offset / 512;
  m_nvme_cmd_r->cdw12 = buffer_size / 512 - 1;
  m_nvme_cmd_r->cdw13 = buffer_size / 512 - 1;
  m_nvme_cmd_r->cdw14 = read_offset / 512 * (NUM + 1);

  ret = ioctl(fd, NVME_IOCTL_IO_CMD, m_nvme_cmd_r);
  if (ret < 0) {
    printf("ioctl failed!! %d\n", ret);
  }

  /* Verify RW command READ */
  num = 0;
  for (int i = 0; i < buffer_size; i++) {
    if (buffer[i] == 'R')
      num++;
  }
  printf("RW command retrieved data filled with %d 'R's from offset(%d)\n", num,
         read_offset);

  /* Initalize WRITE command */
  struct nvme_passthru_cmd nvme_cmd_w;
  struct nvme_passthru_cmd *m_nvme_cmd_w = &nvme_cmd_w;
  memset(m_nvme_cmd_w, 0, sizeof(struct nvme_passthru_cmd));

  m_nvme_cmd_w->nsid = 1;
  m_nvme_cmd_w->opcode = 0x7d;
  m_nvme_cmd_w->addr = (__u64)write_data;
  m_nvme_cmd_w->data_len = buffer_size;
  m_nvme_cmd_w->cdw10 = write_offset / 512;
  m_nvme_cmd_w->cdw12 = buffer_size / 512 - 1;
  m_nvme_cmd_w->cdw13 = buffer_size / 512 - 1;
  m_nvme_cmd_w->cdw14 = read_offset / 512;

  ret = ioctl(fd, NVME_IOCTL_IO_CMD, m_nvme_cmd_w);
  if (ret < 0) {
    printf("ioctl failed!! %d\n", ret);
  }

  /* Verify RW command WRITE*/
  ret = pread(fd, buffer, buffer_size, write_offset);
  num = 0;
  for (int i = 0; i < buffer_size; i++) {
    if (buffer[i] == 'W')
      num++;
  }
  printf("RW command wrote data filled with %d 'W's from offset(%d)\n", num,
         write_offset);

  free(read_data);
  free(write_data);
  free(buffer);
  close(fd);

  return 0;
}
