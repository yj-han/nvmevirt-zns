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
CONFIG_NVMEVIRT_SSD := y
#CONFIG_NVMEVIRT_ZNS := y
#CONFIG_NVMEVIRT_KV := y
```
- commands to add nvmev module
```sh
make
sudo insmod ./nvmev.ko \
  memmap_start=64G \
  memmap_size=16385M   \
  cpus=7,8 
```
![](Screenshot%202024-06-03%20at%201.27.24%E2%80%AFPM.png)<!-- {"width":453} -->
- commands to remove nvmev module
```sh
sudo rmmod nvmev
```

## ZNS Mount
- mount ZoneFS: [ZoneFS](https://zonedstorage.io/docs/filesystems/zonefs)
```sh
sudo mkzonefs $NVMEVIRT_DISK_NAME
sudo mount -t zonefs $NVMEVIRT_DISK_NAME ~/mnt
```
- check statistics of each zone by  `stat /mnt/seq/1`

