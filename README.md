<p align="center">
  <img src="https://github.com/Mark44928/NoNameOS/raw/main/Screenshot_20260524-150528~2.png" width="500" alt="NoNameOS Screenshot"/>
</p>

<h1 align="center">NoNameOS</h1>

> **Note:** This screenshot is from v0.3.1. The latest version (v0.6.0) has more commands, games, and tools not shown here.

<p align="center">
  <b>A pure C++ hobbyist operating-system simulation featuring an interactive shell, virtual filesystem with metadata, 9 built-in games, and developer tools — all contained in a single source file. No external dependencies. No framework. Just compile and run.</b>
</p>

<p align="center">
  <a href="https://github.com/Mark44928/NoNameOS/forks"><img src="https://img.shields.io/github/forks/Mark44928/NoNameOS?style=social" alt="Forks"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/stargazers"><img src="https://img.shields.io/github/stars/Mark44928/NoNameOS?style=social" alt="Stars"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/blob/main/LICENSE"><img src="https://img.shields.io/github/license/Mark44928/NoNameOS?color=blue" alt="License"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/issues"><img src="https://img.shields.io/github/issues/Mark44928/NoNameOS" alt="Issues"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/releases"><img src="https://img.shields.io/github/v/release/Mark44928/NoNameOS" alt="Release"/></a>
  <img src="https://img.shields.io/badge/version-0.6.0-green" alt="Version"/>
  <img src="https://img.shields.io/badge/language-C%2B%2B-blue" alt="Language"/>
  <img src="https://img.shields.io/github/languages/top/Mark44928/NoNameOS" alt="Top Language"/>
  <img src="https://img.shields.io/github/repo-size/Mark44928/NoNameOS" alt="Repo Size"/>
  <a href="https://github.com/Mark44928/NoNameOS/pulse"><img src="https://img.shields.io/github/commit-activity/m/Mark44928/NoNameOS" alt="Commit Activity"/></a>
</p>

---

> **Note:** NoNameOS is **not** a real operating system. It is a C++ simulation that mimics an OS environment with a shell, virtual filesystem, games, and built-in tools.

---

## Table of Contents

- [Features](#features)
- [What's New in v0.6.0](#whats-new-in-v060)
- [Quick Start](#quick-start)
- [Build Instructions](#build-instructions)
- [Command Reference](#command-reference)
  - [Filesystem Commands](#filesystem-commands)
  - [System Commands](#system-commands)
  - [Tools](#tools)
- [Games](#games)
- [Custom Maps](#custom-maps)
- [Platform Requirements](#platform-requirements)
- [Contributing](#contributing)
- [License](#license)

---

## Features

| Category | What You Get |
|----------|-------------|
| **Interactive Shell** | Command prompt with 55+ commands, alias support, command history, and man pages |
| **Virtual Filesystem** | Create, read, copy, move, delete files and directories with sizes & timestamps |
| **Games (9 built-in)** | AsciiDash, Snake, Minesweeper, Tic-Tac-Toe, Hangman, RPS, Guess, Trivia, Adventure RPG |
| **System Tools** | Text editor (`nano`), calculator (`calc`), system info (`cfetch`), `ps`, `uname` |
| **Utilities** | `grep`, `find`, `sort`, `wc`, `head`, `tail`, `cowsay`, `fortune`, `banner`, and more |

---

## What's New in v0.6.0

- **3 new games:** Tic-Tac-Toe (with minimax AI), Hangman, Rock Paper Scissors (best-of-7)
- **18 new commands:** `cp`, `mv`, `head`, `tail`, `sort`, `wc`, `tee`, `yes`, `env`, `hostname`, `sleep`, `which`, `alias`/`unalias`, `users`, `banner`, `fortune`, `factor`, `shuf`, `chmod`, `su`
- **Man pages** for all 55+ commands via `man <cmd>`
- **Working alias system** — create custom shortcuts with `alias name='command'`
- **Updated boot**, help, and cfetch to v0.6.0

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
| `cp` | `cp <source> <dest>` | Copy a file |
| `mv` | `mv <source> <dest>` | Move or rename a file/directory |
| `head` | `head <file>` | Display first 10 lines of a file |
| `tail` | `tail <file>` | Display last 10 lines of a file |
| `sort` | `sort <file>` | Sort lines of a file alphabetically |
| `wc` | `wc <file>` | Count lines, words, and characters |
| `tee` | `tee <file> <text>` | Write to file and display on stdout |

### System

| Command | Usage | Description |
|---------|-------|-------------|
| `whoami` | `whoami` | Show current user |
| `date` | `date` | Show current date and time |
| `uptime` | `uptime` | Show system uptime and load |
| `hostname` | `hostname` | Print system hostname |
| `uname` | `uname [-a\|-r\|-s\|-m]` | Print system information |
| `ps` | `ps` | List running processes |
| `env` | `env` | Show environment variables |
| `history` | `history` | Show command history |
| `cfetch` | `cfetch` | Display system info (like neofetch) |
| `clear` | `clear` | Clear the screen |
| `help` | `help` | Show all available commands |
| `exit` | `exit` | Exit NoNameOS |

### Tools

| Command | Usage | Description |
|---------|-------|-------------|
| `nano` | `nano <file>` | Built-in line-by-line text editor |
| `calc` | `calc <expr>` | Calculator (`+` `-` `*` `/` with precedence) |
| `cowsay` | `cowsay [msg]` | ASCII cow with a speech bubble |
| `man` | `man <command>` | Display manual pages for any command |
| `cal` | `cal` | Display current month's calendar |
| `rainbow` | `rainbow [msg]` | Print text in rainbow colors |
| `yes` | `yes [text]` | Print text repeatedly |
| `sleep` | `sleep <sec>` | Pause for N seconds (max 30) |
| `which` | `which <command>` | Locate a command |
| `alias` | `alias [name=cmd]` | Show or create aliases |
| `unalias` | `unalias <name>` | Remove an alias |
| `su` | `su <user>` | Switch user (root/user) |
| `chmod` | `chmod` | Show VFS permission info |
| `users` | `users` | Show logged-in users |
| `banner` | `banner [msg]` | Display colored ASCII banner |
| `fortune` | `fortune` | Random programming quote |
| `factor` | `factor <n>` | Prime factorization of a number |
| `shuf` | `shuf <text>` | Randomly shuffle text characters |

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

### Snake (`snake`)
Terminal Snake game. Use WASD to move, eat food (`*`) to grow. Don't hit walls or yourself.
```bash
snake
```

### Minesweeper (`minesweeper`)
Classic Minesweeper on a 10x10 grid with 12 mines. Enter `x y` to reveal, `f x y` to flag.
```bash
minesweeper
```

### Tic-Tac-Toe (`tictactoe` / `ttt`)
Play against an AI opponent that uses minimax strategy. You are X, AI is O.
```bash
tictactoe
```

### Hangman (`hangman`)
Guess letters to reveal a hidden word before the stick figure is complete.
```bash
hangman
```

### Rock Paper Scissors (`rps`)
Best-of-7 series against the AI. First to 4 wins.
```bash
rps
```

---

## Custom Maps

Create your own AsciiDash maps using `^` for obstacles and `_` for flat ground:

```bash
cd /geometry
echo mymap.gmd ____^^____^^^^____^___^
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

## Perfect For

- **C++ learners** exploring OS concepts without kernel-level complexity
- **Students** studying filesystems, shells, and process simulation
- **Hackers** wanting a terminal sandbox with games and tools
- **Termux users** looking for a fun single-file project to compile and run

---

## Contributing

Contributions are welcome! Here's how:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Commit your changes (`git commit -m "Add my feature"`)
4. Push to the branch (`git push origin feature/my-feature`)
5. Open a Pull Request

Ideas for contributions:
- Add more games (2048, Tetris, Pong, etc.)
- Add more built-in tools (text-based paint, cron, etc.)
- Improve the AsciiDash engine with graphics
- Add a package manager simulation
- Add pipe support (`cmd1 | cmd2`)

---

## License

This project is licensed under the GNU General Public License v3. See the [LICENSE](LICENSE) file for details.

---

## You Might Also Like

- [Termux TUI Package Store](https://github.com/Mark44928/Termux-TUI-Package-Store) - Interactive fzf-powered package browser for Termux
- [Anti-Bloatware List](https://github.com/Mark44928/Anti-bloatware-list-for-Android-TV-Boxes-and-Sticks-for-rooted) - Debloat rooted Android TV sticks

---

<p align="center">
  If you like NoNameOS, give it a star! It helps others discover the project.
</p>
