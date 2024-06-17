## Installation
### GRUB File Setting
```sh
GRUB_CMDLINE_LINUX="memmap=32G\\\$64G isolcpus=7,8 amd_iommu=off"
```

### NVMeVirt Compilation
- Update `Kbuild`
```makefile
# Select one of the targets to build
# CONFIG_NVMEVIRT_NVM := y
#CONFIG_NVMEVIRT_SSD := y
CONFIG_NVMEVIRT_ZNS := y
#CONFIG_NVMEVIRT_KV := y

```sh
ccflags-$(CONFIG_NVMEVIRT_ZNS) += -DBASE_SSD=WD_ZN540
#ccflags-$(CONFIG_NVMEVIRT_ZNS) += -DBASE_SSD=ZNS_PROTOTYPE
ccflags-$(CONFIG_NVMEVIRT_ZNS) += -Wno-implicit-fallthrough
nvmev-$(CONFIG_NVMEVIRT_ZNS) += ssd.o zns_ftl.o zns_read_write.o zns_mgmt_send.o zns_mgmt_recv.o channel_model.o
```
```
- commands to add nvmev module
```sh
make
sudo insmod ./nvmev.ko \
  memmap_start=64G \
  memmap_size=16385M   \
  cpus=7,8 
```
- commands to remove nvmev module
```sh
sudo rmmod nvmev
```
### ZNS Mount
- mount ZoneFS: [ZoneFS](https://zonedstorage.io/docs/filesystems/zonefs)
```sh
sudo mkzonefs $NVMEVIRT_DISK_NAME
sudo mount -t zonefs $NVMEVIRT_DISK_NAME ~/mnt
```

## ZNS Statistcs
- assume that ZNS device is `/dev/nvme2n1`, execute
```sh
sudo ./zns_report /dev/nvme2n1
```
- `sudo blkzone report /dev/nvme2n1` same information

## Simple IO Direct RW
using `examples/rw.c`,
- conventional
  - Write benchmark: 28174.00 microseconds, Throughput: 1386.47 MB/s
  - Read benchmark: 35957.00 microseconds, Throughput: 1086.37 MB/s
- zns
  - Write benchmark: 38651.00 microseconds, Throughput: 1010.65 MB/s
  - Read benchmark: 45682.00 microseconds, Throughput: 855.10 MB/s

## Host Managed Garbage Collection (GC) for OPT Model Checkpoints
- `gc.c`: the host periodically runs a garbage collector and remove
  old checkpoints in a zone when more than half of the zone is filled.
  - the code assumes that the size of a checkpoint for the model is known.
