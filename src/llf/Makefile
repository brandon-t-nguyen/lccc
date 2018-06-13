SRCDIR = $(shell pwd)
MODULE = llf 

SOURCES += $(shell find $(SRCDIR) -name '*.c')
SOURCES += $(shell find $(SRCDIR) -name '*.cpp')
BIN = lib$(MODULE).a

OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/$(MODULE)/%.o)

GTEST_DIR = test
GTEST_BIN = test_$(MODULE)
GTEST_SRC = $(GTEST_DIR)/*.cpp

$(OBJDIR)/$(MODULE)/%.o:$(SRCDIR)/%.cpp
	mkdir -p $(@D)	# generate the directory
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $(@)

$(OBJDIR)/$(MODULE)/%.o:$(SRCDIR)/%.c
	mkdir -p $(@D)	# generate the directory
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $(@)

$(LIBDIR)/$(BIN): $(OBJECTS)
	mkdir -p $(LIBDIR)
	ar rcs $(LIBDIR)/$(BIN) $(OBJECTS)
	echo $(SRCDIR)
	@echo "Program built"

$(BINDIR)/$(GTEST_BIN): $(OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(BINDIR)/$(GTEST_BIN) $(OBJECTS) $(GTEST_SRC) $(LIBS) -lgtest 
	@echo "gtest suite built"

all: $(LIBDIR)/$(BIN)

gtest: $(BINDIR)/$(GTEST_BIN)
	@$(BINDIR)/$(GTEST_BIN)

clean:
	rm $(LIBDIR)/$(BIN)
	rm -rf $(OBJDIR)/$(MODULE)

.PHONY: all clean test gtest mem debug
