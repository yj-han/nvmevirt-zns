# ./simple_bench /mnt/ssd0/direct_rw
sudo blkzone reset /dev/nvme2n1
./simple_bench /dev/nvme2n1
