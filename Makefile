CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3
TARGET = nonameos
SRC = NoNameOS.cpp

.PHONY: all clean debug

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $< -lpthread

debug: $(SRC)
	$(CXX) -std=c++17 -Wall -Wextra -g -O0 -o $(TARGET) $< -lpthread

clean:
	rm -f $(TARGET)
