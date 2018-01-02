export PROOT  = $(shell pwd)
export BINDIR = $(PROOT)/bin
export OBJDIR = $(PROOT)/obj
export LIBDIR = $(PROOT)/lib

include vars.mk

ifeq ($(PLATFORM), UNIX)
export CC = gcc
export CXX = g++
export LD = ld
export AR = ar
else ifeq ($(PLATFORM), WINDOWS)
export CC = x86_64-w64-mingw32-gcc
export CXX = x86_64-w64-mingw32-g++
export LD = x86_64-w64-mingw32-ld
export AR = x86_64-w64-mingw32-ar
endif

export CFLAGS = -Wall -Wextra -Wno-unused-parameter -g -DPLATFORM_$(PLATFORM) -DDEF_$(DEFAULT)
export LIBS   = -L$(LIBDIR) -lbtn
export INCLUDE = -I$(PROOT)/inc

export LIB_DEPEND = $(LIBDIR)/libbtn.a

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
	@#rm -rf cscope
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
	rm -rf $(LIBDIR)
	+$(MAKE) clean -C external/libbtn

cscope:
	mkdir -p cscope
	tools/cscope_gen.sh -d $(PROOT)/inc -d $(PROOT)/src -o $(PROOT)/cscope -b q

test-as:
	$(PROOT)/bin/lccc-as

mem-as:
	valgrind --leak-check=full $(PROOT)/bin/lccc-as

.PHONY: all as ld cc clean test gtest mem debug cscope
