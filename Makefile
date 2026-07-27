CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = nonameos
SRC = NoNameOS.cpp

.PHONY: all clean debug readme

all: $(TARGET)

readme:
	bash update-readme.sh

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $< -lpthread

debug: $(SRC)
	$(CXX) $(CXXFLAGS) -g -O0 -o $(TARGET)-debug $< -lpthread

clean:
	rm -f $(TARGET) $(TARGET)-debug
