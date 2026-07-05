<p align="center">
  <img src="https://github.com/Mark44928/NoNameOS/raw/main/Screenshot_20260524-150528~2.png" width="500" alt="NoNameOS Screenshot"/>
</p>

> **Note:** This screenshot is from v0.3.1. The latest version (v0.4.0) has more commands, games, and tools not shown here.

<h1 align="center">NoNameOS</h1>

<p align="center">
  <b>A pure C++ hobbyist OS simulation with a virtual filesystem, shell, games, and more.</b>
</p>

<p align="center">
  <a href="https://github.com/Mark44928/NoNameOS/blob/main/LICENSE"><img src="https://img.shields.io/github/license/Mark44928/NoNameOS?color=blue" alt="License"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/stargazers"><img src="https://img.shields.io/github/stars/Mark44928/NoNameOS?style=social" alt="Stars"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/issues"><img src="https://img.shields.io/github/issues/Mark44928/NoNameOS" alt="Issues"/></a>
  <img src="https://img.shields.io/badge/version-0.4.0-green" alt="Version"/>
  <img src="https://img.shields.io/badge/language-C%2B%2B-blue" alt="Language"/>
</p>

---

> **Note:** NoNameOS is **not** a real operating system. It is a C++ simulation that mimics an OS environment with a shell, virtual filesystem, games, and built-in tools.

---

## Table of Contents

- [Features](#features)
- [What's New in v0.4.0](#whats-new-in-v040)
- [Quick Start](#quick-start)
- [Build Instructions](#build-instructions)
- [Command Reference](#command-reference)
- [Games](#games)
- [Custom Maps](#custom-maps)
- [Platform Requirements](#platform-requirements)
- [Contributing](#contributing)
- [License](#license)

---

## Features

| Category | What You Get |
|----------|-------------|
| **Shell** | Interactive command prompt with 25+ commands |
| **Virtual Filesystem** | Create, read, delete files and directories with sizes & timestamps |
| **Games** | AsciiDash runner, Guess the Number, Trivia Quiz, Text Adventure |
| **System Tools** | Text editor (`nano`), calculator (`calc`), system fetch (`cfetch`) |
| **Utilities** | Command history, `grep` search, `cowsay`, `find`, and more |

---

## What's New in v0.4.0

- **9 new commands:** `pwd`, `whoami`, `date`, `history`, `grep`, `find`, `cfetch`, `touch`, `nano`, `calc`, `cowsay`
- **3 new games:** `guess` (Guess the Number), `trivia` (Trivia Quiz), `adventure` (Text RPG)
- **Enhanced `ls -l`** with file sizes and creation timestamps
- **Improved VFS** tracking file metadata
- **Command history** -- use `history` to view past commands

---

## Quick Start

```bash
git clone https://github.com/Mark44928/NoNameOS.git
cd NoNameOS
g++ -O3 NoNameOS.cpp -o nonameos
./nonameos
```

---

## Build Instructions

NoNameOS is written in pure C++ and uses POSIX headers. It compiles on any Linux-based system with a C++ compiler.

### Android (Termux)
```bash
pkg install clang
clang++ -O3 NoNameOS.cpp -o nonameos
./nonameos
```

### Debian / Ubuntu
```bash
sudo apt update && sudo apt install g++
g++ -O3 NoNameOS.cpp -o nonameos
./nonameos
```

### Fedora
```bash
sudo dnf install gcc-c++
g++ -O3 NoNameOS.cpp -o nonameos
./nonameos
```

### Arch Linux
```bash
sudo pacman -S gcc
g++ -O3 NoNameOS.cpp -o nonameos
./nonameos
```

---

## Command Reference

### Filesystem

| Command | Usage | Description |
|---------|-------|-------------|
| `ls` | `ls` | List files in current directory |
| `ls -l` | `ls -l` | Long listing with sizes and timestamps |
| `cd` | `cd <dir>` | Change directory (`..` for parent, `/` for root) |
| `mkdir` | `mkdir <name>` | Create a new directory |
| `touch` | `touch <file>` | Create an empty file |
| `cat` | `cat <file>` | Display file contents |
| `echo` | `echo <file> <content>` | Write content to a file |
| `rm` | `rm <name>` | Remove a file or directory |
| `pwd` | `pwd` | Print current working directory |
| `grep` | `grep <pattern> <file>` | Search for a pattern in a file |
| `find` | `find <name>` | Find files by name |

### System

| Command | Usage | Description |
|---------|-------|-------------|
| `whoami` | `whoami` | Show current user |
| `date` | `date` | Show current date and time |
| `history` | `history` | Show command history |
| `cfetch` | `cfetch` | Display system info (like neofetch) |
| `clear` | `clear` | Clear the screen |
| `help` | `help` | Show all available commands |
| `exit` | `exit` | Exit NoNameOS |

### Tools

| Command | Usage | Description |
|---------|-------|-------------|
| `nano` | `nano <file>` | Built-in line-by-line text editor |
| `calc` | `calc <expr>` | Calculator (e.g. `calc 2+3*4`, supports `+` `-` `*` `/`) |
| `cowsay` | `cowsay [msg]` | ASCII cow with a speech bubble |

---

## Games

### AsciiDash (`play`)
A side-scrolling obstacle runner. Jump over `^` obstacles with the **SPACE** key.
```bash
play                # Run default map "Stereo Madness"
play jumper.gmd     # Run a custom map from the VFS
```

### Guess the Number (`guess`)
Guess a random number between 1 and 100. You get unlimited attempts.
```bash
guess
```

### Trivia Quiz (`trivia`)
Answer 5 multiple-choice questions about computers and technology.
```bash
trivia
```

### Text Adventure (`adventure`)
A dungeon RPG with HP and gold. Explore left or right, find treasure, fight monsters, and survive.
```bash
adventure
```

---

## Custom Maps

Create your own AsciiDash maps using `^` for obstacles and `_` for flat ground:

```bash
mkdir /geometry
echo /geometry/mymap.gmd ____^^____^^^^____^___^
play mymap.gmd
```

The default map `jumper.gmd` is pre-loaded in `/geometry/`.

---

## Platform Requirements

- **Linux** (Termux on Android, Debian, Ubuntu, Fedora, Arch, etc.)
- A C++ compiler (`g++` or `clang++`)
- POSIX-compatible system (uses `<termios.h>`, `<unistd.h>`, `<fcntl.h>`)
- **Windows is not supported** due to POSIX-only dependencies

---

## Contributing

Contributions are welcome! Here's how:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Commit your changes (`git commit -m "Add my feature"`)
4. Push to the branch (`git push origin feature/my-feature`)
5. Open a Pull Request

Ideas for contributions:
- Add more games (snake, minesweeper, etc.)
- Add more built-in tools (text-based paint, calendar, etc.)
- Improve the AsciiDash engine
- Add color themes
- Add a `man` command with detailed docs

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

---

## You Might Also Like

- [Termux TUI Package Store](https://github.com/Mark44928/Termux-TUI-Package-Store) - Interactive fzf-powered package browser for Termux
- [Anti-Bloatware List](https://github.com/Mark44928/Anti-bloatware-list-for-Android-TV-Boxes-and-Sticks-for-rooted) - Debloat rooted Android TV sticks

---

## Star History

<a href="https://star-history.com/#Mark44928/NoNameOS&Date">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=Mark44928/NoNameOS&type=Date&theme=dark" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=Mark44928/NoNameOS&type=Date" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=Mark44928/NoNameOS&type=Date" />
 </picture>
</a>

---

<p align="center">
  If you like NoNameOS, give it a star! It helps others discover the project.
</p>
