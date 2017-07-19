export CC = gcc
export LD = ld
export CFLAGS =-Wall -Wextra -g
export INCLUDE = -Iinc

export BINDIR = $(shell pwd)/bin
export OBJDIR = $(shell pwd)/obj

BASE = src

all:
	+$(MAKE) -C $(BASE)/as
	# +$(MAKE) -C $(BASE)/ld
	# +$(MAKE) -C $(BASE)/cc

.PHONY: all clean test gtest mem debug
