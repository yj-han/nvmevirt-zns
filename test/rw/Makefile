CC = gcc
CFLAGS = -Wall -Wextra -pedantic
KERNELDIR := /home/os/nvmevirt/

SRCS := $(wildcard *.c)
PROGS := $(patsubst %.c,%,$(SRCS))

all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)
