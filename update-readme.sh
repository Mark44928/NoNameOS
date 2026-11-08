#!/data/data/com.termux/files/usr/bin/env bash
# Auto-update README.md stats from NoNameOS.cpp source.
# Run: ./update-readme.sh  or  make readme
#
# Idempotent by design: it patches the numeric badges and version strings in
# place, so it can be re-run after every change (the previous version needed
# <!--STAT:*--> anchors that were consumed on first run and never re-added).
set -euo pipefail

SRC="NoNameOS.cpp"
README="README.md"

if [[ ! -f "$SRC" ]]; then echo "Error: $SRC not found"; exit 1; fi
if [[ ! -f "$README" ]]; then echo "Error: $README not found"; exit 1; fi

# --- Extract stats from source ---
LINES=$(wc -l < "$SRC" | tr -d ' ')
VERSION=$(grep -oP 'const string VERSION = "\K[^"]+' "$SRC" | head -1)

# "commands" means commands that actually dispatch. The ALL_COMMANDS set also
# contains reserved names (future commands); counting those would advertise
# commands that print "command not found".
CMDS_RAW=$(sed -n '/^const set<string> ALL_COMMANDS/,/};/p' "$SRC")
TOTAL=$(echo "$CMDS_RAW" | grep -oP '"[^"]+"' | wc -l | tr -d ' ')
COMMANDS=$(grep -oP 'cmd == "\K[^"]+' "$SRC" | sort -u | wc -l | tr -d ' ')

# Games list (must match the ALL_COMMANDS entries exactly)
GAMES="snake tetris pong sudoku flappy minesweeper tictactoe ttt hangman rps adventure guess 2048 typing reaction nummem memory connect4 lightsout puzzle breakout whack wordle quiz trivia othello mastermind simon blackjack battleship slots maze nim buzz bulls twentyone jumble arith bingo chess clicker pet fishing play war yahtzee higherlower artillery roulette slotmachine towers poker darts bowling"
GAME_COUNT=0
for g in $GAMES; do
    echo "$CMDS_RAW" | grep -q "\"$g\"" && GAME_COUNT=$((GAME_COUNT + 1))
done

# Easter eggs list
EGGS="42 meaning life konami hack hacker rickroll rick beep bell yes-master yes-sir glhf gg lenny shrug tableflip unflip dealwithit disco dance loading wait sandwich open"
EGG_COUNT=0
for e in $EGGS; do
    echo "$CMDS_RAW" | grep -q "\"$e\"" && EGG_COUNT=$((EGG_COUNT + 1))
done

LINES_FMT=$(printf "%'d" "$LINES")

echo "Lines:     $LINES_FMT"
echo "Version:   $VERSION"
echo "Commands:  $COMMANDS dispatched (of $TOTAL reserved names)"
echo "Games:     $GAME_COUNT  Eggs: $EGG_COUNT"

# --- Update the README (regex patches, safe to re-run) ---
sed -i -E "s|/badge/version-v[0-9.]+-|/badge/version-${VERSION}-|" "$README"
sed -i -E "s|/badge/[0-9]+-lines_of_code|/badge/${LINES}-lines_of_code|" "$README"
sed -i -E "s|/badge/[0-9]+-commands|/badge/${COMMANDS}-commands|" "$README"
sed -i -E "s|/badge/[0-9]+-games|/badge/${GAME_COUNT}-games|" "$README"
sed -i -E "s|Current v[0-9.]+ adds|Current ${VERSION} adds|" "$README"
sed -i -E "s|NoNameOS v[0-9.]+ booted successfully|NoNameOS ${VERSION} booted successfully|" "$README"
sed -i -E "s|^(\s+)v[0-9.]+ · Pure C\+\+ OS Simulation$|\1${VERSION} · Pure C++ OS Simulation|" "$README"

echo "README.md updated."
