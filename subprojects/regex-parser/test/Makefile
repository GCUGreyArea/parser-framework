NAME	= parser
TARGET  = test_$(NAME)

CXXSRC = $(wildcard src/*.cpp)
CSRC   = $(wildcard src/*.c)

LIB     = lib$(NAME).so
BUILD   = build
LIBDIR  = ../lib
LIBDEP  = $(LIBDIR)/$(BUILD)/$(LIB)

CXXOBJ    := $(patsubst %.cpp,$(BUILD)/%.o,$(CXXSRC))
COBJ      := $(patsubst %.c,$(BUILD)/%.o,$(CSRC))
PWD       := $(shell pwd)
LINK      := -L$(LNK_PATH) -lgtest -lpthread -lglog

LIBRARIES = -lre2 -lhs
CFLAGS   = -std=c11 -Wall -g -fsanitize=address 
CXXFLAGS = -std=c++17 -Wall -g -O1 -fsanitize=address $(INC)
TEST_RUN_ENV ?= ASAN_OPTIONS=detect_leaks=0

ifeq ($(UNAME), Darwin)
CC       = cc
CXX      = c++
else
CC     = gcc
CXX    = g++
endif


all: $(TARGET) $(OBJ) $(COBJ) $(LIBDEP)

$(TARGET) : build $(CXXOBJ) $(COBJ)
	@echo "LNK_PATH : $(LIBTARGET)"
	$(CXX) $(CXXFLAGS) -L$(LNK_PATH) $(CXXOBJ) $(COBJ) -o $(BUILD)/$(TARGET) -l$(NAME) $(LIBRARIES) $(LINK) -Wl,-rpath,$(LNK_PATH)

build:
	mkdir -p "$(BUILD)/src"

$(LIBDEP) :
	cd $(LIBDIR) && make

$(BUILD)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -c $< -o $@

run:
	cd .. && $(TEST_RUN_ENV) ./test/$(BUILD)/$(TARGET)

leak-check:
	cd .. && ASAN_OPTIONS=detect_leaks=1 ./test/$(BUILD)/$(TARGET)

clean:
	rm -rf $(BUILD)
