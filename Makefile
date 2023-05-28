.PHONY: clean

CXX:=$(CXX)

ifeq ($(OS),Windows_NT)
	CXX:=clang++
endif

CXXFLAGS=-O2 -Wall -Wextra -pedantic -std=c++17 -fno-rtti

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

cli: dirs release bundle
	$(CXX) -o $(BINDIR)/toiletdb $(CXXFLAGS) -DNDEBUG -Iinclude -Isrc cli/main.cpp cli/cli.cpp $(BINDIR)/$(LIB)

cli-debug: dirs debug bundle
	$(CXX) -o $(BINDIR)/toiletdb $(CXXFLAGS) -DDEBUG -g -Iinclude -Isrc cli/main.cpp cli/cli.cpp $(BINDIR)/$(LIB)

debug: CXXFLAGS += -DDEBUG
debug: CXXFLAGS += -g
debug: $(OBJS_OUT) bundle

release: CXXFLAGS += -DNDEBUG
release: $(OBJS_OUT) bundle

bundle: $(OBJS_OUT)
	ar -rcs $(BINDIR)/$(LIB) $(OBJS_OUT)

dirs:
ifeq ($(OS),Windows_NT)
	-cmd /c mkdir $(OBJDIR)
	-cmd /c mkdir $(BINDIR)
else
	mkdir $(OBJDIR)
	mkdir $(BINDIR)
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
	$(CXX) -o $@ $(CXXFLAGS) -c $<
