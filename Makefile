PROJECT_ROOT := $(abspath .)
BUILD_DIR := build
INCLUDE_DIR := include
SRC_DIR := src
EXAMPLE_DIR := examples
TEST_DIR := tests

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -I$(INCLUDE_DIR) -Isubprojects/jsmn/src
SANITIZE_FLAGS := -fsanitize=address
PKG_CONFIG := pkg-config
GTEST_CFLAGS := $(shell $(PKG_CONFIG) --cflags gtest 2>/dev/null)
GTEST_LIBS := $(shell $(PKG_CONFIG) --libs gtest_main gtest 2>/dev/null)
YAML_CPP_LIBS := $(shell $(PKG_CONFIG) --libs yaml-cpp 2>/dev/null)
FRAMEWORK_CXXFLAGS := $(CXXFLAGS) $(SANITIZE_FLAGS) -fPIC
LIB_RPATH := -Wl,-rpath,$(PROJECT_ROOT)/build -Wl,-rpath,$(PROJECT_ROOT)/subprojects/jsmn/build
FRAMEWORK_LDFLAGS := $(SANITIZE_FLAGS) -Lsubprojects/jsmn/build -ljsmn $(YAML_CPP_LIBS) -Wl,-rpath,$(PROJECT_ROOT)/subprojects/jsmn/build
APP_LDFLAGS := $(SANITIZE_FLAGS) -L$(BUILD_DIR) -lparser_framework -Lsubprojects/jsmn/build -ljsmn $(YAML_CPP_LIBS) $(LIB_RPATH)

FRAMEWORK_OBJ := $(BUILD_DIR)/src/ParserFramework.o $(BUILD_DIR)/src/RuleLoader.o
FRAMEWORK_LIB := $(BUILD_DIR)/libparser_framework.so
EXAMPLE_BIN := $(BUILD_DIR)/example_parser
TEST_BIN := $(BUILD_DIR)/test_parser_framework

.PHONY: all demo example_parser test clean subprojects

all: example_parser

demo: example_parser

example_parser: subprojects $(EXAMPLE_BIN)

test: subprojects $(TEST_BIN)
	ASAN_OPTIONS=detect_leaks=0 $(TEST_BIN)

subprojects:
	$(MAKE) -C subprojects/jsmn all

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/src
	mkdir -p $(BUILD_DIR)/examples
	mkdir -p $(BUILD_DIR)/tests

$(BUILD_DIR)/src/ParserFramework.o: $(SRC_DIR)/ParserFramework.cpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) -c $(SRC_DIR)/ParserFramework.cpp -o $(BUILD_DIR)/src/ParserFramework.o

$(BUILD_DIR)/src/RuleLoader.o: $(SRC_DIR)/RuleLoader.cpp $(INCLUDE_DIR)/parser_framework/RuleLoader.hpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) -c $(SRC_DIR)/RuleLoader.cpp -o $(BUILD_DIR)/src/RuleLoader.o

$(FRAMEWORK_LIB): $(FRAMEWORK_OBJ)
	$(CXX) -shared -o $(FRAMEWORK_LIB) $(FRAMEWORK_OBJ) $(FRAMEWORK_LDFLAGS)

$(EXAMPLE_BIN): $(EXAMPLE_DIR)/example_parser.cpp $(FRAMEWORK_LIB)
	$(CXX) $(CXXFLAGS) $(SANITIZE_FLAGS) $(EXAMPLE_DIR)/example_parser.cpp $(APP_LDFLAGS) -o $(EXAMPLE_BIN)

$(TEST_BIN): $(TEST_DIR)/smoke.cpp $(FRAMEWORK_LIB)
	$(CXX) $(CXXFLAGS) $(SANITIZE_FLAGS) $(GTEST_CFLAGS) $(TEST_DIR)/smoke.cpp $(APP_LDFLAGS) $(GTEST_LIBS) -o $(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)
