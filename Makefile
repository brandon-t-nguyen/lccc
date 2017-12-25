PROOT = $(shell pwd)
export BINDIR = $(PROOT)/bin
export OBJDIR = $(PROOT)/obj
export LIBDIR = $(PROOT)/lib

export CC = gcc
export LD = ld
export CFLAGS =-Wall -Wextra -g
export INCLUDE = -I$(PROOT)/inc
export LIBS   = -L$(LIBDIR) -lbtn

export LIB_DEPEND = $(LIBDIR)/libbtn.a

export CFLAGS +=-DDEBUG

BASE = src

$(LIBDIR)/libbtn.a:
	mkdir -p $(LIBDIR)
	+$(MAKE) -C external/libbtn
	cp external/libbtn/lib/libbtn.a $(LIBDIR)/

as: $(LIB_DEPEND)
	+$(MAKE) -C $(BASE)/as

ld: $(LIB_DEPEND)
	+$(MAKE) -C $(BASE)/ld

cc: $(LIB_DEPEND)
	+$(MAKE) -C $(BASE)/cc

all: $(LIB_DEPEND)

clean:
	#rm -rf cscope
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
	rm -rf $(LIBDIR)

cscope:
	mkdir -p cscope
	tools/cscope_gen.sh -d $(PROOT)/inc -d $(PROOT)/src -o $(PROOT)/cscope -b q

test-as:
	$(PROOT)/bin/lccc-as

mem-as:
	valgrind --leak-check=full $(PROOT)/bin/lccc-as

.PHONY: all as ld cc clean test gtest mem debug cscope
