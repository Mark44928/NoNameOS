<p align="center">
  <img src="https://github.com/Mark44928/NoNameOS/raw/main/Screenshot_20260524-150528~2.png" width="500" alt="NoNameOS Screenshot"/>
</p>

<h1 align="center">NoNameOS</h1>

> **Note:** This screenshot is from v0.3.1. The latest version (v1.0.0) has more commands, games, and tools not shown here.

<p align="center">
  <b>A pure C++ hobbyist operating-system simulation featuring an interactive shell, virtual filesystem with metadata, 24+ built-in games, and 135+ developer tools — all contained in a single source file. No external dependencies. No framework. Just compile and run.</b>
</p>

<p align="center">
  <a href="https://github.com/Mark44928/NoNameOS/forks"><img src="https://img.shields.io/github/forks/Mark44928/NoNameOS?style=social" alt="Forks"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/stargazers"><img src="https://img.shields.io/github/stars/Mark44928/NoNameOS?style=social" alt="Stars"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/blob/main/LICENSE"><img src="https://img.shields.io/github/license/Mark44928/NoNameOS?color=blue" alt="License"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/issues"><img src="https://img.shields.io/github/issues/Mark44928/NoNameOS" alt="Issues"/></a>
  <a href="https://github.com/Mark44928/NoNameOS/releases"><img src="https://img.shields.io/github/v/release/Mark44928/NoNameOS" alt="Release"/></a>
  <img src="https://img.shields.io/badge/version-1.0.0-green" alt="Version"/>
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
- [What's New in v1.0.0](#whats-new-in-v100)
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
| **Interactive Shell** | Color-coded prompt with command duration, 135+ commands, alias support, command history, man pages, fuzzy command suggestions |
| **Virtual Filesystem** | Create, read, copy, move, delete, symlink files and directories with sizes, timestamps & permissions; trash/recycle |
| **Games (24+ built-in)** | AsciiDash, Snake, Minesweeper, Tic-Tac-Toe, Hangman, RPS, Guess, Trivia, Adventure RPG, 2048, Typing Test, Reaction Test, Number Memory, Tetris, Pong, Sudoku, Flappy Bird, Memory Cards, Connect Four, Lights Out, Sliding Puzzle, Breakout, Whack-a-Mole, Wordle, Quiz |
| **System Tools** | Text editor (`nano`), calculator (`calc`/`bc`), system info (`cfetch`), `free`, `dmesg`, `lscpu`, `ps`, `top`, `uname`, `mem`, `cpu`, `disk` |
| **Utilities** | `grep`, `find`, `locate`, `sort`, `wc`, `head`, `tail`, `rev`, `tr`, `cut`, `uniq`, `nl`, `fold`, `cowsay`, `fortune`, `lolcat`, `sl`, `matrix`, `colors`, `weather`, and more |
| **Converters & Encoding** | `base64`, `rot13`, `morse`, `roman`, `binary`, `hash`, `urlencode`/`urldecode`, `hexdump`, `encode` |
| **Math & Stats** | `calc`, `calc2`, `bc`, `bmi`, `tip`, `stats`, `factor`, `random`, `dice`, `coin`, `scrabble` |
| **Productivity** | `todo`, `notes`, `stopwatch`, `timer`, `pom` (pomodoro), `alarm`, `countdown`, `password`, `worldclock` |
| **Text Tools** | `wordcount`, `diff`, `capitalize`, `reverse`, `repeat`, `csv`, `sparkline`, `bar` |
| **Fun** | `quote`, `joke`, `emoji`, `pick`, `zodiac`, `age`, `datecalc`, `weather`, `ip` |

---

## What's New in v1.0.0

- **11 new games:** Tetris, Pong, Sudoku, Flappy Bird, Memory Cards, Connect Four, Lights Out, Sliding Puzzle, Breakout, Whack-a-Mole, Wordle, Quiz
- **60+ new commands:** `tetris`, `pong`, `sudoku`, `flappy`, `memory`, `connect4`, `lightsout`, `puzzle`, `breakout`, `whack`, `wordle`, `quiz`, `colors`, `weather`, `epoch`, `uuid`, `base64`, `rot13`, `uppercase`, `lowercase`, `wordcount`, `matrix`, `cmtheme`, `countdown`, `ascii`, `hexdump`, `password`, `quote`, `joke`, `ip`, `mem`, `cpu`, `disk`, `calc2`, `bmi`, `tip`, `units`, `roman`, `binary`, `morse`, `bar`, `sparkline`, `colorgen`, `palette`, `diff`, `csv`, `stats`, `age`, `datecalc`, `encode`, `hash`, `urlencode`, `urldecode`, `reverse`, `capitalize`, `repeat`, `scrabble`, `emoji`, `random`, `pick`, `dice`, `coin`, `zodiac`, `worldclock`, `quiz`, `wordle`
- **Enhanced visuals** — 256-color ANSI support, truecolor, animated boot logo with RGB gradient, styled game headers, progress bars, color-coded system info
- **All previous v0.7.0 features** — 75+ commands, 13 games, VFS, shell, man pages, fuzzy suggestions

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
| `countdown` | `countdown <sec>` | Countdown with progress bar (1-600s) |
| `password` | `password` | Generate secure random password (12-20 chars) |
| `worldclock` | `worldclock` | Show times across 6 time zones |

### Converters & Encoding

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

### Math & Stats

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

### Text Tools

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

### Fun & Info

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

### Tetris (`tetris`)
Classic falling-block puzzle. Move with A/D, rotate with W, hard-drop with S.
```bash
tetris
```

### Pong (`pong`)
Two-player Pong with AI. Left paddle: W/S. Right paddle: O/L. First to 5 wins.
```bash
pong
```

### Sudoku (`sudoku`)
9x9 Sudoku puzzle with pre-filled clues. Enter `r c value` to place numbers.
```bash
sudoku
```

### Flappy Bird (`flappy`)
Flap through pipes! SPACE or W to flap. How far can you go?
```bash
flappy
```

### Memory Cards (`memory`)
Match emoji pairs on a 4x4 grid. Enter `r c` to flip cards.
```bash
memory
```

### Connect Four (`connect4`)
Drop discs into a 7-column grid. First to 4 in a row wins. AI opponent.
```bash
connect4
```

### Lights Out (`lightsout`)
Toggle lights on a 5x5 grid. Each toggle flips neighbors too. Turn all lights off.
```bash
lightsout
```

### Sliding Puzzle (`puzzle`)
The classic 15-puzzle. Slide tiles with WASD to arrange them in order.
```bash
puzzle
```

### Breakout (`breakout`)
Break all the bricks with a bouncing ball. A/D to move the paddle.
```bash
breakout
```

### Whack-a-Mole (`whack`)
Whack moles as they pop up! Press 1-9 to whack. 3 misses and you're out.
```bash
whack
```

### Wordle (`wordle`)
Guess the 5-letter word in 6 attempts. Green = correct, Yellow = wrong position.
```bash
wordle
```

### Quiz (`quiz`)
5 random questions from a pool of 10. Test your general knowledge.
```bash
quiz
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
- Add pipe support (`cmd1 | cmd2`)
- Add a package manager simulation
- Add persistent state across sessions
- Improve the AsciiDash engine with graphics
- Add more trivia questions and quiz content

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
