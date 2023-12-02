# This should support both Windows/Clang and Linux :3

.PHONY: clean

CXX:=$(CXX)

ifeq ($(OS),Windows_NT)
	CXX:=clang++
endif

CXXFLAGS=-Wall -Wextra -pedantic -std=c++23 -fno-rtti -Wno-deprecated -Wno-gnu
CCFLAGS=-Wall -Wextra -std=c11 -std=c99 -Wno-deprecated -Wno-gnu

EXE:=toiletdb
LIB:=toiletdb.lib

ifeq ($(OS),Windows_NT)
	EXE:=toiletdb.exe
endif

SRCDIR=src
OBJDIR=obj
BINDIR=build

FILES=common.cpp debug.cpp errors.cpp types.cpp format.cpp parser.cpp table.cpp
SRC_FILES=$(addprefix $(SRCDIR)/, $(FILES))

OBJS=$(FILES:.cpp=.o)
OBJS_OUT=$(addprefix $(OBJDIR)/, $(OBJS))

cli: dirs release toiletline bundle
	$(CXX) -o $(BINDIR)/$(EXE) $(CXXFLAGS) -O2 -DNDEBUG -Iinclude cli/main.cpp cli/cli.cpp $(OBJDIR)/toiletline.o $(BINDIR)/$(LIB)

cli-debug: dirs debug toiletline-debug bundle
	$(CXX) -o $(BINDIR)/$(EXE) $(CXXFLAGS) -DDEBUG -g -Iinclude cli/main.cpp cli/cli.cpp $(OBJDIR)/toiletline.o $(BINDIR)/$(LIB)

debug: CXXFLAGS += -DDEBUG
debug: CXXFLAGS += -g
debug: CCFLAGS += -g
debug: dirs $(OBJS_OUT) bundle

release: CXXFLAGS += -DNDEBUG
release: dirs $(OBJS_OUT) bundle

toiletline-debug:
	$(CC) $(CCFLAGS) -O2 -c -o $(OBJDIR)/toiletline.o cli/toiletline.c

toiletline:
	$(CC) $(CCFLAGS) -O2 -c -o $(OBJDIR)/toiletline.o cli/toiletline.c

bundle: $(OBJS_OUT)
	ar -rcs $(BINDIR)/$(LIB) $(OBJS_OUT)

dirs:
ifeq ($(OS),Windows_NT)
	-cmd /c mkdir $(OBJDIR)
	-cmd /c mkdir $(BINDIR)
else
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)
endif

clean:
ifeq ($(OS),Windows_NT)
		cmd /c rmdir $(OBJDIR) /s /q
		cmd /c rmdir $(BINDIR) /s /q
else
		rm -r $(OBJDIR)
		rm -r $(BINDIR)
endif

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) -o $@ $(CXXFLAGS) -O2 -c $<
