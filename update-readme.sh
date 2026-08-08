#!/bin/bash
# Auto-update README.md stats from NoNameOS.cpp source
# Run: ./update-readme.sh  or  make readme
set -euo pipefail

SRC="NoNameOS.cpp"
README="README.md"

if [[ ! -f "$SRC" ]]; then echo "Error: $SRC not found"; exit 1; fi
if [[ ! -f "$README" ]]; then echo "Error: $README not found"; exit 1; fi

# --- Extract stats from source ---
LINES=$(wc -l < "$SRC" | tr -d ' ')
VERSION=$(grep -oP 'const string VERSION = "\K[^"]+' "$SRC" | head -1)

# Pull the ALL_COMMANDS set (first 20 lines covers it)
CMDS_RAW=$(sed -n '/^const set<string> ALL_COMMANDS/,/};/p' "$SRC" | head -20)
TOTAL=$(echo "$CMDS_RAW" | grep -oP '"[^"]+"' | wc -l | tr -d ' ')

# Games list (must match the ALL_COMMANDS entries exactly)
GAMES="snake tetris pong sudoku flappy minesweeper tictactoe hangman rps adventure guess 2048 typing reaction nummem memory connect4 lightsout puzzle breakout whack wordle quiz trivia"
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

COMMANDS=$((TOTAL - GAME_COUNT - EGG_COUNT))

# Format line count with commas for display
LINES_FMT=$(printf "%'d" "$LINES")

echo "Lines:     $LINES_FMT"
echo "Version:   $VERSION"
echo "Commands:  $COMMANDS  Games: $GAME_COUNT  Eggs: $EGG_COUNT  Total: $TOTAL"

# --- Update README placeholders ---
sed -i "s/<!--STAT:LINES-->/$LINES/g"          "$README"
sed -i "s/<!--STAT:LINES_FMT-->/$LINES_FMT/g"  "$README"
sed -i "s/<!--STAT:VERSION-->/$VERSION/g"      "$README"
sed -i "s/<!--STAT:COMMANDS-->/$COMMANDS/g"    "$README"
sed -i "s/<!--STAT:GAMES-->/$GAME_COUNT/g"     "$README"
sed -i "s/<!--STAT:TOTAL-->/$TOTAL/g"          "$README"
sed -i "s/<!--STAT:EGGS-->/$EGG_COUNT/g"       "$README"

echo "README.md updated."
