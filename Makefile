.PHONY: clean

CXX=g++
CXXFLAGS=-O2 -Wall -Wextra -pedantic -std=c++17 -fno-rtti

EXE=toiletdb

SRCDIR=src
OBJDIR=obj
BINDIR=build

FILES=common.cpp debug.cpp errors.cpp types.cpp format.cpp parser.cpp table.cpp
SRC_FILES=$(addprefix $(SRCDIR)/, $(FILES))

OBJS=$(FILES:.cpp=.o)
OBJS_OUT=$(addprefix $(OBJDIR)/, $(OBJS))

cli: dirs release bundle
	$(CXX) -o $(BINDIR)/toiletdb $(CXXFLAGS) -DNDEBUG -Iinclude -Isrc cli/main.cpp cli/cli.cpp -L$(BINDIR) -ltoiletdb

cli-debug: dirs debug bundle
	$(CXX) -o $(BINDIR)/toiletdb $(CXXFLAGS) -DDEBUG -g -Iinclude -Isrc cli/main.cpp cli/cli.cpp -L$(BINDIR) -ltoiletdb

debug: CXXFLAGS += -DDEBUG
debug: CXXFLAGS += -g
debug: dirs $(OBJS_OUT) bundle

release: CXXFLAGS += -DNDEBUG
release: dirs $(OBJS_OUT) bundle

bundle: $(OBJS_OUT)
	ar -rcs $(BINDIR)/libtoiletdb.a $(OBJS_OUT)
	ranlib $(BINDIR)/libtoiletdb.a

dirs:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

clean:
	rm -r $(OBJDIR)
	rm -r $(BINDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<
