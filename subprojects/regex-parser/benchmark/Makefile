NAME  = parser
CXSRC = $(wildcard src/*.cpp)
CSRC  = $(wildcard src/*.c)

TARGET = benchmark_$(NAME)

BUILD   = build

OBJ := $(patsubst %.cpp,$(BUILD)/%.o,$(CXSRC)) $(patsubst %.c,$(BUILD)/%.o,$(CSRC))

ifeq ($(UNAME), Darwin)
CC       = cc
CXX      = c++
else
CC     = gcc
CXX    = g++
endif

all:  build $(TARGET) $(OBJ) $(LIBDEP)

$(TARGET) : build $(OBJ) $(LIBDEP)
	cd ../lib && make
	$(CXX) $(CXXFLAGS) $(CXXOBJ) $(COBJ) -L$(LNK_PATH) -o $(BUILD)/$(TARGET) -lpthread -l$(NAME) $(LIBRARIES) -lbenchmark -lbenchmark_main -Wl,-rpath,$(LNK_PATH)

build:
	mkdir -p "$(BUILD)/src"


$(LIBDEP) :
	cd $(LIBDIR) && make

run: $(TARGET)
	./$(BUILD)/$(TARGET)

$(BUILD)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
