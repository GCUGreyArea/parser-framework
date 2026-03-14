PROJECT_ROOT := $(abspath .)
BUILD_DIR := build
INCLUDE_DIR := include
SRC_DIR := src
EXAMPLE_DIR := examples
TEST_DIR := tests

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -I$(INCLUDE_DIR) -Isubprojects/jsmn/src
LDFLAGS := -Lsubprojects/jsmn/build -ljsmn -Wl,-rpath,$(PROJECT_ROOT)/subprojects/jsmn/build

FRAMEWORK_OBJ := $(BUILD_DIR)/src/ParserFramework.o
FRAMEWORK_LIB := $(BUILD_DIR)/libparser_framework.a
DEMO_BIN := $(BUILD_DIR)/demo
TEST_BIN := $(BUILD_DIR)/smoke_test

.PHONY: all demo test clean subprojects

all: demo

demo: subprojects $(DEMO_BIN)

test: subprojects $(TEST_BIN)
	$(TEST_BIN)

subprojects:
	$(MAKE) -C subprojects/jsmn all

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/src
	mkdir -p $(BUILD_DIR)/examples
	mkdir -p $(BUILD_DIR)/tests

$(FRAMEWORK_OBJ): $(SRC_DIR)/ParserFramework.cpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/ParserFramework.cpp -o $(FRAMEWORK_OBJ)

$(FRAMEWORK_LIB): $(FRAMEWORK_OBJ)
	ar rcs $(FRAMEWORK_LIB) $(FRAMEWORK_OBJ)

$(DEMO_BIN): $(EXAMPLE_DIR)/demo.cpp $(FRAMEWORK_LIB)
	$(CXX) $(CXXFLAGS) $(EXAMPLE_DIR)/demo.cpp $(FRAMEWORK_LIB) $(LDFLAGS) -o $(DEMO_BIN)

$(TEST_BIN): $(TEST_DIR)/smoke.cpp $(FRAMEWORK_LIB)
	$(CXX) $(CXXFLAGS) $(TEST_DIR)/smoke.cpp $(FRAMEWORK_LIB) $(LDFLAGS) -o $(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)
