# AGENTS.md

Single-file C++ terminal OS simulation. All code lives in `NoNameOS.cpp` (~7837 lines, `NoNameOS.cpp:1`). No packages, tests, linters, or CI.

## Repo Layout

```
NoNameOS.cpp        # everything — ~7837 lines, 12+ sections (see Architecture)
Makefile            # CXX ?= g++ -std=c++17 -Wall -Wextra -O2 -lpthread (Makefile:2)
README.md           # screenshots, command reference — prose not source of truth
update-readme.sh    # tries to patch <!--STAT:*--> placeholders that don't currently exist — don't run blindly
.github/            # only ISSUE_TEMPLATE/ + PULL_REQUEST_TEMPLATE.md — no workflows/CI
.gitignore          # ignores nonameos, nonameos-debug, *.o/*.out, .vscode/, .idea/, .mimocode/
LICENSE / CODE_OF_CONDUCT.md / CONTRIBUTING.md  # standard; fork→branch→PR flow
Screenshot_*.png    # readme assets
```

## Build & Run

```bash
make                    # -> ./nonameos  (Makefile:14: $(CXX) $(CXXFLAGS) -o $@ $< -lpthread)
make debug              # -> ./nonameos-debug (-g -O0)
make clean              # rm -f nonameos nonameos-debug
clang++ -O2 NoNameOS.cpp -o nonameos -lpthread   # Termux: g++ exists only as a clang wrapper — call clang++ directly
./nonameos              # interactive raw-mode REPL; Ctrl+C / `exit` to quit; needs a tty
```

- **Must have `-lpthread` + `-std=c++17`** (`Makefile:2`,`14`) or link fails. `CXX ?= g++` → `CXX=clang++ make` works.
- **POSIX-only:** `<termios.h> <unistd.h> <fcntl.h> <poll.h>` (`NoNameOS.cpp:51`). Linux/WSL/Termux; not Windows.
- Full `-O2` build takes >120s on this Termux device — use generous timeouts. Fast gate (~60s here): `clang++ -std=c++17 -Wall -Wextra -O0 -fsyntax-only NoNameOS.cpp`.
- Binaries gitignored — never commit. `make readme` patches `<!--STAT:*-->` placeholders that don't exist — skip it.
- Zero-warning rule: `-Wall -Wextra` must stay clean (verified Termux clang 21).

## Architecture — Sections (`grep '// SECTION' NoNameOS.cpp`)

| Sec | Line | Contents / key symbols |
|-----|------|------------------------|
| 1 | 28 | utils: `rng()/rng_int()`, `expr::Parser` (recursive-descent `+ - * / % ^`, right-assoc `^`), game `constexpr`s |
| 2 | 59 | signals+globals `g_orig_term/g_term_saved/g_sigint/g_quit/g_stdin_eof`; SIGINT flag-only, SIGTSTP/SIGCONT toggle termios |
| 3 | 97 | `namespace clr`: fixed 256-colors + **semantic colors = mutable refs** bound below; `THEME_DB` (:164) + `apply_theme()` (:174) runtime palettes — default `nonameos` + matrix/dracula/ocean/sunset/mono |
| 4 | 403 | `FSNode`, `TerminalGuard` RAII raw+O_NONBLOCK, `get_timestamp()` |
| 5 | 462 | VFS: `resolved_path` (symlink depth 8), `help_text`, `parse_command`, `resolve_user_path`, `has_traversal` |
| 5.8 | 654 | v3 shell globals: `g_stdin_buf/g_stdin_live` (pipe handoff), `g_script_vars`, `g_prompt_fmt` (`%u %h %d %t %%`), `g_tab_cmds/g_tab_paths/g_tab_cwd` |
| 6 | 673 | input: `kbhit`, `getkey` (cross-call `pend` escape buffer), `readline_global` (**TAB-complete + Ctrl+R search**), `enum Key KEY_*=1000+` |
| 6.5 | 1037 | `cooked_readline()` (:1042) — drains kbhit; getline fallback when `g_term_saved==0` |
| 7 | 1077 | `ALL_COMMANDS` set **269** entries, `edit_dist`/`closest_cmd` (threshold 4), text-tool helpers |
| 8 | 1407 | v1/v2 games: asciidash snake minesweeper tictactoe hangman rps 2048 typing reaction nummem tetris pong sudoku flappy memory connect4 lightsout puzzle breakout whack |
| 11.5 | 3763 | v2 games/system/eggs/effects: othello…bingo, kernel/boot/netstat…, llm/rain/fire/stream… |
| 11.7 | 4702 | v3 engine helpers: `persist_save/load`, `vfs_escape/unescape`, `expand_globs`, `filter_source` (pipe-aware), `script_subst`, `PKG_DB`+`g_installed_pkgs` |
| 11.8 | 4908 | v3 games/toys: `chess::` (full legality+castling+promo, greedy-2ply AI), `play_clicker` (`/home/clicker.sav`, offline earnings cap 8h), `cmd_pet` (`/home/pet.save`, time decay), `play_fishing` (`/home/fishing.log`), figlet/nyancat/starwars/sheep (pkg-gated), `cmd_news` |
| 12 | 5452 | `main()`: early theme/prompt peek → boot → persist_load → MOTD → `exec_segment` lambda (~5596) + `run_one` lambda (~7740) engine → REPL → `persist_save` on every exit |

- `VERSION = "v3.0.0"` at `NoNameOS.cpp:94` — bump together with README badges/history.
- `SYSTEM_USERS {"root","user","guest","admin"}` mutated by su/useradd/userdel, persisted via `U` lines.

## VFS — Where Agents Guess Wrong

- Backing store: `map<string, FSNode> file_system` local to `main`. **All keys end with `/`**, even files (`/geometry/jumper.gmd/`). `FSNode` ctor: dir→`rwxr-xr-x`, file→`rw-r--r--`, stamps `"%b %d %H:%M"`.
- Use the helpers, never hand-roll paths: `resolve_user_path(arg, cdir)` (absolute/relative, `..`, always trailing `/`), `resolved_path(fs,path)` (symlink chase ≤8, slash-variant fallback), `has_traversal(path)` (any `..` component → block), `vfs_read(path,fs,cdir)` → `optional<string>`.
- Ops notes: `mkdir` creates intermediate parents · `echo f c` first-space split, strips outer quotes · `rm` moves to `/trash/<epoch>_<name>` (guards `rm -rf /` → ☢) · `cp/mv` copy INTO existing dirs by basename; `-r` collects into vector before mutating (iterator safety) · `cat` is multi-file since v3 · `chmod` expects literal 9-char mode · `ls -l` shows type+mode+size+stamp.
- `ls == ls /` easter 🐉 · `echo hello world` easter · `cat /dev/brain` easter.

## v3.0 Shell Engine — How Lines Execute

- REPL → `run_one(line)` (~7740): split unquoted `;` and `&&` markers → each part split on unquoted `|` → stages through `exec_segment(seg,capture,captured)` (~5596).
- `exec_segment`: trim → `< file` input redirect (overrides pipe stdin) → `> >>` extraction → `expand_globs()` vs VFS keys → alias resolve once → giant dispatch chain. Returns false only on unknown command (drives `&&`; the skip is `i += 3` — marker AND target).
- Pipes: stdout captured via `cout.rdbuf()` swap into ostringstream; carried to next stage through `g_stdin_buf` + `g_stdin_live`. Filters consume via `filter_source()` or `args.empty() && g_stdin_live` branches (cat grep head tail sort wc rev uniq nl fold). **Never reset `g_stdin_*` at segment entry** — that broke pipes once.
- Scripts: `run x.nnsh args…` → per line strip `#comment` → `script_subst` (`$var $1-$9 $@`) → `set k=v` → one-line `if A == B then cmd` → else `run_one`. Interactive `$x` expansion also applies to any line containing `$`.
- Persistence `~/.nonameos.vfs` (HOME-based): `NNOS3` header, `T theme`, `P escaped-prompt`, `A alias=v`, `U user`, `K pkg`, nodes `D/L/F` with escaped content. Auto-saved on every exit path; theme/prompt peeked BEFORE logo so palette boots correctly.

## Terminal / Input — Do Not Break

- `TerminalGuard` RAII: raw flags + `O_NONBLOCK`, restores on destruction; inactive (non-tty) → `cooked_readline` getline fallback keeps piping usable.
- SIGINT handler sets flag ONLY (never tcsetattr); main loop + readline poll it. SIGTSTP/SIGCONT pragmatically toggle cooked/raw.
- `getkey()` reads at fd level + poll(); **never `getchar()`** (desyncs poll, splits escapes). Lone ESC stays pending 120ms; CSI/SS3/modified arrows/`~`-keys handled.
- Always drain `while(kbhit()) getkey();` before mode switches. Realtime loops drain on exit too.
- **Headless caveat:** without tty, fd blocks so `kbhit()` drains swallow piped input — interactive games are NOT headless-testable; text/chains/pipes ARE: `printf '%s\n' 'a | b' 'exit' | ./nonameos`.

## Command System

- Add a command ⇒ touch ALL of: `ALL_COMMANDS` (`1083`), `help_text()` map, dispatch chain inside `exec_segment`, optional `manpages` map + `section(...)` line in `help`. Missing `ALL_COMMANDS` breaks `which`, fuzzy suggest, update-readme counts.
- Fuzzy: Levenshtein `edit_dist`, suggestion threshold 4 via `closest_cmd`.
- Aliases default `ll .. ttt`, user aliases persist (`A` lines). Prompt template tokens `%u %h %d %t %%` rendered each prompt with themed colors; duration `…ms` shown after >100ms.
- pkg gating pattern: branch checks `g_installed_pkgs.count("figlet")` else prints install hint (see figlet/nyancat/starwars/sheep).

## Games — know the input mode

- Turn-based (`cooked_readline`): chess (`e2 e4`), othello mastermind blackjack battleship slots maze nim buzz bulls twentyone jumble arith bingo minesweeper tictactoe hangman rps sudoku wordle quiz trivia adventure guess memory connect4 lightsout puzzle typing nummem reaction.
- Realtime (`kbhit/getkey`): snake tetris pong breakout flappy/play AsciiDash clicker (SPACE click, 1-5 buy, q saves) simon rain fire stream.
- Persistent progress lives in VFS under `/home/` (clicker.sav, pet.save, fishing.log) — survives via session persistence.
- Custom AsciiDash maps: `echo map.gmd ____^^__` then `play map.gmd`.

## Visuals / Helpers

- `clr::` for ALL color output (semantic names follow active theme). `vsep/repeat` for rules. UTF-8 helpers `utf8_*` for char-safe slicing/reversal/folding (readline cursor counts BYTES — wide chars drift).
- RNG: `rng_int(lo,hi)` everywhere; never srand/rand.

## Verification

```bash
make clean && make          # zero warnings required
./nonameos                  # smoke: help · TAB complete · Ctrl+R · pipes · chains
printf '%s\n' 'echo /t.txt hi' 'cat /t.txt' 'cat /t.txt | grep hi' \
  'ls > /o.txt' 'wc /o.txt' 'bogus && echo NO' 'theme list' 'pkg list' 'exit' | ./nonameos
```

Gotchas checklist: missing `-lpthread` · trailing-`/` VFS keys · `getchar()` in realtime · undrained input before cooked mode · missing `ALL_COMMANDS` entry · resetting pipe globals per-stage · `i+=2` vs `i+=3` in `&&` skip · editing README counts by hand instead of grepping `ALL_COMMANDS`.

## Workflow

- Branch `main` only. PR template checklist: style/tested/docs/no-warnings. Commit style: short imperative ("fix …", "add …"). Never commit binaries. No opencode.json present.

## References

- Executable truth: `Makefile:2` · POSIX includes `NoNameOS.cpp:51` · TerminalGuard `:415` · input `:654-1077` · `ALL_COMMANDS` `:1083` · persistence `:4702+` · main `:5452` · exec_segment `:5596` · run_one `:7740`.
- Docs: README (features/build/FAQ), CONTRIBUTING.md, `.github/PULL_REQUEST_TEMPLATE.md`.
