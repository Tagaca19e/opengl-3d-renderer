# The compiler and flags
CC = clang++
CFLAGS = -std=c++17 -stdlib=libc++ -D GLEW_STATIC -D FMT_HEADER_ONLY
# The linker and flags
LD = clang++
# Static libraries linked: glfw3 (.a file in ./lib),
# Shared libraries linked: OpenGL, GL Extension Wrangler, dynamic loader, X11 window management, POSIX threads
LFLAGS = -std=c++17 -stdlib=libc++ -L/opt/homebrew/lib -lglfw -lGLEW -framework CoreVideo -framework OpenGL -framework IOKit -framework Cocoa -framework Carbon 

# Flags for when calling make debug. Lets you use #if DEBUG in .cpp files

# Flags for when calling make release. Lets you ignore debug info and uses level 2 optimization


# Makes sure that headers are searched for in both the external code libraries (./include)
# as well as the internal code libraries (./headers)
INC =-I./include -I./headers

# Specifies where the source files for compilation are located.
# This allows CPP files in the following location variations:
#	./src/file1.cpp
#	./src/folder/file2.cpp
SRCDIR = ./src
SRC     = $(wildcard $(SRCDIR)/*.cpp $(SRCDIR)/*/*.cpp)
# Ignore nfd_win.cpp for Linux makefile builds
SRC := $(filter-out $(SRCDIR)/nfd/nfd_win.cpp, $(SRC))

# Specifies the names of object files for each of the source files using make notation
OBJDIR = ./obj
#OBJS := $(SRC:.cpp=.o)
#OBJS := $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
OBJS := $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Name and location of outputs (object files, executables)
BINDIR =  ./bin
OBJDIR = ./obj
EXENAME = 3480-main

# Debug build specifications
DBGDIR = debug
DBGEXE = $(BINDIR)/$(DBGDIR)/$(EXENAME)
DBGOBJ = $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/$(DBGDIR)/%.o)
DEBUGFLAGS = -g -D DEBUG


# Release build specifications
RELDIR = release
RELEXE = $(BINDIR)/$(RELDIR)/$(EXENAME)
RELOBJ = $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/$(RELDIR)/%.o)
RELEASEFLAGS = -D NDEBUG -O2

.PHONY: all clean debug release macdebug macrelease wildbundle dump

all: debug

# Debug build rules
debug: $(DBGEXE)

$(DBGEXE): $(DBGOBJ)
	@mkdir -p $$(dirname $(DBGEXE))
	$(LD) $(CFLAGS) $(DEBUGFLAGS) $(DBGOBJ) -o $(DBGEXE) $(LFLAGS)

# Creates object files by compiling each source file individually
$(OBJDIR)/$(DBGDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(INC) -c -o $@ $<


# Release build rules
release: $(RELEXE)

$(RELEXE): $(RELOBJ)
	@mkdir -p $$(dirname $(RELEXE))
	$(LD) $(CFLAGS) $(RELEASEFLAGS) $(RELOBJ) -o $(RELEXE) $(LFLAGS)

# Creates object files by compiling each source file individually
$(OBJDIR)/$(RELDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS) $(RELEASEFLAGS) $(INC) -c -o $@ $<

# Info dump of make variables
dump:
	@echo src files: $(SRC)
	@echo CFLAGS: $(CFLAGS)
	@echo LFLAGS: $(LFLAGS)
	@echo OBJS: $(OBJS)
	@echo DBGDIR and DBGEXE: $(DBGDIR), $(DBGEXE)
	@echo DBGOBJ: $(DBGOBJ)
	@echo RELDIR and RELEXE: $(RELDIR), $(RELEXE)
	@echo RELOBJ: $(RELOBJ)

# Cleans everything up
clean:
	@echo removing $(OBJDIR) ...
	@rm -rf $(OBJDIR)
	@echo removing $(BINDIR) ...
	@rm -rf $(BINDIR)
	@echo "so clean! "

wildbundle:
	tar --exclude ".vs" -czvf 3480-wildbundle.tgz ./*
