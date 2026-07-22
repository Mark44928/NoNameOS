<p align="center">
  <img src="https://github.com/Mark44928/NoNameOS/raw/main/Screenshot_20260524-150528~2.png" width="500" alt="NoNameOS Screenshot"/>
</p>

<h1 align="center">NoNameOS</h1>

> **Note:** This screenshot is from v0.3.1. The latest version (v0.7.0) has more commands, games, and tools not shown here.

<p align="center">
  <b>A pure C++ hobbyist operating-system simulation featuring an interactive shell, virtual filesystem with metadata, 13+ built-in games, and 75+ developer tools — all contained in a single source file. No external dependencies. No framework. Just compile and run.</b>
</p>

<p align="center">
  <a href="https://github.com/Mark44928/NoNameOS/forks"><img src="https://img.shields.io/github/forks/Mark44928/NoNameOS?style=social" alt="Forks"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/stargazers"><img src="https://img.shields.io/github/stars/Mark44928/NoNameOS?style=social" alt="Stars"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/blob/main/LICENSE"><img src="https://img.shields.io/github/license/Mark44928/NoNameOS?color=blue" alt="License"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/issues"><img src="https://img.shields.io/github/issues/Mark44928/NoNameOS" alt="Issues"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/releases"><img src="https://img.shields.io/github/v/release/Mark44928/NoNameOS" alt="Release"/></a>
  <img src="https://img.shields.io/badge/version-0.7.0-green" alt="Version"/>
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
- [What's New in v0.7.0](#whats-new-in-v070)
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
| **Interactive Shell** | Color-coded prompt with command duration, 75+ commands, alias support, command history, man pages, fuzzy command suggestions |
| **Virtual Filesystem** | Create, read, copy, move, delete, symlink files and directories with sizes, timestamps & permissions; trash/recycle |
| **Games (13+ built-in)** | AsciiDash, Snake, Minesweeper, Tic-Tac-Toe, Hangman, RPS, Guess, Trivia, Adventure RPG, 2048, Wordle, Typing Test, Reaction Test, Number Memory |
| **System Tools** | Text editor (`nano`), calculator (`calc`/`bc`), system info (`cfetch`), `free`, `dmesg`, `lscpu`, `ps`, `top`, `uname` |
| **Utilities** | `grep`, `find`, `locate`, `sort`, `wc`, `head`, `tail`, `rev`, `tr`, `cut`, `uniq`, `nl`, `fold`, `cowsay`, `fortune`, `lolcat`, `sl`, and more |
| **Productivity** | `todo`, `notes`, `stopwatch`, `timer`, `pom` (pomodoro), `alarm`, colored `help <cmd>` descriptions |

---

## What's New in v0.7.0

- **5 new games:** 2048, Wordle (coming soon), Typing Test (WPM), Reaction Time Test, Number Memory
- **20+ new commands:** `tree`, `watch`, `ping`, `top`, `df`, `seq`, `free`, `dmesg`, `lscpu`, `lsusb`, `arch`, `nproc`, `rev`, `tr`, `cut`, `paste`, `uniq`, `nl`, `fold`, `basename`, `dirname`, `ln -s`, `du`, `locate`, `trash`, `pom`, `alarm`, `bc`, `lolcat`, `sl`/`train`, `stopwatch`, `timer`, `who`, `useradd`, `userdel`
- **Productivity tools:** `todo` (task list), `notes` (VFS notes), `pom` (pomodoro timer), `alarm`, `stopwatch`, `timer`
- **Scientific calc** — `sin()`, `cos()`, `tan()`, `sqrt()`, `log()`, `pow()`, `%` modulo, `^` power
- **Symlinks** via `ln -s` with follow resolution
- **Trash/recycle** — `rm` moves to `/trash/` instead of deleting; `trash list`/`trash empty`
- **VFS permissions** — `chmod` now writes real mode strings, `ls -l` shows dynamic perms
- **Color-coded prompt** — green user, white host, blue directory; shows last command duration
- **Polished boot** — progress bars during startup
- **Fuzzy command suggestions** — "Did you mean `cal`?" on typos like `clac`
- **`help <cmd>`** — one-line description per command
- **Man pages** for all 75+ commands via `man <cmd>`
- **RAII TerminalGuard** — safe terminal state restoration

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
| `ls -l` | `ls -l` | Long listing with sizes, timestamps, and permissions |
| `cd` | `cd <dir>` | Change directory (`..` for parent, `/` for root) |
| `mkdir` | `mkdir <name>` | Create a new directory (with intermediate parents) |
| `touch` | `touch <file>` | Create an empty file |
| `cat` | `cat <file>` | Display file contents |
| `echo` | `echo <file> <content>` | Write content to a file |
| `rm` | `rm <name>` | Move to trash (recycle bin, use `trash` to manage) |
| `rm -r` | `rm -r <dir>` | Recursively trash a directory |
| `pwd` | `pwd` | Print current working directory |
| `grep` | `grep <pattern> <file>` | Search for a pattern in a file |
| `find` | `find <name>` | Find files by name |
| `locate` | `locate <pattern>` | Search VFS for paths matching pattern |
| `cp` | `cp <source> <dest>` | Copy a file |
| `cp -r` | `cp -r <src> <dst>` | Recursively copy a directory |
| `mv` | `mv <source> <dest>` | Move or rename a file/directory |
| `ln -s` | `ln -s <target> <link>` | Create a symbolic link |
| `du` | `du [dir]` | Show disk usage of current or specified directory |
| `tree` | `tree [dir]` | Display directory tree with indentation |
| `trash` | `trash list\|empty` | List or empty the trash |
| `chmod` | `chmod <mode> <file>` | Change file permissions in VFS |
| `head` | `head <file>` | Display first 10 lines of a file |
| `tail` | `tail <file>` | Display last 10 lines of a file |
| `sort` | `sort <file>` | Sort lines of a file alphabetically |
| `wc` | `wc <file>` | Count lines, words, and characters |
| `tee` | `tee <file> <text>` | Write to file and display on stdout |

### System

| Command | Usage | Description |
|---------|-------|-------------|
| `whoami` | `whoami` | Show current user |
| `who` | `who` | Show logged-in users |
| `date` | `date` | Show current date and time |
| `uptime` | `uptime` | Show system uptime and load |
| `hostname` | `hostname` | Print system hostname |
| `uname` | `uname [-a\|-r\|-s\|-m]` | Print system information |
| `arch` | `arch` | Print machine architecture |
| `nproc` | `nproc` | Print number of CPUs |
| `free` | `free` | Show simulated memory usage |
| `dmesg` | `dmesg` | Print boot messages |
| `lscpu` | `lscpu` | Show CPU information |
| `lsusb` | `lsusb` | List USB devices |
| `ps` | `ps` | List running processes |
| `top` | `top` | Show process snapshot |
| `env` | `env` | Show environment variables |
| `printenv` | `printenv` | Print environment variables |
| `history` | `history` | Show command history |
| `seq` | `seq <end>` | Print sequence of numbers |
| `ping` | `ping [host]` | Simulated ping |
| `watch` | `watch <cmd>` | Run a command repeatedly |
| `df` | `df` | Show VFS disk usage |
| `cfetch` | `cfetch` | Display system info (like neofetch) |
| `clear` | `clear` | Clear the screen |
| `help` | `help [cmd]` | Show all commands or describe a command |
| `exit` | `exit` | Exit NoNameOS |

### Tools

| Command | Usage | Description |
|---------|-------|-------------|
| `nano` | `nano <file>` | Built-in line-by-line text editor |
| `calc` | `calc <expr>` | Calculator (+ - * / sin/cos/tan/sqrt/log/pow) |
| `bc` | `bc <expr>` | Better calculator (+ - * / % ^) |
| `cowsay` | `cowsay [msg]` | ASCII cow with a speech bubble |
| `man` | `man <command>` | Display manual pages for any command |
| `cal` | `cal [-e\|-a\|-r]` | Calendar with event support |
| `rainbow` | `rainbow [msg]` | Print text in rainbow colors |
| `lolcat` | `lolcat [text]` | Rainbow gradient text |
| `yes` | `yes [text]` | Print text repeatedly |
| `sleep` | `sleep <sec>` | Pause for N seconds (max 30) |
| `which` | `which <command>` | Locate a command |
| `alias` | `alias [name=cmd]` | Show or create aliases |
| `unalias` | `unalias <name>` | Remove an alias |
| `su` | `su [user]` | Switch user (root/user/guest/admin) |
| `useradd` | `useradd <name>` | Add a new user |
| `userdel` | `userdel <name>` | Remove a user |
| `users` | `users` | Show logged-in users |
| `banner` | `banner [msg]` | Display colored ASCII banner |
| `fortune` | `fortune` | Random programming quote |
| `factor` | `factor <n>` | Prime factorization of a number |
| `shuf` | `shuf <text>` | Randomly shuffle text characters |
| `sl` | `sl` | Steam locomotive animation |
| `train` | `train` | Same as `sl` |

### Text Processing

| Command | Usage | Description |
|---------|-------|-------------|
| `rev` | `rev <file>` | Reverse each line of a file |
| `tr` | `tr <file> <f> <r>` | Replace character `<f>` with `<r>` in file |
| `cut` | `cut <file> <n>` | Extract first N characters of each line |
| `paste` | `paste <f1> <f2>` | Merge two files line by line |
| `uniq` | `uniq <file>` | Remove consecutive duplicate lines |
| `nl` | `nl <file>` | Number lines of a file |
| `fold` | `fold <file> [n]` | Wrap lines at N characters (default 80) |
| `basename` | `basename <path>` | Strip directory from path |
| `dirname` | `dirname <path>` | Extract directory from path |

### Productivity

| Command | Usage | Description |
|---------|-------|-------------|
| `todo` | `todo add/list/done/clear` | Task list manager (persistent in VFS) |
| `notes` | `notes <name>\|list\|rm` | Note editor (persistent in VFS) |
| `stopwatch` | `stopwatch` | Press Enter to start/stop/show elapsed time |
| `timer` | `timer <sec>` | Countdown timer with visual feedback |
| `pom` | `pom` | Pomodoro timer (25min focus / 5min break) |
| `alarm` | `alarm <sec>` | Set an alarm that counts down |

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

### 2048 (`2048`)
Slide numbered tiles with WASD to merge them. Reach the 2048 tile to win.
```bash
2048
```

### Typing Test (`typing`)
Test your typing speed. Type the shown sentence as fast as you can and get your WPM and accuracy.
```bash
typing
```

### Reaction Time (`reaction`)
Test your reflexes. Press any key when the screen flashes green. 3 rounds, average displayed.
```bash
reaction
```

### Number Memory (`nummem`)
Memorize a growing sequence of digits and type it back. How many can you remember?
```bash
nummem
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
- Add more games (Tetris, Pong, Sudoku, etc.)
- Add pipe support (`cmd1 | cmd2`)
- Add a package manager simulation
- Add persistent state across sessions
- Improve the AsciiDash engine with graphics

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
