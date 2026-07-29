<p align="center">
  <img src="https://github.com/Mark44928/NoNameOS/raw/main/Screenshot_20260524-150528~2.png" width="500" alt="NoNameOS Screenshot"/>
</p>

<p align="center"><sub><i>Screenshot from v0.3.1 — current version (v1.0.2) has 24+ games, 149+ commands, and full color support not shown here.</i></sub></p>

<h1 align="center">NoNameOS</h1>

<p align="center">
  <img src="https://img.shields.io/badge/version-v1.0.2-green?style=for-the-badge" alt="Version"/>
  <img src="https://img.shields.io/badge/language-C%2B%2B17-blue?style=for-the-badge" alt="C++17"/>
  <img src="https://img.shields.io/badge/license-GPLv3-purple?style=for-the-badge" alt="License"/>
  <img src="https://img.shields.io/badge/4928-lines_of_code-orange?style=for-the-badge" alt="Lines"/>
  <img src="https://img.shields.io/badge/24%2B_games-red?style=for-the-badge" alt="Games"/>
  <img src="https://img.shields.io/badge/149%2B_commands-cyan?style=for-the-badge" alt="Commands"/>
  <img src="https://img.shields.io/badge/49_bugs_fixed-2ea44f?style=for-the-badge" alt="Bugs Fixed"/>
  <img src="https://img.shields.io/badge/dependencies-zero-brightgreen?style=for-the-badge" alt="Zero Dependencies"/>
  <img src="https://img.shields.io/badge/platform-linux-lightgrey?style=for-the-badge" alt="Linux"/>
  <img src="https://img.shields.io/github/languages/code-size/Mark44928/NoNameOS?style=for-the-badge" alt="Code Size"/>
  <img src="https://img.shields.io/github/last-commit/Mark44928/NoNameOS?style=for-the-badge" alt="Last Commit"/>
</p>

<blockquote align="center">
  <b>One file. Zero dependencies. 24 games. 149 commands. One terminal.</b>
  <br/><sub><i>Simulated OS meets sandbox playground — all in a single .cpp</i></sub>
</blockquote>

<p align="center">
  <b>A pure C++ terminal operating-system simulation featuring an interactive shell, virtual filesystem with metadata, 24+ built-in games, and 149+ developer tools — all contained in a single source file.</b><br/>
  <sub>No external dependencies. No framework. No build system headaches. Just compile and run.</sub>
</p>

<p align="center">
  <a href="https://github.com/Mark44928/NoNameOS/forks"><img src="https://img.shields.io/github/forks/Mark44928/NoNameOS?style=social" alt="Forks"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/stargazers"><img src="https://img.shields.io/github/stars/Mark44928/NoNameOS?style=social" alt="Stars"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/releases"><img src="https://img.shields.io/github/v/release/Mark44928/NoNameOS" alt="Release"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/issues"><img src="https://img.shields.io/github/issues/Mark44928/NoNameOS" alt="Issues"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/pulse"><img src="https://img.shields.io/github/commit-activity/m/Mark44928/NoNameOS" alt="Commit Activity"/></a>
</p>

---

> **Note:** NoNameOS is **not** a real operating system. It is a C++ simulation that mimics an OS environment with a shell, virtual filesystem, games, and built-in tools.

---

## Why NoNameOS?

| | |
|---|---|
| **Single File** | Entire project is one `.cpp` — no build system, no `#include` hell, no headaches |
| **Fun to Use** | 24+ games, easter eggs, colored output, animated boot — it's genuinely enjoyable |
| **Learn C++** | See real-world STL, OOP, RAII, ANSI escape codes, and POSIX I/O in action |
| **Hackable** | Every feature in one file — read it, mod it, break it, fix it |
| **Portable** | Compiles everywhere — Linux, Termux, WSL, even your toaster if it runs g++ |

## Perfect For

- **C++ learners** wanting to see real OS concepts without kernel-level complexity
- **Students** exploring filesystems, shells, and process simulation hands-on
- **Hackers** looking for a terminal sandbox packed with games and tools
- **Termux users** craving a fun single-file project to compile in under 10 seconds

## Table of Contents

- [Why NoNameOS?](#why-nonameos)
- [Perfect For](#perfect-for)
- [Features](#features)
- [What's New in v1.0.2](#whats-new-in-v102)
- [Quick Start](#quick-start)
- [Keyboard Shortcuts](#keyboard-shortcuts)
- [Build Instructions](#build-instructions)
- [Command Reference](#command-reference)
- [Games](#games)
- [Easter Eggs](#easter-eggs)
- [Custom Maps](#custom-maps)
- [Architecture](#architecture)
- [What You'll Learn](#what-youll-learn)
- [FAQ](#faq)
- [Platform Requirements](#platform-requirements)
- [Tech Stack](#tech-stack)
- [Fun Facts](#fun-facts)
- [Version History](#version-history)
- [Contributing](#contributing)
- [License](#license)

---

## Features

| Category | What You Get | Highlights |
|----------|-------------|------------|
| **Interactive Shell** | Color-coded prompt, command duration, 149+ commands | Arrow keys, history, fuzzy suggestions |
| **Virtual Filesystem** | Files, directories, symlinks, permissions, trash | Timestamps, sizes, `ls -l`, `chmod` |
| **24+ Games** | Arcade, puzzle, strategy, word, reflex games | Tetris, Pong, Sudoku, Wordle, Flappy Bird |
| **System Tools** | Text editor, calculators, system info | `nano`, `calc`, `cfetch`, `free`, `top` |
| **Text Processing** | grep, find, sort, word count, diff | `rev`, `tr`, `cut`, `uniq`, `nl`, `fold` |
| **Converters** | Base64, ROT13, Morse, Roman, Binary | `urlencode`, `hexdump`, `hash` |
| **Math & Stats** | Calculators, BMI, tip, statistics | `calc2`, `bmi`, `stats`, `dice`, `coin` |
| **Productivity** | Todo list, notes, timers, password gen | `pom`, `countdown`, `worldclock` |
| **Visual Effects** | Matrix rain, rainbow text, color themes | `matrix`, `lolcat`, `colors`, `disco` |
| **Fun & Easter Eggs** | 25+ hidden commands | `sudo`, `rickroll`, `konami`, `hack` |

---

## What's New in v1.0.2

### Bug Fixes (43 total across all audits)
- Fixed 2 critical Sudoku infinite loops in initialization
- Fixed `setfill('0')` state leak across commands
- Fixed 5 dead easter eggs (moved inside primary handlers)
- Fixed hangman — now draws complete figure at game end
- Fixed Tic-Tac-toe error pause in raw mode
- Added `cin.ignore` before `cin.get()` in all 21 prompts
- Added quit mechanism to guess game
- Fixed 2048 duplicate win message
- Whack-a-mole '0' now triggers miss as documented
- World clock uses UTC offset math (no global `TZ` modification)
- Tetris J-piece now has 4 rotations
- kbhit/getkey use static peek buffer (race-free)
- CSV separator width fixed

### New in v1.0.1
- Global arrow key support in all 7 real-time games
- Shell readline with cursor movement, backspace, history navigation
- 25+ easter eggs (`sudo`, `konami`, `rickroll`, `disco`, `matrix`, etc.)

### New in v1.0.0
- **11 new games:** Tetris, Pong, Sudoku, Flappy Bird, Memory Cards, Connect Four, Lights Out, Sliding Puzzle, Breakout, Whack-a-Mole, Wordle, Quiz
- **60+ new commands:** Converters, math tools, text processing, fun utilities
- **Enhanced visuals:** 256-color ANSI, truecolor RGB, animated boot, styled prompts

---

## Quick Start

```bash
git clone https://github.com/Mark44928/NoNameOS.git && cd NoNameOS && g++ -O3 NoNameOS.cpp -o nonameos -lpthread && ./nonameos
```

> **One command. One file. One terminal. Pure C++ fun.**

Or use `make`:
```bash
git clone https://github.com/Mark44928/NoNameOS.git && cd NoNameOS && make && ./nonameos
```

---



## Keyboard Shortcuts

### Shell
| Key | Action |
|-----|--------|
| `↑` / `↓` | Navigate command history |
| `←` / `→` | Move cursor left/right |
| `Backspace` | Delete character at cursor |
| `Enter` | Execute command |

### Games
| Key | Action |
|-----|--------|
| `WASD` / `Arrow Keys` | Move (snake, tetris, pong, breakout) |
| `SPACE` / `W` / `↑` | Jump (flappy, asciidash) |
| `Q` | Quit any game |
| `1-9` | Whack-a-mole input |
| `Enter` | Confirm / continue |

---

## Build Instructions

NoNameOS is written in pure C++ and uses POSIX headers. It compiles on any Linux-based system with a C++ compiler.

### Android (Termux)
```bash
pkg install clang
clang++ -O3 NoNameOS.cpp -o nonameos -lpthread
./nonameos
```

### Debian / Ubuntu
```bash
sudo apt update && sudo apt install g++
g++ -O3 NoNameOS.cpp -o nonameos -lpthread
./nonameos
```

### Fedora
```bash
sudo dnf install gcc-c++
g++ -O3 NoNameOS.cpp -o nonameos -lpthread
./nonameos
```

### Arch Linux
```bash
sudo pacman -S gcc
g++ -O3 NoNameOS.cpp -o nonameos -lpthread
./nonameos
```

---

## Command Reference

<details>
<summary><b>📁 Filesystem (24 commands)</b></summary>

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

</details>

<details>
<summary><b>⚙️ System (25 commands)</b></summary>

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

</details>

<details>
<summary><b>🛠️ Tools (24 commands)</b></summary>

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

</details>

<details>
<summary><b>📝 Text Processing (9 commands)</b></summary>

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

</details>

<details>
<summary><b>📋 Productivity (9 commands)</b></summary>

| Command | Usage | Description |
|---------|-------|-------------|
| `todo` | `todo add/list/done/clear` | Task list manager (persistent in VFS) |
| `notes` | `notes <name>\|list\|rm` | Note editor (persistent in VFS) |
| `stopwatch` | `stopwatch` | Press Enter to start/stop/show elapsed time |
| `timer` | `timer <sec>` | Countdown timer with visual feedback |
| `pom` | `pom` | Pomodoro timer (25min focus / 5min break) |
| `alarm` | `alarm <sec>` | Set an alarm that counts down |
| `countdown` | `countdown <sec>` | Countdown with progress bar (1-600s) |
| `password` | `password` | Generate secure random password (12-20 chars) |
| `worldclock` | `worldclock` | Show times across 6 time zones |

</details>

<details>
<summary><b>🔄 Converters & Encoding (10 commands)</b></summary>

| Command | Usage | Description |
|---------|-------|-------------|
| `base64` | `base64 <file\|text>` | Base64 encode text or file |
| `rot13` | `rot13 <text>` | ROT13 text rotation |
| `morse` | `morse <text>` | Convert text to Morse code |
| `roman` | `roman <num>` | Convert number to Roman numerals (1-3999) |
| `binary` | `binary <num>` | Convert number to binary |
| `hash` | `hash <text>` | Generate DJB2 hash |
| `urlencode` | `urlencode <text>` | URL-encode text |
| `urldecode` | `urldecode <text>` | URL-decode text |
| `hexdump` | `hexdump <file\|text>` | Hex dump with ASCII sidebar |
| `encode` | `encode <text>` | Show ROT13, uppercase, and lowercase |

</details>

<details>
<summary><b>🧮 Math & Stats (11 commands)</b></summary>

| Command | Usage | Description |
|---------|-------|-------------|
| `calc2` | `calc2 <expr>` | Infix calculator with proper precedence |
| `bmi` | `bmi <weight_kg> <height_m>` | Calculate BMI |
| `tip` | `tip <bill> <percent>` | Calculate tip and total |
| `stats` | `stats <num1> <num2> ...` | Mean, median, std dev, min, max |
| `random` | `random [lo] [hi]` | Random number (default 1-100) |
| `dice` | `dice [n]` | Roll n dice (default 1) |
| `coin` | `coin` | Flip a coin |
| `scrabble` | `scrabble <word>` | Calculate Scrabble score |
| `age` | `age <year> <month> <day>` | Calculate age from birthdate |
| `datecalc` | `datecalc <y m d> +\|- <days>` | Add/subtract days from a date |
| `units` | `units <val> <from> <to>` | Convert units (km/mi, kg/lb, C/F, etc.) |

</details>

<details>
<summary><b>📊 Text Tools (8 commands)</b></summary>

| Command | Usage | Description |
|---------|-------|-------------|
| `wordcount` | `wordcount <file>` | Word/line/char count + frequency chart |
| `diff` | `diff <file1> <file2>` | Show differences between two files |
| `capitalize` | `capitalize <text>` | Capitalize first letter of each word |
| `reverse` | `reverse <text>` | Reverse a string |
| `repeat` | `repeat <n> <text>` | Repeat text n times |
| `bar` | `bar <label value> ...` | Draw a bar chart |
| `sparkline` | `sparkline <n1> <n2> ...` | Draw a sparkline |
| `csv` | `csv <file>` | Display CSV as formatted table |

</details>

<details>
<summary><b>🎨 Fun & Info (17 commands)</b></summary>

| Command | Usage | Description |
|---------|-------|-------------|
| `colors` | `colors` | Show all 16 color palette swatches |
| `cmtheme` | `cmtheme` | Show color theme options |
| `colorgen` | `colorgen` | Generate random color with RGB + hex |
| `palette` | `palette` | Generate 10-color palette from seed |
| `matrix` | `matrix [rows]` | Matrix rain animation |
| `ascii` | `ascii` | Random ASCII art |
| `emoji` | `emoji [name]` | Show emoji or list all emojis |
| `pick` | `pick a \| b \| c` | Randomly pick from options |
| `zodiac` | `zodiac <month> <day>` | Get zodiac sign |
| `quote` | `quote` | Random inspirational quote |
| `joke` | `joke` | Random programming joke |
| `weather` | `weather` | Simulated weather report |
| `ip` | `ip` | Show simulated IP addresses |
| `uptime2` | `uptime2` | Detailed uptime with progress bar |
| `mem` | `mem` | Memory usage with bar |
| `cpu` | `cpu` | Per-core CPU usage |
| `disk` | `disk` | Disk usage across mounts |

</details>

---

## See It In Action

```
$ ./nonameos

  _   __      _   __                     ____  _____
 / | / /___  / | / /___ _____ ___  ___  / __ \/ ___/
/  |/ / __ \/  |/ / __ `/ __ `__ \/ _ \/ / / /\__ \
/ /|  / /_/ / /|  / /_/ / / / / / /  __/ /_/ /___/ /
/_/ |_/\____/_/ |_|\__,_/_/ /_/ /_/\___/\____//____/

root@nonameos:/$
❯ tetris

  🧱 TETRIS  Score: 4200  (AD/←→=move W/↑=rotate S/↓=drop Q=quit)

  ┌──────────────────┐
  │          ■       │
  │        ■■■       │
  │                  │
  │ ■■■■             │
  │                  │
  └──────────────────┘
```

---

## Games

### Classic Arcade
| Game | Command | Description | Controls |
|------|---------|-------------|----------|
| **Snake** | `snake` | Eat food to grow. Don't hit walls or yourself. | WASD / Arrows |
| **Tetris** | `tetris` | Falling blocks. Clear lines to score. | AD=move W=rotate S=drop |
| **Pong** | `pong` | Classic Pong vs AI. First to 5 wins. | WS=move |
| **Breakout** | `breakout` | Break bricks with a bouncing ball. | AD=move |
| **Flappy Bird** | `flappy` | Flap through pipes. How far can you go? | SPACE=flap |

### Puzzle & Logic
| Game | Command | Description | Controls |
|------|---------|-------------|----------|
| **2048** | `2048` | Slide tiles to merge them. Reach 2048. | WASD |
| **Sudoku** | `sudoku` | 9x9 number puzzle with pre-filled clues. | r c value |
| **Minesweeper** | `minesweeper` | 10x10 grid. Find mines without blowing up. | x y to reveal |
| **Sliding Puzzle** | `puzzle` | Classic 15-puzzle. Arrange tiles in order. | WASD / Arrows |
| **Lights Out** | `lightsout` | Toggle lights on 5x5 grid. Turn all off. | r c |

### Strategy
| Game | Command | Description | Controls |
|------|---------|-------------|----------|
| **Tic-Tac-Toe** | `tictactoe` | Play against minimax AI. You are X. | 1-9 |
| **Connect Four** | `connect4` | Drop discs. First to 4 in a row wins. | 1-7 |

### Word & Knowledge
| Game | Command | Description | Controls |
|------|---------|-------------|----------|
| **Wordle** | `wordle` | Guess the 5-letter word in 6 attempts. | Type word |
| **Hangman** | `hangman` | Guess letters before the figure is complete. | Type letter |
| **Trivia** | `trivia` | 5 tech questions. Test your knowledge. | 1-4 |
| **Quiz** | `quiz` | 5 random general knowledge questions. | 1-4 |

### Reflex & Speed
| Game | Command | Description | Controls |
|------|---------|-------------|----------|
| **Whack-a-Mole** | `whack` | Whack moles as they pop up! | 1-9 |
| **Typing Test** | `typing` | Test your typing speed and accuracy. | Type |
| **Reaction Time** | `reaction` | Press key when screen flashes. 3 rounds. | Any key |

### Other
| Game | Command | Description | Controls |
|------|---------|-------------|----------|
| **AsciiDash** | `play` | Side-scrolling obstacle runner. | SPACE=jump |
| **Adventure** | `adventure` | Dungeon RPG with HP and gold. | left/right/rest |
| **Guess** | `guess` | Guess a number 1-100. | Type number |
| **Rock Paper Scissors** | `rps` | Best-of-7 vs AI. | 1-3 |
| **Number Memory** | `nummem` | Memorize growing digit sequences. | Type digits |

---

## Easter Eggs

NoNameOS has **25+ hidden easter eggs**. Try these:

<details>
<summary><b>🥚 Spoiler: Easter Eggs (click to reveal)</b></summary>

| Command | What Happens |
|---------|-------------|
| `sudo make me a sandwich` | "Okay." |
| `sudo rm -rf /` | "NUCLEAR LAUNCH DENIED" |
| `42` | The Answer to the Ultimate Question |
| `konami` | Konami Code activated! |
| `rickroll` | Never gonna give you up... |
| `hack` | Hollywood-style hacking animation |
| `disco` | Rainbow disco mode |
| `dance` | Dancing stick figure |
| `matrix -r` | "Wake up, Neo..." |
| `whoami really` | Existential crisis |
| `lenny` | ( ͡° ͜ʖ ͡°) |
| `tableflip` | (╯°□°)╯︵ ┻━┻ |
| `dealwithit` | (⌐■_■) |
| `loading` | Fake loading bar |
| `glhf` | Good luck, have fun! |
| `version -a` | Developer stats |
| `cat /dev/brain` | "You're already using it" |
| `echo hello world` | Hello, World! greeting |

</details>

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

## What You'll Learn

NoNameOS is a goldmine for C++ learners. Here's what each feature teaches:

| Feature | C++ Concept | Real-World Pattern |
|---------|-------------|-------------------|
| `TerminalGuard` | RAII (Resource Acquisition Is Initialization) | Always restore resources in destructors |
| `map<string, FSNode>` | STL associative containers | Virtual filesystems, databases, caches |
| `mt19937 rng{...}` | Modern C++ random | Cryptographic vs. pseudo-random generation |
| `namespace color` | Namespaces & constants | Avoiding global pollution, naming conventions |
| `cooked_readline()` | POSIX terminal I/O | How shells actually read input |
| Command dispatch | String parsing & routing | Interpreter design, plugin systems |
| `cp -r` iterator | Iterator safety | Avoiding undefined behavior with containers |
| `cmd_bc` shunting-yard | Algorithm implementation | Expression parsing, operator precedence |
| `has_traversal()` | Input validation | Security: path traversal attacks |
| `volatile sig_atomic_t` | Signal handling | Async-signal-safe programming |
| ANSI escape codes | Terminal rendering | How `htop`, `vim`, `tmux` draw UIs |
| `fork()`/`waitpid()` | Process simulation | Conceptual: how real OS process management works (simulated in NoNameOS) |

---

## FAQ

<details>
<summary><b>Does NoNameOS work on Windows?</b></summary>

No. NoNameOS uses POSIX headers (`<termios.h>`, `<unistd.h>`, `<fcntl.h>`) that don't exist on Windows. However, it works perfectly under **WSL** (Windows Subsystem for Linux) and **Git Bash**.

</details>

<details>
<summary><b>How do I exit?</b></summary>

Type `exit` or press `Ctrl+C`. The `TerminalGuard` RAII class will restore your terminal settings automatically.

</details>

<details>
<summary><b>Can I save data between sessions?</b></summary>

No. The virtual filesystem is entirely in-memory. When you exit, everything is gone. This is by design — it's a simulation, not a real OS.

</details>

<details>
<summary><b>How is this different from a real OS?</b></summary>

NoNameOS is a **simulation** for learning and fun. It has a shell, filesystem, and processes — but they're all implemented in userspace C++ code. There's no kernel, no hardware access, no real multitasking.

</details>

<details>
<summary><b>Can I add my own commands?</b></summary>

Yes! The entire project is one file. Find the command dispatcher (around line 3153) and add your own `else if (cmd == "mycommand")` block. Then recompile.

</details>

<details>
<summary><b>Why is it called "NoNameOS"?</b></summary>

Because naming things is hard. The author couldn't decide on a name, so... NoNameOS it is.

</details>

---

## Platform Requirements

- **Linux** (Termux on Android, Debian, Ubuntu, Fedora, Arch, etc.)
- A C++ compiler (`g++` or `clang++`)
- POSIX-compatible system (uses `<termios.h>`, `<unistd.h>`, `<fcntl.h>`)
- **Windows is not supported** due to POSIX-only dependencies

---

## Tech Stack

| Technology | Usage |
|------------|-------|
| **C++17** | Core language — STL containers, lambdas, structured bindings, optional |
| **POSIX I/O** | Terminal control via `termios`, non-blocking input via `fcntl` |
| **ANSI Escape Codes** | 256-color and truecolor (RGB) terminal rendering |
| **RAII** | `TerminalGuard` ensures terminal state is always restored |
| **Single-file Design** | Entire project in one `.cpp` file — zero build complexity |

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        NoNameOS Architecture                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────┐    ┌─────────────────┐    ┌──────────────┐    │
│  │ Terminal I/O │───▶│  Shell (REPL)   │───▶│  Dispatcher  │    │
│  │ termios.h    │    │ cooked_readline │    │  if/else map │    │
│  │ fcntl.h      │    │ history, arrows │    │  149+ cmds   │    │
│  └──────────────┘    └─────────────────┘    └───────┬──────┘    │
│                                                     │           │
│                    ┌────────────────────────────────┼─────┐     │
│                    │                                │     │     │
│           ┌────────▼───────┐              ┌─────────▼───┐ │     │
│           │  Virtual FS    │              │  24 Games   │ │     │
│           │  map<string,   │              │  Tetris     │ │     │
│           │    FSNode>     │              │  Snake      │ │     │
│           │  timestamps    │              │  Pong       │ │     │
│           │  permissions   │              │  Wordle     │ │     │
│           │  symlinks      │              │  ...        │ │     │
│           └────────────────┘              └─────────────┘ │     │
│                                                           │     │
│           ┌────────────────┐              ┌─────────────┐ │     │
│           │  System Tools  │              │   Easter    │ │     │
│           │  nano, calc    │              │   Eggs      │─┘     │
│           │  cfetch, top   │              │   25+       │       │
│           └────────────────┘              └─────────────┘       │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │  TerminalGuard (RAII) — auto-restores terminal on exit   │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

### Key Design Patterns

| Pattern | Where | Why |
|---------|-------|-----|
| **RAII** | `TerminalGuard` class | Terminal state always restored, even on exceptions |
| **STL Containers** | `map<string, FSNode>` | Virtual filesystem backed by associative containers |
| **MT19937 RNG** | `mt19937 rng{random_device{}()}` | Modern C++ random number generation |
| **ANSI Colors** | `namespace color` | 256-color and truecolor RGB terminal rendering |
| **Single Dispatch** | Line 3153+ | One massive if/else chain routes all 149+ commands |

## Fun Facts

- **Single-file C++ project** — the entire OS in one `.cpp` file
- **Zero external dependencies** — only standard library + POSIX
- **Compiles in ~10 seconds** on a modern machine
- **Runs on anything**: Linux, macOS, Termux (Android), WSL
- **43 bugs found and fixed** across 5 automated audit passes
- **25+ easter eggs** hidden throughout the codebase
- **Built with**: C++, POSIX, and questionable life choices

---

## Version History

| Version | Lines | Games | Commands | Key Changes |
|---------|-------|-------|----------|-------------|
| **v1.0.2** | 4,928 | 24 | 149 | 43 audit bug fixes |
| **v1.0.1** | 4,561 | 24 | 135 | Arrow keys, cursor movement, easter eggs |
| **v1.0.0** | 4,500 | 24 | 135 | 11 new games, 60+ commands, visual overhaul |
| **v0.7.0** | 2,500 | 13 | 75 | VFS, games, tools, alias system |
| **v0.6.0** | 1,800 | 9 | 55 | 3 games, 18 commands, man pages |
| **v0.4.0** | 1,100 | 6 | 35 | 13 new features |
| **v0.3.1** | 800 | 3 | 20 | Initial release |

---

## Contributing

Contributions are welcome! Here's how:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Commit your changes (`git commit -m "Add my feature"`)
4. Push to the branch (`git push origin feature/my-feature`)
5. Open a Pull Request

### Ideas for contributions:
- Add pipe support (`cmd1 | cmd2`)
- Add a package manager simulation
- Add persistent state across sessions
- Improve the AsciiDash engine with graphics
- Add more trivia questions and quiz content
- Add new games (Tetris Pong variants, puzzles, etc.)
- Add `vim` command with basic modal editing
- Add tab completion for commands and paths

---

## License

This project is licensed under the GNU General Public License v3. See the [LICENSE](LICENSE) file for details.

---

## You Might Also Like

- [Termux TUI Package Store](https://github.com/Mark44928/Termux-TUI-Package-Store) - Interactive fzf-powered package browser for Termux
- [Anti-Bloatware List](https://github.com/Mark44928/Anti-bloatware-list-for-Android-TV-Boxes-and-Sticks-for-rooted) - Debloat rooted Android TV sticks

---

<p align="center">
  <b>Found a bug? Open an issue. Made something cool? Submit a PR. Just here for the games? We don't blame you.</b><br/><br/>
  <img src="https://img.shields.io/badge/Made_with-❤️-red?style=for-the-badge" alt="Made with love"/>
  <img src="https://img.shields.io/badge/Powered_by-C%2B%2B-blue?style=for-the-badge" alt="C++"/>
  <img src="https://img.shields.io/badge/Termux-Ready-green?style=for-the-badge" alt="Termux"/>
  <img src="https://img.shields.io/badge/Zero-Dependencies-brightgreen?style=for-the-badge" alt="Zero Dependencies"/>
</p>

---

<p align="center">
  <sub><i>NoNameOS — because who needs a real OS when you can have 24 games and a terminal that judges your typing speed?</i></sub>
</p>
