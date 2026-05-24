# NoNameOS
Pure C++ Hobbyist OS simulation.

### 😬 Warming: NoNameOS is **NOT** a OS. it was just a C++ OS simulation.

## 🚀 How to Build & Run

NoNameOS is written in pure C++ and uses standard POSIX headers, so it compiles natively on almost any Linux-based system. 

### 📱 Android (Termux)
```bash
pkg install clang
clang++ -O3 NoNameOS.cpp -o nonameos
./nonameos
```

### 🐧 Debian / Ubuntu
```bash
sudo apt update && sudo apt install g++
g++ -O3 NoNameOS.cpp -o nonameos
./nonameos
```

### 🎩 Fedora
```bash
sudo dnf install gcc-c++
g++ -O3 NoNameOS.cpp -o nonameos
./nonameos
```

### 🏹 Arch Linux
```bash
sudo pacman -S gcc
g++ -O3 NoNameOS.cpp -o nonameos
./nonameos
```

