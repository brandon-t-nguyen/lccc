SRCDIR = $(shell pwd)
MODULE = as

SRCMAIN = $(SRCDIR)/$(MODULE).c
SOURCES += $(shell find $(SRCDIR) -name '*.c' ! -wholename $(SRCMAIN))
SOURCES += $(shell find $(SRCDIR) -name '*.cpp' ! -wholename $(SRCMAIN))
BIN = lccc-as

OBJECTS  = $(SOURCES:$(SRCDIR)/%.c*=$(OBJDIR)/$(MODULE)/%.o)
OBJMAIN  = $(SRCMAIN:$(SRCDIR)/%.c*=$(OBJDIR)/$(MODULE)/%.o)

GTEST_DIR = test
GTEST_BIN = test_lccc-as
GTEST_SRC = $(GTEST_DIR)/*.cpp

$(OBJDIR)/$(MODULE)/%.o:$(SRCDIR)/%.cpp
	mkdir -p $(@D)	# generate the directory
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $(@)

$(OBJDIR)/$(MODULE)/%.o:$(SRCDIR)/%.c
	mkdir -p $(@D)	# generate the directory
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $(@)

$(BINDIR)/$(BIN): $(OBJECTS) $(OBJMAIN)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o $(BINDIR)/$(BIN) $(OBJECTS) $(OBJMAIN)
	@echo "Program built"

$(BINDIR)/$(GTEST_BIN): $(OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -lgtest -o $(BINDIR)/$(GTEST_BIN) $(OBJECTS) $(GTEST_SRC)
	@echo "gtest suite built"

all: $(BINDIR)/$(BIN)

gtest: $(BINDIR)/$(GTEST_BIN)
	@$(BINDIR)/$(GTEST_BIN)

clean:
	rm $(BINDIR)/$(BIN)
	rm -rf $(OBJDIR)/$(MODULE)

test: $(BINDIR)/$(BIN)
	@$(BINDIR)/$(BIN)

mem: $(BINDIR)/$(BIN)
	valgrind --leak-check=yes $(BINDIR)/$(BIN)

debug: $(BINDIR)/$(BIN)
	gdb $(BINDIR)/$(BIN)

.PHONY: all clean test gtest mem debug
