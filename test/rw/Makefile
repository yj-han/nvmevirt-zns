CC = gcc
CFLAGS = -Wall -Wextra -pedantic
KERNELDIR := /home/os/nvmevirt/

all: host host_zns

host: host.c
	$(CC) $(CFLAGS) -o host host.c

host_zns: host_zns.c
	$(CC) $(CFLAGS) -o host_zns host_zns.c

clean:
	rm -f host host_zns
