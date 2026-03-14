TARGET = parser

BUILD	   = build
TESTDIR    = test
BENCHDIR   = benchmark
LIBDIR     = lib
DOXYDIR	   = docs
BUILD      = build
DEP		   = dependancies

VERSION    = v1.13.0
DEP        = dependancies

TESTTARGET = $(TESTDIR)/$(BUILD)/test_$(TARGET)
BNCTARGET  = $(BENCHDIR)/$(BUILD)/benchmark_$(TARGET)
LIBTARGET  = $(PWD)/$(LIBDIR)/$(BUILD)/lib$(TARGET).so
DOCTARGET  = $(DOXYDIR)/generated

SRC = $(shell find src -name '*.cpp')

OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(SRC))

INC = -I$(PWD)/$(LIBDIR) -I$(PWD)/$(LIBDIR)/src
LNK_PATH = $(PWD)/lib/build 
    
LIBRARIES = -lyaml-cpp -lre2 -lhs
CFLAGS   = -std=c11 -Wall -g -fsanitize=address $(INC)
CXXFLAGS = -std=c++17 -fPIC -Wall -g -fsanitize=address $(INC)

export CXXFLAGS
export CFLAGS
export LNK_PATH
export INC
export LIBRARIES
export LIBTARGET

ARGS =

PWD :=$(shell pwd)
UNAME := $(shell uname)

all: $(LIBTARGET) $(TARGET)

$(TARGET) : build $(LIBTARGET) $(TESTTARGET) $(BENCHTARGET) $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(BUILD)/$(TARGET) -L$(LNK_PATH) -l$(TARGET) $(LIBRARIES) -Wl,-rpath,$(LNK_PATH)

build :
	mkdir -p "$(BUILD)/src"

$(LIBTARGET) :
	cd $(LIBDIR) && make 

$(TESTTARGET) : $(LIBTARGET)
	cd $(TESTDIR) && make

$(BNCTARGET) : $(LIB)
	cd $(BENCHDIR) && make

$(DOCTARGET) :
	cd $(DOXYDIR) && make

$(BUILD)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -c $< -o $@

clean:
	cd $(DOXYDIR) && make clean
	cd $(BENCHDIR) && make clean
	cd $(TESTDIR) && make clean
	cd $(LIBDIR) && make clean
	rm -rf $(BUILD)

valgrind: $(TARGET)
	valgrind --leak-check=full ./$(TARGET)

docs: $(DOCTARGET)
	cd $(DOXYDIR) && doxygen Doxyfile

test: $(TESTTARGET)
	cd $(LIBDIR) && make
	cd $(TESTDIR) && make

test-run: $(TESTTARGET)
	cd $(TESTDIR) && make run

test-leak-check: $(TESTTARGET)
	cd $(TESTDIR) && make leak-check

benchmark: $(BNCTARGET)
	cd $(LIBDIR) && make
	cd $(BENCHDIR) && make

project:
	cd $(LIBDIR) && make
	cd $(TESTDIR) && make
	cd $(BENCHDIR) && make
	make

help:
	@echo "\n\n"
	@echo "============================================================================================="
	@echo "Run: 'make' to build the project"
	@echo "Run: 'make test' to build unit tests"
	@echo "Run: 'make benchmark' to build benchmarks - currently no benchmarks are implemented"
	@echo "Run: 'make doxygen' to build doxygen documentation"
	@echo "This project depends on:"
	@echo "\tGoogles test framework availible at https://github.com/google/googletest"
	@echo "\tGoogles benchmark framework availible at https://github.com/google/benchmark"
	@echo "\tyaml-cpp availible at https://github.com/jbeder/yaml-cpp"
	@echo "============================================================================================="
	@echo "\n\n"
