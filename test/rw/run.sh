ssd="/dev/nvme2n1"

sudo dmesg -C

make
sudo ./host_zns $ssd

echo ""

dmesg
