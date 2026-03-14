
NAME  = parser

DYNPROP_PARSER_SRC = src/DynamicProperty/DynamicPropertyParser.cpp
DYNPROP_PARSER_HDR = src/DynamicProperty/DynamicPropertyParser.hpp
DYNPROP_LEXER_SRC  = src/DynamicProperty/DynamicPropertyLexer.cpp
DYNPROP_GRAMMAR    = src/DynamicProperty/DynamicPropertyParser.y
DYNPROP_LEXER      = src/DynamicProperty/DynamicPropertyLexer.l

CXSRC = $(shell find src -name '*.cpp' ! -name 'DynamicPropertyParser.cpp' ! -name 'DynamicPropertyLexer.cpp') $(DYNPROP_PARSER_SRC) $(DYNPROP_LEXER_SRC)
CSRC  = $(shell find src -name *.c)
UNAME = $(shell uname)

OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CXSRC)) $(patsubst %.c,$(BUILD)/%.o,$(CSRC))

BUILD = build
LIBDIR = .

YAML=$(BUILD)/src/YAML
EXCEPTION=$(BUILD)/src/Exception
OUTPUT=$(BUILD)/src/Output
FRAMEWORK=$(BUILD)/src/Framework
DYNPROP=$(BUILD)/src/DynamicProperty

TARGET  = lib$(NAME).so
LIBDIR  =  .
INSTALL = /usr/local/lib
HDRINST = /usr/local/include

OBJ    := $(patsubst %.cpp,$(BUILD)/%.o,$(CXSRC)) $(patsubst %.c,$(BUILD)/%.o,$(CSRC))

ifeq ($(UNAME), Darwin)
CC       = cc
CXX      = c++
else
CC     = gcc
CXX    = g++
endif

.PHONY: all build clean copy

all: $(TARGET)

$(TARGET) : build $(DYNPROP_PARSER_SRC) $(DYNPROP_LEXER_SRC) $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -shared  -o $(BUILD)/$(TARGET) -lpthread -lre2 -lyaml-cpp $(LINK)

build :
	mkdir -p $(YAML)
	mkdir -p $(EXCEPTION)
	mkdir -p $(OUTPUT)
	mkdir -p $(FRAMEWORK)
	mkdir -p $(DYNPROP)

copy:
	cp $(BUILD)/$(TARGET) $(INSTALL)

$(BUILD)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(DYNPROP_PARSER_SRC) $(DYNPROP_PARSER_HDR): $(DYNPROP_GRAMMAR)
	bison --output=$(DYNPROP_PARSER_SRC) --defines=$(DYNPROP_PARSER_HDR) $(DYNPROP_GRAMMAR)

$(DYNPROP_LEXER_SRC): $(DYNPROP_LEXER) $(DYNPROP_PARSER_HDR)
	flex --outfile=$(DYNPROP_LEXER_SRC) $(DYNPROP_LEXER)

clean:
	rm -rf $(BUILD)
