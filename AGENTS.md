# AGENTS.md — NoNameOS

Single-file C++ terminal OS simulation. All code lives in `NoNameOS.cpp` (~8533 lines). No packages, tests, or linters; CI exists at `.github/workflows/build.yml`.

## Repo Layout

```
NoNameOS.cpp        # everything — ~8533 lines, 13+ sections
Makefile            # CXX ?= g++ -std=c++17 -Wall -Wextra -O2 -lpthread
README.md           # screenshots, command reference (prose not source of truth)
update-readme.sh    # patches <!--STAT:*--> anchors in README.md; `make readme`
.github/            # ISSUE_TEMPLATE, PULL_REQUEST_TEMPLATE.md, workflows/build.yml
.gitignore          # ignores nonameos, nonameos-debug, *.o/*.out, .vscode/, .idea/, .mimocode/
LICENSE / CODE_OF_CONDUCT.md / CONTRIBUTING.md  # fork→branch→PR flow
Screenshot_*.png    # readme assets
```

## Build & Run

```bash
make                    # ./nonameos
make debug              # ./nonameos-debug (-g -O0)
make clean              # rm -f nonameos nonameos-debug
clang++ -std=c++17 -Wall -Wextra -O2 NoNameOS.cpp -o nonameos -lpthread  # Termux: call clang++ directly
./nonameos              # interactive raw-mode REPL; Ctrl+C / `exit` to quit; needs a tty
```

- **Must have `-lpthread` + `-std=c++17`** or link fails. `CXX=clang++ make` works on Termux.
- Full `-O2` build takes >120s on this Termux device. Fast gate (~60s): `clang++ -std=c++17 -Wall -Wextra -O0 -fsyntax-only NoNameOS.cpp`.
- Binaries gitignored — never commit. `make readme` replaces `<!--STAT:*-->` anchors only; ASCII-art sections and Version History are NOT anchored — patch by hand only when stats change.
- Zero-warning rule: `-Wall -Wextra` must stay clean (verified Termux clang 21).
- `VERSION = "v4.0.0"` at `NoNameOS.cpp:96` — bump with README badges/history.

## Architecture — Sections (`grep '// SECTION' NoNameOS.cpp`)

| Sec | Line | Contents |
|-----|------|----------|
| 1 | 28 | utils: `rng()/rng_int()`, `expr::Parser` (+ - * / % ^, right-assoc `^`), game `constexpr`s |
| 2 | 61 | signals+globals `g_orig_term/g_term_saved/g_sigint/g_quit/g_stdin_eof`; SIGINT flag-only, SIGTSTP/SIGCONT toggle termios |
| 3 | 139 | `namespace clr`: fixed 256-colors + semantic colors (mutable refs); `THEME_DB` (:206) + `apply_theme()` (:218) |
| 4 | 447 | `FSNode` (:514), `TerminalGuard` RAII raw+O_NONBLOCK (:463), `get_timestamp()` (:496) |
| 5 | 506 | VFS: `resolved_path` (symlink depth 8), `help_text` (:544), `parse_command`, `resolve_user_path`, `has_traversal` |
| 5.8 | 708 | v3 shell globals: `g_stdin_buf/g_stdin_live`, `g_script_vars`, `g_prompt_fmt` (`%u %h %d %t %%`), tab/cwd completion |
| 6 | 727 | input: `kbhit` (:735), `getkey` (cross-call `pend` escape buffer), `readline_global` (TAB + Ctrl+R), `KEY_*=1000+` (:750) |
| 6.5 | 1106 | `cooked_readline()` (:1111) — drains kbhit; getline fallback when `g_term_saved==0` |
| 7 | 1146 | `ALL_COMMANDS` set **286** entries (:1152), `edit_dist`/`closest_cmd` (threshold 4), text-tool helpers |
| 8 | 1481 | v1/v2 games: snake, minesweeper, tictactoe, hangman, rps, 2048, tetris, pong, sudoku, flappy, ... |
| 11.5 | 3842 | v2 games/system/eggs/effects: othello…bingo, kernel/boot/netstat…, llm/rain/fire/stream… |
| 11.7 | 4784 | v3 engine helpers: `persist_save/load`, `vfs_escape/unescape`, `expand_globs`, `filter_source`, `script_subst`, `PKG_DB`+`g_installed_pkgs` |
| 11.8 | 4991 | v3 games/toys: `chess::` (legality+castling+promo, greedy-2ply AI), clicker/pet/fishing, pkg-gated figlet/nyancat/starwars/sheep, `cmd_news` |
| 11.9 | 5502 | v4 games/tools/effects/eggs: war, yahtzee, higherlower, artillery; bitwise/regex/ipcalc/utime; budget/keytest; snow/wave/starfield; eggs coffee/potato/banana/goose |
| 12 | 6078 | `main()`: theme/prompt peek → boot → persist_load → MOTD → `exec_segment` (:6231) + `run_one` (:8418) → REPL → `persist_save` on exit |

- `SYSTEM_USERS {"root","user","guest","admin"}` mutated by su/useradd/userdel, persisted via `U` lines.

## VFS — Where Agents Guess Wrong

- Backing store: `map<string, FSNode> file_system` local to `main`. **All keys end with `/`**, even files (`/geometry/jumper.gmd/`). `FSNode` ctor: dir→`rwxr-xr-x`, file→`rw-r--r--`, stamps `"%b %d %H:%M"`.
- Use helpers, never hand-roll paths: `resolve_user_path(arg, cdir)`, `resolved_path(fs,path)` (symlink chase ≤8), `has_traversal(path)` (any `..` → block), `vfs_read(path,fs,cdir)` → `optional<string>`.
- Ops: `mkdir` creates intermediate parents · `echo f c` first-space split, strips outer quotes · `rm` moves to `/trash/<epoch>_<name>` (guards `rm -rf /` → ☢) · `cp/mv` copy INTO existing dirs by basename; `-r` collects into vector before mutating · `cat` is multi-file · `chmod` expects literal 9-char mode · `ls -l` shows type+mode+size+stamp.
- `ls == ls /` easter 🐉 · `echo hello world` easter · `cat /dev/brain` easter.

## v3 Shell Engine — How Lines Execute

- REPL → `run_one(line)` (:8418): split unquoted `;` and `&&` markers → each part split on unquoted `|` → stages through `exec_segment(seg,capture,captured)` (:6231).
- `exec_segment`: trim → `< file` input redirect → `> >>` extraction → `expand_globs()` vs VFS keys → alias resolve once → giant dispatch chain. Returns false only on unknown command (drives `&&`; skip is `i += 3` — marker AND target).
- Pipes: stdout captured via `cout.rdbuf()` swap into ostringstream; carried to next stage through `g_stdin_buf` + `g_stdin_live`. Filters consume via `filter_source()` or `args.empty() && g_stdin_live`. **Never reset `g_stdin_*` at segment entry** — that broke pipes once.
- Scripts: `run x.nnsh args…` → per line strip `#comment` → `script_subst` (`$var $1-$9 $@`) → `set k=v` → one-line `if A == B then cmd` → else `run_one`. Interactive `$x` expansion applies to any line containing `$`.
- Persistence `~/.nonameos.vfs`: `NNOS3` header, `T theme`, `P escaped-prompt`, `A alias=v`, `U user`, `K pkg`, nodes `D/L/F`. Auto-saved on every exit; theme/prompt peeked BEFORE logo.

## Terminal / Input — Do Not Break

- `TerminalGuard` RAII: raw flags + `O_NONBLOCK`, restores on destruction; inactive (non-tty) → `cooked_readline` getline fallback keeps piping usable.
- SIGINT handler sets flag ONLY (never tcsetattr); main loop + readline poll it. SIGTSTP/SIGCONT toggle cooked/raw.
- `getkey()` reads at fd level + poll(); **never `getchar()`** (desyncs poll, splits escapes). Lone ESC stays pending 120ms; CSI/SS3/modified arrows/`~`-keys handled.
- Always drain `while(kbhit()) getkey();` before mode switches. Realtime loops drain on exit too.
- **Headless caveat:** without tty, fd blocks so `kbhit()` drains swallow piped input — interactive games are NOT headless-testable; text/chains/pipes ARE: `printf '%s\n' 'a | b' 'exit' | ./nonameos`.

## Command System

- Add a command ⇒ touch ALL of: `ALL_COMMANDS` (:1152), `help_text()` map, dispatch chain inside `exec_segment`, optional `manpages` map + `section(...)` line in `help`. Missing `ALL_COMMANDS` breaks `which`, fuzzy suggest, update-readme counts.
- Fuzzy: Levenshtein `edit_dist`, suggestion threshold 4 via `closest_cmd`.
- Aliases default `ll .. ttt`, user aliases persist (`A` lines). Prompt template tokens `%u %h %d %t %%` rendered each prompt with themed colors; duration `…ms` shown after >100ms.
- pkg gating pattern: branch checks `g_installed_pkgs.count("figlet")` else prints install hint (figlet/nyancat/starwars/sheep).

## Games — know the input mode

- Turn-based (`cooked_readline`): chess (`e2 e4`), othello, mastermind, blackjack, battleship, slots, maze, nim, buzz, bulls, twentyone, jumble, arith, bingo, minesweeper, tictactoe, hangman, rps, sudoku, wordle, quiz, trivia, adventure, guess, memory, connect4, lightsout, puzzle, typing, nummem, reaction; v4: war (ENTER draw), yahtzee (`1 4` reroll), higherlower (h/l), artillery (`45 80`).
- Realtime (`kbhit/getkey`): snake, tetris, pong, breakout, flappy, asciidash, clicker (SPACE click, 1-5 buy, q saves), simon, rain, fire, stream; v4 effects: snow, wave, starfield.
- Persistent progress lives in VFS under `/home/` (clicker.sav, pet.save, fishing.log, budget.save) — survives via session persistence.
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

## Gotchas

missing `-lpthread` · trailing-`/` VFS keys · `getchar()` in realtime · undrained input before cooked mode · missing `ALL_COMMANDS` entry · resetting pipe globals per-stage · `i+=2` vs `i+=3` in `&&` skip · editing README counts by hand instead of grepping `ALL_COMMANDS`.

## Workflow

- Branch `main` only. PR template checklist: style/tested/docs/no-warnings. Commit style: short imperative ("fix …", "add …"). Never commit binaries. No opencode.json present.

## References

- Executable truth: `Makefile:2` · POSIX includes `NoNameOS.cpp:53` · TerminalGuard `:463` · input `:727-1145` · `ALL_COMMANDS` `:1152` · persistence `:4784+` · main `:6078` · exec_segment `:6231` · run_one `:8418`.
- Docs: README (features/build/FAQ), CONTRIBUTING.md, `.github/PULL_REQUEST_TEMPLATE.md`.
