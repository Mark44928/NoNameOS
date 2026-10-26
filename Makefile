# Prefer the native Termux compiler when available; otherwise use the regular
# GNU C++ toolchain. This keeps the repo working on Android/Termux without
# forcing developers to remember the `clang++` workaround.
ifeq ($(origin CXX), default)
  ifneq ($(TERMUX_VERSION),)
    CXX := clang++
  else
    CXX := g++
  endif
endif

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
