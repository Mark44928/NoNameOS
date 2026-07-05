![NoNameOS Screenshot](https://github.com/Mark44928/NoNameOS/raw/main/Screenshot_20260524-150528~2.png)

# NoNameOS
Pure C++ Hobbyist OS simulation.

### Warning: NoNameOS is **NOT** an OS. It is a C++ OS simulation.

## What's New in v0.4.0

- **New commands:** `pwd`, `whoami`, `date`, `history`, `grep`, `find`, `cfetch`, `touch`, `nano`, `calc`
- **New games:** `guess` (Guess the Number), `trivia` (Trivia Quiz), `adventure` (Text Adventure)
- **Enhanced `ls -l`** with file sizes and timestamps
- **Improved VFS** with timestamps and file size tracking
- **Command history** (use `history` to view)

## 📥 Getting the Code

First things first, you need to pull the source code onto your machine and move into the directory.

```bash
git clone [https://github.com/Mark44928/NoNameOS.git](https://github.com/Mark44928/NoNameOS.git)
cd NoNameOS
```

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

