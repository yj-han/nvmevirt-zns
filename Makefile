CC = gcc
CFLAGS = -D_GNU_SOURCE -Wall -pedantic
KERNELDIR := /home/os/nvmevirt/
SRCS := $(wildcard *.c)
PROGS := $(patsubst %.c,%,$(SRCS))
OBJDIR := bin
BINDIR := bin

all: $(PROGS)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROGS): %: $(OBJDIR)/%.o
	$(CC) $(CFLAGS) -o $(BINDIR)/$@ $<

clean:
	rm -rf $(OBJDIR) $(PROGS)

