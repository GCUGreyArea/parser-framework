PROJECT_ROOT := $(abspath .)
BUILD_ROOT := build
INCLUDE_DIR := include
SRC_DIR := src
EXAMPLE_DIR := examples
TEST_DIR := tests

CXX := g++
BUILD_MODE ?= debug
BUILD_DIR := $(BUILD_ROOT)/$(BUILD_MODE)
SUBPROJECT_JSMN_BUILD_DIR := $(PROJECT_ROOT)/subprojects/jsmn/build/$(BUILD_MODE)
SUBPROJECT_REGEX_BUILD_DIR := $(PROJECT_ROOT)/subprojects/regex-parser/lib/build/$(BUILD_MODE)
COMMON_CXXFLAGS := -std=c++17 -Wall -Wextra -I$(INCLUDE_DIR) -Isubprojects/jsmn/src -Isubprojects/regex-parser/lib -Isubprojects/regex-parser/lib/src
COMMON_REGEX_CFLAGS := -std=c11 -Wall -I. -Isrc
COMMON_REGEX_CXXFLAGS := -std=c++17 -fPIC -Wall -I. -Isrc
COMMON_JSMN_CFLAGS := -std=c11 -Wall -fPIC
COMMON_JSMN_CXXFLAGS := -std=c++17 -Wall -fPIC
SANITIZE_FLAGS :=
MODE_FLAGS :=
RUN_ENV :=
EXTRA_APP_LIBS :=

ifeq ($(BUILD_MODE),release)
MODE_FLAGS := -O2 -DNDEBUG
EXTRA_APP_LIBS := -lasan
else
MODE_FLAGS := -O0 -g
SANITIZE_FLAGS := -fsanitize=address
RUN_ENV := ASAN_OPTIONS=detect_leaks=0
endif

CXXFLAGS := $(COMMON_CXXFLAGS) $(MODE_FLAGS)
PKG_CONFIG := pkg-config
GTEST_CFLAGS := $(shell $(PKG_CONFIG) --cflags gtest 2>/dev/null)
GTEST_LIBS := $(shell $(PKG_CONFIG) --libs gtest_main gtest 2>/dev/null)
YAML_CPP_LIBS := $(shell $(PKG_CONFIG) --libs yaml-cpp 2>/dev/null)
FRAMEWORK_CXXFLAGS := $(CXXFLAGS) $(SANITIZE_FLAGS) -fPIC
DEPFLAGS := -MMD -MP
LIB_RPATH := -Wl,-rpath,$(PROJECT_ROOT)/$(BUILD_DIR) -Wl,-rpath,$(SUBPROJECT_JSMN_BUILD_DIR) -Wl,-rpath,$(SUBPROJECT_REGEX_BUILD_DIR)
FRAMEWORK_LDFLAGS := $(SANITIZE_FLAGS) -L$(SUBPROJECT_JSMN_BUILD_DIR) -ljsmn -L$(SUBPROJECT_REGEX_BUILD_DIR) -lparser -lre2 -lhs $(YAML_CPP_LIBS) -Wl,-rpath,$(SUBPROJECT_JSMN_BUILD_DIR) -Wl,-rpath,$(SUBPROJECT_REGEX_BUILD_DIR)
APP_LDFLAGS := $(EXTRA_APP_LIBS) $(SANITIZE_FLAGS) -L$(BUILD_DIR) -lparser_framework -L$(SUBPROJECT_JSMN_BUILD_DIR) -ljsmn -L$(SUBPROJECT_REGEX_BUILD_DIR) -lparser -lre2 -lhs $(YAML_CPP_LIBS) $(LIB_RPATH)

FRAMEWORK_OBJ := $(BUILD_DIR)/src/DynamicPropertyEngine.o $(BUILD_DIR)/src/IngestionBundle.o $(BUILD_DIR)/src/IngestionPipeline.o $(BUILD_DIR)/src/JSONParsingEngine.o $(BUILD_DIR)/src/KVParsingEngine.o $(BUILD_DIR)/src/MessageLoader.o $(BUILD_DIR)/src/ParserFramework.o $(BUILD_DIR)/src/RegexParsingEngine.o $(BUILD_DIR)/src/ReportAnalyzer.o $(BUILD_DIR)/src/RuleLoader.o $(BUILD_DIR)/src/utils/Args.o
FRAMEWORK_DEP := $(FRAMEWORK_OBJ:.o=.d)
FRAMEWORK_LIB := $(BUILD_DIR)/libparser_framework.so
EXAMPLE_BIN := $(BUILD_DIR)/example_parser
EXAMPLE_DEP := $(BUILD_DIR)/examples/example_parser.d
REPORT_BIN := $(BUILD_DIR)/example_breach_report
REPORT_DEP := $(BUILD_DIR)/examples/example_breach_report.d
INGESTION_BIN := $(BUILD_DIR)/example_ingestion_bundle
INGESTION_DEP := $(BUILD_DIR)/examples/example_ingestion_bundle.d
TEST_BIN := $(BUILD_DIR)/test_parser_framework
TEST_DEP := $(BUILD_DIR)/tests/smoke.d

.PHONY: all clean clean_all clean_subprojects demo example_breach_report example_ingestion_bundle example_parser release subprojects test

all: example_parser

demo: example_parser

example_breach_report: subprojects $(REPORT_BIN)

example_ingestion_bundle: subprojects $(INGESTION_BIN)

example_parser: subprojects $(EXAMPLE_BIN)

release:
	$(MAKE) BUILD_MODE=release example_parser example_breach_report example_ingestion_bundle

test: subprojects $(TEST_BIN)
	$(RUN_ENV) $(TEST_BIN)

subprojects:
	$(MAKE) -C subprojects/jsmn all \
		BUILD_DIR=build/$(BUILD_MODE) \
		CFLAGS="$(COMMON_JSMN_CFLAGS) $(MODE_FLAGS)" \
		CXXFLAGS="$(COMMON_JSMN_CXXFLAGS) $(MODE_FLAGS)"
	$(MAKE) -C subprojects/regex-parser/lib all \
		BUILD=build/$(BUILD_MODE) \
		CFLAGS="$(COMMON_REGEX_CFLAGS) $(MODE_FLAGS) $(SANITIZE_FLAGS)" \
		CXXFLAGS="$(COMMON_REGEX_CXXFLAGS) $(MODE_FLAGS) $(SANITIZE_FLAGS)"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/src
	mkdir -p $(BUILD_DIR)/examples
	mkdir -p $(BUILD_DIR)/tests

$(BUILD_DIR)/src/utils:
	mkdir -p $(BUILD_DIR)/src/utils

$(BUILD_DIR)/src/DynamicPropertyEngine.o: $(SRC_DIR)/DynamicPropertyEngine.cpp $(INCLUDE_DIR)/parser_framework/DynamicPropertyEngine.hpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/DynamicPropertyEngine.cpp -o $(BUILD_DIR)/src/DynamicPropertyEngine.o

$(BUILD_DIR)/src/IngestionBundle.o: $(SRC_DIR)/IngestionBundle.cpp $(INCLUDE_DIR)/parser_framework/IngestionBundle.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/IngestionBundle.cpp -o $(BUILD_DIR)/src/IngestionBundle.o

$(BUILD_DIR)/src/IngestionPipeline.o: $(SRC_DIR)/IngestionPipeline.cpp $(INCLUDE_DIR)/parser_framework/IngestionBundle.hpp $(INCLUDE_DIR)/parser_framework/IngestionPipeline.hpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp $(INCLUDE_DIR)/parser_framework/ReportAnalyzer.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/IngestionPipeline.cpp -o $(BUILD_DIR)/src/IngestionPipeline.o

$(BUILD_DIR)/src/JSONParsingEngine.o: $(SRC_DIR)/JSONParsingEngine.cpp $(INCLUDE_DIR)/parser_framework/ParsingEngines.hpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/JSONParsingEngine.cpp -o $(BUILD_DIR)/src/JSONParsingEngine.o

$(BUILD_DIR)/src/KVParsingEngine.o: $(SRC_DIR)/KVParsingEngine.cpp $(INCLUDE_DIR)/parser_framework/ParsingEngines.hpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/KVParsingEngine.cpp -o $(BUILD_DIR)/src/KVParsingEngine.o

$(BUILD_DIR)/src/MessageLoader.o: $(SRC_DIR)/MessageLoader.cpp $(INCLUDE_DIR)/parser_framework/MessageLoader.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/MessageLoader.cpp -o $(BUILD_DIR)/src/MessageLoader.o

$(BUILD_DIR)/src/ParserFramework.o: $(SRC_DIR)/ParserFramework.cpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/ParserFramework.cpp -o $(BUILD_DIR)/src/ParserFramework.o

$(BUILD_DIR)/src/RegexParsingEngine.o: $(SRC_DIR)/RegexParsingEngine.cpp $(INCLUDE_DIR)/parser_framework/ParsingEngines.hpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/RegexParsingEngine.cpp -o $(BUILD_DIR)/src/RegexParsingEngine.o

$(BUILD_DIR)/src/ReportAnalyzer.o: $(SRC_DIR)/ReportAnalyzer.cpp $(INCLUDE_DIR)/parser_framework/DynamicPropertyEngine.hpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp $(INCLUDE_DIR)/parser_framework/ReportAnalyzer.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/ReportAnalyzer.cpp -o $(BUILD_DIR)/src/ReportAnalyzer.o

$(BUILD_DIR)/src/RuleLoader.o: $(SRC_DIR)/RuleLoader.cpp $(INCLUDE_DIR)/parser_framework/ParsingEngines.hpp $(INCLUDE_DIR)/parser_framework/RuleLoader.hpp $(INCLUDE_DIR)/parser_framework/ParserFramework.hpp | $(BUILD_DIR)
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/RuleLoader.cpp -o $(BUILD_DIR)/src/RuleLoader.o

$(BUILD_DIR)/src/utils/Args.o: $(SRC_DIR)/utils/Args.cpp $(INCLUDE_DIR)/parser_framework/utils/Args.hpp | $(BUILD_DIR) $(BUILD_DIR)/src/utils
	$(CXX) $(FRAMEWORK_CXXFLAGS) $(DEPFLAGS) -c $(SRC_DIR)/utils/Args.cpp -o $(BUILD_DIR)/src/utils/Args.o

$(FRAMEWORK_LIB): $(FRAMEWORK_OBJ)
	$(CXX) -shared -o $(FRAMEWORK_LIB) $(FRAMEWORK_OBJ) $(FRAMEWORK_LDFLAGS)

$(EXAMPLE_BIN): $(EXAMPLE_DIR)/example_parser.cpp $(FRAMEWORK_LIB) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SANITIZE_FLAGS) $(DEPFLAGS) -MF $(EXAMPLE_DEP) -MT $@ $(EXAMPLE_DIR)/example_parser.cpp $(APP_LDFLAGS) -o $(EXAMPLE_BIN)

$(REPORT_BIN): $(EXAMPLE_DIR)/example_breach_report.cpp $(FRAMEWORK_LIB) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SANITIZE_FLAGS) $(DEPFLAGS) -MF $(REPORT_DEP) -MT $@ $(EXAMPLE_DIR)/example_breach_report.cpp $(APP_LDFLAGS) -o $(REPORT_BIN)

$(INGESTION_BIN): $(EXAMPLE_DIR)/example_ingestion_bundle.cpp $(FRAMEWORK_LIB) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SANITIZE_FLAGS) $(DEPFLAGS) -MF $(INGESTION_DEP) -MT $@ $(EXAMPLE_DIR)/example_ingestion_bundle.cpp $(APP_LDFLAGS) -o $(INGESTION_BIN)

$(TEST_BIN): $(TEST_DIR)/smoke.cpp $(FRAMEWORK_LIB) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SANITIZE_FLAGS) $(GTEST_CFLAGS) $(DEPFLAGS) -MF $(TEST_DEP) -MT $@ $(TEST_DIR)/smoke.cpp $(APP_LDFLAGS) $(GTEST_LIBS) -o $(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)

clean_all:
	rm -rf $(BUILD_ROOT)

clean_subprojects:
	$(MAKE) -C subprojects/jsmn clean BUILD_DIR=build/$(BUILD_MODE)
	$(MAKE) -C subprojects/regex-parser/lib clean BUILD=build/$(BUILD_MODE)

-include $(FRAMEWORK_DEP) $(EXAMPLE_DEP) $(REPORT_DEP) $(INGESTION_DEP) $(TEST_DEP)
