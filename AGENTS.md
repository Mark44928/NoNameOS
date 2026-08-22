# AGENTS.md

Single-file C++ terminal OS simulation. All code lives in `NoNameOS.cpp` (~6374 lines, `NoNameOS.cpp:1`). No packages, tests, linters, or CI.

## Repo Layout

```
NoNameOS.cpp        # everything — ~6374 lines, 12 sections (see Architecture)
Makefile            # CXX ?= g++ -std=c++17 -Wall -Wextra -O2 -lpthread (Makefile:2)
README.md           # 825 lines, screenshots, 251-command reference — prose not source of truth
update-readme.sh    # tries to patch <!--STAT:*--> placeholders that don't currently exist — don't run blindly
.github/            # only ISSUE_TEMPLATE/ + PULL_REQUEST_TEMPLATE.md — no workflows/CI
.gitignore          # ignores nonameos, nonameos-debug, *.o/*.out, .vscode/, .idea/, .mimocode/ (Gitignore:1)
LICENSE / CODE_OF_CONDUCT.md / CONTRIBUTING.md  # standard; CONTRIBUTING has fork→branch→PR flow
Screenshot_*.png    # readme assets
```

## Build & Run

```bash
make                    # -> ./nonameos  (Makefile:14: $(CXX) $(CXXFLAGS) -o $@ $< -lpthread)
make debug              # -> ./nonameos-debug (-g -O0, Makefile:17)
make clean              # rm -f nonameos nonameos-debug (Makefile:20)
g++ -O3 NoNameOS.cpp -o nonameos -lpthread   # README one-liner
clang++ -O3 NoNameOS.cpp -o nonameos -lpthread # Termux primary compiler (Termux ships clang, not g++)
./nonameos              # interactive raw-mode REPL; Ctrl+C / `exit` to quit; needs a tty
```

- **Must have `-lpthread` and `-std=c++17`** (`Makefile:2`, `Makefile:14`). Without `-lpthread` = link error (undefined `pthread_*`). `CXX ?= g++` means `CXX=clang++ make` works.
- **POSIX-only:** `<termios.h> <unistd.h> <fcntl.h> <poll.h>` (`NoNameOS.cpp:51`). Linux/WSL/Termux only; fails on native Windows. README Build Instructions list `pkg install clang` (Termux), `apt install g++` (Debian/Ubuntu), `dnf install gcc-c++` (Fedora), `pacman -S gcc` (Arch).
- **Optimisation:** Makefile uses `-O2`; README one-liner advertises `-O3` — both pass `-Wall -Wextra` clean. Full `-O2` build can exceed 60–90s on low-end Termux devices; quick syntax check: `clang++ -std=c++17 -Wall -Wextra -O0 -fsyntax-only NoNameOS.cpp`.
- **Binaries are gitignored** (`.gitignore:1`) — never commit `nonameos`/`nonameos-debug`/`*.o`. `compile_commands.json`, `.cache/`, `core` also ignored.
- **`make readme` is stale:** `update-readme.sh` does `sed -n '/^const set<string> ALL_COMMANDS/,/};/p'` and counts totals, then `sed -i "s/<!--STAT:...-->//"` — but `README.md` has no such placeholders, so the script no-ops/fails to patch.

## Architecture — 12 Sections (grep `// SECTION` at NoNameOS.cpp:28)

| Sec | Line | Contents | Key symbols |
|-----|------|----------|-------------|
| 1 | 28 | Includes, Utils | `rng()` `rng_int()` (`NoNameOS.cpp:217`), `expr::Parser` recursive-descent (`NoNameOS.cpp:230`, ops `+ - * / % ^`, `()` unary, right-assoc `^`, prints own errors), `constexpr` constants (`NoNameOS.cpp:325`) |
| 2 | 59 | Signals/Terminal | `g_orig_term/g_term_saved/g_sigint/g_quit/g_stdin_eof` (`NoNameOS.cpp:69`), `sigint_handler` flag-only (`74`), `sigtstp_handler` restores cooked+`raise(SIGTSTP)` (`77`), `sigcont_handler` re-applies raw (`84`) |
| 3 | 97 | Colors | `namespace clr` (`100`): `fg256()/rgb()/rgbbg()`, 256-colors `red/green/...`, semantic `success/error/warning/info/accent/muted/header/prompt_*`; helpers `vsep()`/`repeat()` (`148`), UTF-8 `utf8_*()` (`161`–`214`) |
| 4 | 355 | FSNode/TerminalGuard/Question | `FSNode` (`422`: `is_dir/is_link/content/mode/link_target/created_at`), `TerminalGuard` RAII raw+`O_NONBLOCK` (`367`), `get_timestamp()` (`404`) |
| 5 | 414 | VFS impl | `resolved_path()` symlink depth 8 (`436`), `help_text()` map (`452`), `parse_command()` (`540`), `resolve_user_path()` (`551`), `has_traversal()` (`582`) |
| 6 | 594 | Input | `kbhit()` peek_buf+`read()` (`602`), `getkey()` `pb[2]/pend`+`poll()`+fd-read (`628`), `enum Key` `KEY_*=1000+` (`617`), `readline_global()` (`755`) |
| 6.5 | 874 | Cooked helper | `cooked_readline()` (`879`): drains `kbhit()`, falls back to `getline` if `g_term_saved==0`, saves/restores `O_NONBLOCK` + `termios` |
| 7 | 914 | Command DB + fuzzy | `ALL_COMMANDS` set 278 entries (`920`), `edit_dist()` Levenshtein (`951`), `closest_cmd()` threshold 4 (`964`), text-tools `cmd_rev` etc (`973`) |
| 8 | 1236 | Games (24+) | `play_asciidash()` (`1245`), `play_snake()` (`1301`), `play_minesweeper()` (`1388`), `play_tictactoe()` minimax (`1533`), `play_tetris/pong/sudoku/flappy/memory/connect4/lightsout/puzzle/breakout/whack` etc |
| 11.5 | 3592 | v2.0 additions | `play_othello/mastermind/simon/blackjack/battleship/slots/maze/nim/buzz/bulls/twentyone/jumble/arith/bingo`, system `cmd_kernel/boot/shutdown/netstat/dns/traceroute/lsblk/mount/fib/primes/collatz/sine/cube/shell/logout/report/mac/geoip/dict/log`, eggs `cmd_llm/gpt/blockchain/mine/neo/...` (`4333`), effects `cmd_spinner/progress/gauge/rain/fire/stream/type` (`4414`) |
| 12 | 4532 | main | boot logo (`4547`), animated bar 8 steps (`4575`), VFS init `/`+`/geometry/jumper.gmd/` (`4605`), `aliases` map (`4616`), REPL loop + `if/else if` dispatch (~4700+) |

- **Entrypoint:** `int main()` at `NoNameOS.cpp:4537` — `signal(SIGINT/SIGTSTP/SIGCONT)`, `TerminalGuard tg`, clear `\033[2J\033[1;1H`, gradient logo, `boot_steps` vector, `file_system["/"]`, `current_user="root"`/`current_dir="/"`, `cmd_history`, `aliases["ll"]="ls -l"`, `[".."]="cd .."`, `["ttt"]="tictactoe"` (`4618`), then `while(true)` with `readline_global()` → `parse_command()` → one-shot alias resolve → dispatch. `last_cmd_end` tracks duration; prints `...ms` if history-nonempty and dur>100 (`4626`).
- **Version:** `const string VERSION = "v2.0.0"` (`NoNameOS.cpp:94`). Bump there + update `README.md` badges/history table.
- **Constants:** replace magic numbers with named `constexpr` at `NoNameOS.cpp:325` — `SNAKE_W/H`, `MINESWEEPER_W/H/MINES`, `TTT_BOARD_CELLS`, `HANGMAN_ATTEMPTS`, `SLEEP_MAX_SEC` (30), `YES_COUNT` (100), `HEAD_TAIL_LINES` (10), `TETRIS_W/H`, `PONG_W/H`, `FLAPPY_W/H`, etc.
- **Global users:** `vector<string> SYSTEM_USERS = {"root","user","guest","admin"}` (`NoNameOS.cpp:1731`); `su/useradd/userdel` mutate it.

## VFS — Where Agents Guess Wrong

- **Backing store:** `map<string, FSNode> file_system` local to `main` (`4605`). `FSNode` ctor: dir → `mode="rwxr-xr-x"`, file → `"rw-r--r--"`, `created_at=get_timestamp()` (`"%b %d %H:%M"`).
- **All keys end with `/`** — even files: `"/geometry/jumper.gmd/"` (`4610`). `resolve_user_path(arg, current_dir)` (`551`) always appends `/` after collapsing `// ./ ../` via `parts` vector; root `"/"` is special. `resolved_path(fs, path, depth)` (`436`) follows `is_link` up to depth 8, trying target with/without trailing `/` to match VFS keys.
- **Helpers — use them, don't hand-roll:**
  - `resolve_user_path(arg, cdir)` (`551`) — absolute vs relative, `..` pop, trailing `/`.
  - `resolved_path(fs, path)` (`436`) — symlink chase, slash-variant fallback.
  - `has_traversal(path)` (`582`) — component == `".."` → block; checked by `echo/cat/rm/cp/mv/play/grep/head/tail/sort/wc/tee` etc.
  - `vfs_read(path, fs, cdir)` (`866`) — `resolve_user_path`+`resolved_path`, returns `optional<string>` only if `!is_dir`.
- **Operations:**
  - `mkdir` (`4712`): splits on `/`, creates **intermediate parents** automatically.
  - `touch` (dispatch `touch`): `FSNode(false,"")` if missing.
  - `echo <file> <content>` (`4750`): first space splits filename/content, strips outer `"`; `has_traversal` guard; `fs[fullpath]=FSNode(false,content)`. Easter: `echo hello world` prints `Hello, World! 🌍`.
  - `cat <file>` (`4775`): `/dev/brain` easter, `vfs_read`, `file not found` else print `content`.
  - `cp` (`5132`) / `cp -r` (`5138`): `-r` collects `to_copy` vector first (iterator-safe) then inserts; if `dst` is existing dir, copies **into** it with basename. Non-`-r` copies single file similarly.
  - `mv` (`5202`): similar into-dir logic; directory mv moves all `rfind(srcDir)==0` descendants.
  - `rm` (`4788`): moves to `/trash/<epoch>_<basename>` (creates `/trash/` if absent). `rm -r <dir>` recursively via `rfind`. Guards: `rm -rf /` / `-rf /*` / `-r /` → `☢ NUCLEAR LAUNCH DENIED ☢`.
  - `ln -s <target> <link>` (dispatch `ln`): `FSNode` with `is_link=true`, `link_target=target`; `resolved_path` later resolves verbatim then slash-toggled variant.
  - `ls` (`4683`): `ls == ls` with easter `args=="/"` → `Here be dragons. 🐉`; `ls -l` long mode prints `type+mode size timestamp name`. Empty dir → `(Empty)`. Direct child check via `rfind(current_dir)==0` + `slash_pos==npos`.
  - `tree` (`1954`), `du` (`1152`), `locate` (`1170`), `chmod <mode 9> <file>` (`5255` expects `rwxr-xr-x` length 9), `head/tail` → `HEAD_TAIL_LINES=10` (`5327`/`5341`), `sort`/`wc`/`tee`, `grep <pat> <file>` (substring `find`), `find <name>` (path substring).
  - `trash list|empty` manages `/trash/` prefix.
- **Permissions/timestamps/sizes:** `mode`+`created_at`+`size()==content.size()` shown in `ls -l`; `chmod` mutates `mode`; timestamps from `get_timestamp()`.

## Terminal / Input — Do Not Break

- **RAII:** `TerminalGuard` at `367` does `tcgetattr` → saves `g_orig_term`+`g_term_saved=1`, sets `c_iflag&=~(IGNBRK|BRKINT|...|IXON)`, `c_lflag&=~(ICANON|ECHO|ECHONL|IEXTEN)`, `c_cflag` to `CS8`, `tcsetattr(TCSANOW,newt)`, `fcntl(F_SETFL,O_NONBLOCK)`. Destructor restores. If `tcsetattr` fails, marks `active=false`, `g_term_saved=0` and restores old.
- **Globals:** `g_orig_term` (`69`), `g_term_saved` volatile (`70`), `g_sigint` (`71`), `g_quit` (`72`), `g_stdin_eof` (`73`).
- **Signals:** `SIGINT` handler sets `g_sigint=1` only (`74`) — **never** `tcsetattr` there (not async-signal-safe). Main loop & `readline_global` poll flag: `if(g_sigint){ g_sigint=0; cout<<"\n"; line.clear(); return line; }` (`767`). `SIGTSTP` handler restores cooked, `signal(SIGTSTP,SIG_DFL); raise(SIGTSTP); signal(...,handler)` (`77`); `SIGCONT` re-applies raw from `g_orig_term` (`84`). Comment at `59` notes `tcsetattr` in `SIGTSTP/CONT` is technically unsafe but pragmatic to avoid leaving terminal raw.
- **`kbhit()` (`602`):** returns 1 if `peek_buf!=EOF` else `read(STDIN_FILENO,&c,1)==1` stores to `peek_buf`. On `n==0` or `errno!=EAGAIN/EWOULDBLOCK` sets `g_stdin_eof=true`.
- **`getkey()` (`628`):** static `pb[2]={EOF,EOF}` + `pend` string (cross-call escape-sequence buffer) + `peek_buf`. Helpers: `read_ch()` (drains `pb`, then `peek_buf`, else fd `read()` — **never `getchar()`** which desyncs `poll()`), `available(ms)` via `poll(POLLIN)`, `normalize`, `arrow_of`, `tilde_of`, `wait_byte(120)`, `param1_of`. Handles:
  - `ESC` lone → `KEY_ESC` but keeps `"\x1b"` pending 120ms so slow phones don't split arrow.
  - `ESC O A/B/C/D` (SS3) → arrows, `ESC [ A/B/C/D/H/F` plain, `ESC [ 1 ; <m> A/B/C/D` modified arrows (`m=2..8`), `ESC [ 1~/2~/3~/4~/5~/6~` → `HOME/INSERT/DELETE/END/PGUP/PGDN`. Returns `KEY_NONE` while stalled so callers retry; `poll`/`read` at fd level avoids stdio buffering lost bytes.
- **`readline_global(history)` (`755`):** full line editor with `string line; size_t cursor; int hist_idx=history.size(); string saved;`. `redraw_from()` prints `\033[2K\r❯ line` and moves cursor. Loop: poll `g_sigint`→abort, `getkey()==KEY_NONE` + `g_stdin_eof` check, `KEY_ENTER` returns, `KEY_UP/DOWN` history (saves current to `saved`), `LEFT/RIGHT/HOME/END/PGUP/PGDN/DEL`, `Backspace` (127/8), printable `32..255` or `KEY_SPACE` inserted at `cursor` (bytes, not UTF-8 chars — `// NOTE:` at `753` warns cursor counts **bytes**, multibyte/wide chars drift). Signals and EOF handled each iteration with `sleep(1ms)`.
- **`cooked_readline()` (`879`):** drains pending `while(kbhit()) getkey();`. If `g_term_saved==0` (non-tty/no `TerminalGuard`) falls back to `getline(cin,line)` with `cin.clear()` and `g_stdin_eof` on `eof/fail`. Else saves `raw_t`, clears `O_NONBLOCK`, restores `g_orig_term` (cooked), `getline`, then re-applies raw via `g_orig_term` with same mask as `TerminalGuard`. Also sets `g_stdin_eof` on `eof/fail`.
- **Mode-switch rule:** **always** `while(kbhit()) getkey();` before `cooked_readline()` (`880` comment); `cooked_readline` itself drains first 3 lines. Real-time games (`rain/fire/stream` etc) use `kbhit()`/`getkey()` loop with `!kbhit()` sleep and drain on exit `while(kbhit()) getkey();` (`4460`).

## Command System

- **`parse_command(input)` (`540`):** `first_space=find(' ')`; if none → `{input,""}` else `args=substr(first_space+1)` stripped of **surrounding** `"` only (`if args.front()=='"' && args.back()=='"'`). Alias expansion happens once before dispatch in `main` (`4641`): if `aliases[cmd]` exists, `alias_cmd += " "+args` → re-`parse_command`.
- **Dispatch:** single `if/else if (cmd=="...")` chain from `~4700` through `~6350` (`12: main`). Sections commented `// --- UTILITIES ---`, `// --- GAMES ---`, etc. Includes `if(cmd=="/")` easter, `echo` hello-world, `cat /dev/brain`, `whoami really` existential, `rm -rf` deny.
- **Help:** `help` (`4650`) without arg prints categorized via local `section(title,cmds)` lambda with `vsep(42)`; with arg prints `help_text(args)` (`452` map). `help_text` covers ~80 commands; `man <cmd>` (`5413`) has static `manpages` map for common commands with `NAME/SYNOPSIS/DESCRIPTION`. Keep both maps in sync with `ALL_COMMANDS`.
- **Fuzzy:** `edit_dist(a,b)` (`951`) Levenshtein DP `prev/cur` vectors; `closest_cmd(cmd)` (`964`) iterates `ALL_COMMANDS` with `best_d=4`, returns `""` if dist>=4. Dispatch fallback near end (~`6340`) prints `closest_cmd` suggestion: `did you mean 'X'?`.
- **`ALL_COMMANDS` (`920`):** `const set<string> ALL_COMMANDS = {"ls","cd",...,"up"}` — **278 entries** including games, eggs, effects. Controls `which`, `closest_cmd`, and `which` lookup. **Forgetting a new command here = no typo suggestion, `which` says not found, and `update-readme.sh` miscounts** (it greps this set).
- **Aliases:** `map<string,string> aliases` in `main` (`4616`) with defaults `ll→ls -l`, `..→cd ..`, `ttt→tictactoe`; `alias [name=val]` (`5054`) and `unalias` mutate it at runtime; `alias` without args lists all.
- **Prompt:** `\n<user>@nonameos:<dir>\n❯ ` with `clr::prompt_user/host/dir/sep` (`4630`); command duration `…ms` printed before next prompt if `dur>100` and history-nonempty (`4626`).

## Games — 40 Total (know the input mode)

- **Turn-based via `cooked_readline()`** ( echo-typed, ENTER to submit): `othello` (r c), `mastermind` (4 digits 1-6), `blackjack` (h/s/q), `battleship` (r c), `slots` (ENTER/q), `maze` (wasd), `nim` (pile stones), `buzz` (fizzbuzz), `bulls` (4 digits), `twentyone` (r/h/q), `jumble` (word), `arith` (number), `bingo` (auto), plus older: `minesweeper` (x y / f x y), `tictactoe` (1-9), `hangman` (letter), `rps` (1-3), `2048` (wasd), `typing`, `reaction` (cooked after), `nummem`, `wordle`, `quiz`, `trivia`, `adventure` (left/right/rest), `guess` (1-100), `sudoku` (r c value), `memory`, `connect4` (1-7), `lightsout` (r c), `puzzle` (wasd), `whack` (1-9), `flappy` (SPACE — but also cooked fallback). Check `g_stdin_eof||g_sigint` each iteration; `q/Q` (and often `ESC`) quits; final `Press Enter to return…` via `cooked_readline()`.
- **Realtime via `kbhit()/getkey()` loop** (non-blocking, no ENTER): `snake` (WASD/arrows), `tetris` (A/D move, W rotate, S drop), `pong` (W/S), `breakout` (A/D), `flappy`/`play` AsciiDash (SPACE/W/↑ jump), `simon` (arrow keys, `getkey()` matching `KEY_*`), `rain/fire/stream` effects (any `kbhit()` exits). They `poll`/`sleep(1..50ms)` and drain on exit.
- **Custom maps:** `play [file]` (`4836`) loads VFS file content as AsciiDash map data (`^` obstacles, `_` flat). Default `jumper.gmd` preloaded at `/geometry/jumper.gmd/` (`4610`) content `"_______^_______^^..."`. `echo mymap.gmd ____^^...` then `play mymap.gmd`.
- **Categories reflected in `help`:** `Games` (classic), `v2.0 Games` (14 new: `othello mastermind simon blackjack battleship slots maze nim buzz bulls twentyone jumble arith bingo`), plus `Productivity` etc.

## Visuals / Helpers / RNG / Expr

- **`clr` namespace** (`100`): `fg256(c)`, `rgb(r,g,b)`, `rgbbg(r,g,b)`, constants `reset/bold/dim/italic/underline/blink/inverse/black/red/lred/green/.../white/gray/dgray`, semantic `success/error/warning/info/accent/muted/header/prompt_*`. Use for all output; `colors` command (`2333`) shows swatches, `cmtheme/colorgen/palette` generate themes.
- **Visual helpers:** `vsep(width,ch="─")` (`148`), `repeat(n,ch)` (`155`).
- **UTF-8:** `utf8_seq_len(c)` (`163`), `utf8_length(s)` (`170` counts chars), `utf8_take(s,n)` (`177` no split), `utf8_reverse(s)` (`188` walks continuation bytes `0x80`), `utf8_fold(s,width)` (`200` splits into width-char lines). Used by `rev/tr/cut/fold/reverse/type/slots`. **Readline cursor is byte-based**, so wide chars drift — documented at `753`.
- **RNG:** `mt19937& rng()` (`217`) `static mt19937 gen(random_device{}())`, `rng_int(lo,hi)` (`221`) uniform_int. Never use `srand/rand`. Used everywhere: games, `fortune`, `quote`, `shuf`, `dice`, `uuid`, `password`, `colorgen`.
- **Expr:** `namespace expr { struct Parser { ... } }` (`230`) recursive-descent: `number()`, `primary()` (`(` `expr_op` `)`), `unary()` (`-/+`), `power()` (`^` right-assoc via recursion), `term()` (`* / %`), `expr_op()` (`+ -`), `eval()` checks `i==size && !error`. Prints own errors; `cmd_bc`/`calc`/`calc2` call `expr::evaluate(s,out)`.
- **Other helpers:** `boot_delay(ms)` (`535`), `Question` struct (`361`), `get_timestamp()` (`404` `"%b %d %H:%M"` via `localtime_r`).

## Editing Conventions

- **Keep everything in `NoNameOS.cpp`** — no new files/build system. C++17 + STL + POSIX only; zero dependencies (`README.md:29` tech stack table). Approved headers at `30`–`54`; don't add external libs.
- **Style:** follow existing 4-space indent, `clr::` for colors, `rng_int` for randomness, `cooked_readline()` vs `kbhit/getkey` as appropriate, `has_traversal` guard on any path arg, `resolved_path` for symlink. Keep `-Wall -Wextra` clean (Termux `clang` is primary — must compile there).
- **Magic numbers → `constexpr`** at `325` block. Add new constants there rather than inline literals.
- **Sections:** edit within the correct `// SECTION` block and comment header `// --- ... ---` (e.g., `// --- TEXT PROCESSING TOOLS ---` at `973`, `// --- ASCIIDASH ENGINE ---` at `1242`). Don't reorder sections; `main` dispatch order roughly follows help categories but any order works as long as `ALL_COMMANDS`/`help_text` stay synced.
- **No formatting/lint/CI:** no `clang-format`/`.editorconfig` — match surrounding code.

## How to Add a New Command / Game / Easter Egg / Effect

1. **Pick location:** command helper `cmd_foo()` near related helpers (e.g., text tools at `973`, system at `1092`, productive at `1179`, massive batch at `3044`, v2.0 at `3592/4092/4333/4414`) or game `play_foo()` in Section 8/11.5. Keep forward-decl if needed.
2. **Register strings:** add `"foo"` to `ALL_COMMANDS` (`920`) — determines `which` + fuzzy — and `{"foo","description"}` to `help_text()` (`452`). For `man`, add entry to `manpages` map inside `man` branch (`5413`). For help category, add to the `section("Category", "...")` string in `help` (`4667`), otherwise it won't appear in `help` listing.
3. **Dispatch:** add `else if (cmd=="foo") { /* handle args, use resolve_user_path/vfs_read/has_traversal as needed, print with clr:: */ }` in `main`'s chain (~4700+). For games: `else if (cmd=="foo") { play_foo(); }` at `// --- GAMES ---` or `// --- NEW GAMES ---` (`5771`).
4. **Input choice:**
   - Turn-based: `string line=cooked_readline(); if(line=="q"||g_stdin_eof) break;` — **precede with `while(kbhit()) getkey();` if coming from realtime mode.**
   - Realtime: `if(kbhit()){ int k=getkey(); if(k==KEY_Q) break; /* handle arrows */ }` with `sleep(1ms)` loop, drain on exit.
5. **Example command snippet:**
   ```cpp
   void cmd_foo(const string& args, map<string,FSNode>& fs, const string& cdir){
       if(args.empty()){ cout<<"Usage: foo <file>\n"; return; }
       if(has_traversal(args)){ cout<<"\033[31merror:\033[0m path traversal not allowed.\n"; return; }
       auto c=vfs_read(args,fs,cdir);
       if(!c){ cout<<"error: file not found.\n"; return; }
       cout<< *c << "\n";
   }
   // then in main dispatch:
   // else if(cmd=="foo") cmd_foo(args, file_system, current_dir);
   ```
6. **Easter egg/effect:** put under `// ---------- v2.0.0 EASTER EGGS ----------` (`4333`) or `// ---------- v2.0.0 TERMINAL EFFECTS ----------` (`4414`); reuse animation patterns (`\033[2J\033[1;1H` clear, `clr::` colors, `sleep_for`).

## Verification

No test suite, task runner, linter, or CI to run. Verify manually:

```bash
make clean && make          # or clang++ variant; must be zero warnings (-Wall -Wextra)
# quick syntax-only if build is slow on Termux:
clang++ -std=c++17 -Wall -Wextra -O0 -fsyntax-only NoNameOS.cpp
./nonameos                  # manual smoke — needs tty for raw mode
# smoke sequence:
help                        # categories show
ls; ls -l; mkdir /tmp/test; touch /tmp/test/f; echo /tmp/test/f hello; cat /tmp/test/f; tree; du; chmod rwxr-xr-x /tmp/test/f
calc 1+2*3; bc 2^10; calc2 "(1+2)*3"
alias ll="ls -l"; ll        # alias
ttt                         # alias for tictactoe — q to quit
snake; 2048                 # realtime — q to quit, arrows/WASD
othello; simon             # v2.0 game — q/ESC quits
matrix; rain; fire         # effects — any key exits
exit                        # or Ctrl+C (SIGINT flag path)
```

- **Non-tty fallback:** `cooked_readline()` checks `g_term_saved==0` and uses plain `getline`; so `echo help | ./nonameos` or piping works via cooked path but won't test raw editing/history. Canonical testing is interactive tty.
- **Common pitfalls:** forgetting `-lpthread` link; hand-rolling VFS paths without trailing `/`; using `getchar()` instead of `getkey()`/fd `read()`; not draining `kbhit()` before `cooked_readline()` leaving escape tail as typed garbage; forgetting `ALL_COMMANDS` entry; adding file without `help_text` entry.

## Gotchas Checklist (most frequent agent mistakes)

- **Never create new `.cpp`/headers** — single-file project. Add code to `NoNameOS.cpp` in the correct section.
- **Never commit `nonameos` binary** — gitignored; CI would reject.
- **Don't call `tcsetattr` from `SIGINT`** — only set `g_sigint`. `SIGTSTP/CONT` are the only handlers that touch termios (pragmatically).
- **Don't use `std::getline` directly in games** — use `cooked_readline()` so echo and mode restore work.
- **Don't use `getchar`/`cin.get()` in realtime** — desyncs `poll()`; use `kbhit()`/`getkey()` with `KEY_*` enum (`617`: `KEY_UP=1000, KEY_DOWN, LEFT, RIGHT, HOME, END, PGUP, PGDN, DEL, ENTER, SPACE, Q, ESC`).
- **Byte vs character cursor** — `readline_global` counts bytes; multiline UTF-8 near 256-code wraps may misalign — keep prompts simple.
- **Stale `VERSION` in badge/docs** — update `NoNameOS.cpp:94` and `README.md` badges/version history together.
- **`update-readme.sh` not to be trusted** — run only if you also add `<!--STAT:…-->` placeholders back to `README.md`.

## Workflow / Git

- **Branch:** `main` is sole branch (`origin/main`, `origin/HEAD → main`). No `develop`.
- **PR template:** `.github/PULL_REQUEST_TEMPLATE.md` expects Description, Type of Change (bug fix/new feature/docs/refactor), How Tested, Checklist (style, tested, docs, no warnings). Follow it.
- **Issues:** `bug_report.md` + `feature_request.md` under `.github/ISSUE_TEMPLATE/`.
- **Commit style:** existing log short imperative: `fix dispat: …`, `kill remaining arrow-garbage paths`, `feat: …`. `git status` should show only source changes; `nonameos` untracked is expected but don't `git add` it.
- **Contributing flow:** `CONTRIBUTING.md:1` fork → `git checkout -b feature/my-feature` → `git commit -m "Add my feature"` → `push` → PR.
- **Opencode config:** no `opencode.json`/`opencode.jsonc`/`.opencode/` present — no extra instructions to load via `instructions` field.

## References

- Executable source of truth over prose: `Makefile:2`, `NoNameOS.cpp:51` (POSIX headers), `NoNameOS.cpp:367` (TerminalGuard), `NoNameOS.cpp:594`/`874` (input), `NoNameOS.cpp:920` (ALL_COMMANDS), `NoNameOS.cpp:4532` (main).
- Docs to read first: `README.md` (features/build table/FAQ/platform notes), `CONTRIBUTING.md`, `.gitignore:1`, `.github/PULL_REQUEST_TEMPLATE.md`.
