CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = nonameos
SRC = NoNameOS.cpp

.PHONY: all clean debug

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $< -lpthread

debug: $(SRC)
	$(CXX) $(CXXFLAGS) -g -O0 -o $(TARGET)-debug $< -lpthread

clean:
	rm -f $(TARGET) $(TARGET)-debug
