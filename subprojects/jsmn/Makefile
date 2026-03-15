NAME 	 = jsmn

BUILD_DIR ?= build
CFLAGS   = -std=c11 -Wall -g -fPIC
CXXFLAGS = -std=c++17 -Wall -g -fPIC
BENCHMARK_FOUND  := $(shell pkg-config --exists benchmark && echo 1)
BENCHMARK_CFLAGS := $(shell pkg-config --cflags benchmark 2>/dev/null)
BENCHMARK_LIBS   := $(shell pkg-config --libs benchmark 2>/dev/null)
SIMDJSON_FOUND   := $(shell pkg-config --exists simdjson && echo 1)
SIMDJSON_CFLAGS  := $(shell pkg-config --cflags simdjson 2>/dev/null)
SIMDJSON_LIBS    := $(shell pkg-config --libs simdjson 2>/dev/null)
YYJSON_FOUND     := $(shell pkg-config --exists yyjson && echo 1)
YYJSON_CFLAGS    := $(shell pkg-config --cflags yyjson 2>/dev/null)
YYJSON_LIBS      := $(shell pkg-config --libs yyjson 2>/dev/null)
RAPIDJSON_FOUND  := $(shell if [ -d /usr/include/rapidjson ] || [ -d /usr/local/include/rapidjson ]; then echo 1; fi)

BENCH_COMPARE_CFLAGS :=
BENCH_COMPARE_LIBS   :=

ifeq ($(SIMDJSON_FOUND),1)
BENCH_COMPARE_CFLAGS += $(SIMDJSON_CFLAGS) -DHAVE_SIMDJSON=1
BENCH_COMPARE_LIBS   += $(SIMDJSON_LIBS)
endif

ifeq ($(YYJSON_FOUND),1)
BENCH_COMPARE_CFLAGS += $(YYJSON_CFLAGS) -DHAVE_YYJSON=1
BENCH_COMPARE_LIBS   += $(YYJSON_LIBS)
endif

ifeq ($(RAPIDJSON_FOUND),1)
BENCH_COMPARE_CFLAGS += -DHAVE_RAPIDJSON=1
endif

CC=gcc
CXX=g++

.PHONY: all benchmark build clean cmd_example doc jsondump simple test

# Automated code gerneation. Don't fuck with it 
# unless you know what your doing!
TARGET  = lib$(NAME).so
TEST	= test_$(NAME)
BENCH	= benchmark_$(NAME)

# Yacc file go first because they generat headers
YACSRC = $(patsubst %.y,%.tab.c,$(wildcard src/*.y))
LEXSRC = $(patsubst %.l,%.lex.c,$(wildcard src/*.l))
CXXSRC = $(wildcard src/*.cpp)
CSRC   = $(wildcard src/*.c)

# YACC goes first!
OBJ := $(patsubst %.tab.c,$(BUILD_DIR)/%.tab.o,$(YACSRC))
OBJ += $(patsubst %.lex.c,$(BUILD_DIR)/%.lex.o,$(LEXSRC))
OBJ += $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CXXSRC))
OBJ += $(patsubst %.c,$(BUILD_DIR)/%.o,$(CSRC))

# Test code
TESTCXXSRC := $(wildcard test/*.cpp)
TESTCSRC   := $(wildcard test/*.c)
TESTOBJ	   := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(TESTCXXSRC))
TESTOBJ	   += $(patsubst %.c,$(BUILD_DIR)/%.o,$(TESTCSRC))

BENCHCXXSRC := $(wildcard bench/*.cpp)
BENCHOBJ    := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(BENCHCXXSRC))

BUILDTARGET = $(BUILD_DIR)/$(TARGET)
TESTTARGET  = $(BUILD_DIR)/$(TEST)
BENCHTARGET = $(BUILD_DIR)/$(BENCH)

# Force rebuild of flex and bison files each time
all: $(BUILDTARGET)
# 	rm -f src/*.tab.* src/*.lex.*

test: $(BUILDTARGET) $(TESTTARGET)
	$(TESTTARGET)

benchmark: $(BUILDTARGET) $(BENCHTARGET)
	$(BENCHTARGET)

# Dynamic Build Rules
$(BUILDTARGET) : build $(OBJ)
	$(CXX) $(CXXFLAGS) -fPIC -L$(BUILD_DIR) -Isrc $(OBJ) -shared  -o $(BUILDTARGET)

$(TESTTARGET): build $(TESTOBJ)
	$(CXX) $(CXXFLAGS) -L$(BUILD_DIR) -Isrc -Itest $(TESTOBJ) -ljsmn -lgtest -lpthread -lglog -o $(TESTTARGET) -Wl,-rpath,$(BUILD_DIR)

$(BENCHTARGET): build $(BENCHOBJ)
	@if [ "$(BENCHMARK_FOUND)" != "1" ]; then \
		echo "Google Benchmark not found. Install libbenchmark or expose it via pkg-config."; \
		exit 1; \
	fi
	$(CXX) $(CXXFLAGS) $(BENCHMARK_CFLAGS) $(BENCH_COMPARE_CFLAGS) -L$(BUILD_DIR) -Isrc -Ibench $(BENCHOBJ) -ljsmn $(BENCHMARK_LIBS) $(BENCH_COMPARE_LIBS) -lpthread -o $(BENCHTARGET) -Wl,-rpath,$(BUILD_DIR)

cmd_example: $(BUILDTARGET)
	g++ -std=c++17 -g -Wall -Isrc -c examples/cmd_example.cpp -o cmd_example.o
	g++ -std=c++17 -g -L$(BUILD_DIR) -o $(BUILD_DIR)/cmd_example cmd_example.o -ljsmn -Wl,-rpath,$(BUILD_DIR)
	rm -f cmd_example.o

jsondump: $(BUILDTARGET)
	g++ -std=c++17 -g -Wall -Isrc -c examples/jsondump.cpp -o jsondump.o
	g++ -std=c++17 -g -L$(BUILD_DIR) -o jsondump jsondump.o -ljsmn -Wl,-rpath,$(BUILD_DIR)
	rm -f jsondump.o

simple: $(BUILDTARGET)
	g++ -std=c++17 -g -Wall -Isrc -c examples/simple.cpp -o simple.o
	g++ -std=c++17 -g -L$(BUILD_DIR) -o simple simple.o -ljsmn -Wl,-rpath,$(BUILD_DIR)
	rm -f simple.o

doc:
	cd docs/Doxygen && doxygen Doxyfile

build:
	mkdir -p $(BUILD_DIR)/src
	mkdir -p $(BUILD_DIR)/test
	mkdir -p $(BUILD_DIR)/bench

src/%.tab.c: src/%.y
	bison -d $< -o $@

src/%.lex.c: src/%.l
	flex -o $@ $<

$(BUILD_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -L$(BUILD_DIR) -Isrc -Itest $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -L$(BUILD_DIR) -Isrc -Itest -c $< -o $@

$(BUILD_DIR)/bench/%.o: bench/%.cpp
	$(CXX) -c $(CXXFLAGS) $(BENCHMARK_CFLAGS) $(BENCH_COMPARE_CFLAGS) -L$(BUILD_DIR) -Isrc -Ibench -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) cmd_example jsondump simple test.bin tests src/*.tab.h src/*.tab.c src/*.lex.c docs/html

.PRECIOUS: *.tab.c 
