PROOT = $(shell pwd)
export CC = gcc
export LD = ld
export CFLAGS =-Wall -Wextra -g
export INCLUDE = -I$(PROOT)/inc

export BINDIR = $(PROOT)/bin
export OBJDIR = $(PROOT)/obj
export LIBDIR = $(PROOT)/lib

BASE = src

all:
	+$(MAKE) -C $(BASE)/libbrandon
	+$(MAKE) -C $(BASE)/as
	# +$(MAKE) -C $(BASE)/ld
	# +$(MAKE) -C $(BASE)/cc

.PHONY: all clean test gtest mem debug
