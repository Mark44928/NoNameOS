#!/data/data/com.termux/files/usr/bin/env bash
# tests/smoke.sh — pipe-driven REPL smoke tests for NoNameOS.
#
# The REPL accepts commands on stdin (cooked_readline), so behavior is testable
# without a TTY. Each assertion pipes a session and greps for a needle.
# HOME is redirected to a scratch dir because the VFS persists to
# ~/.nonameos.vfs and a leaked file would make runs order-dependent.
set -euo pipefail
cd "$(dirname "$0")/.."

# Prefer the caller's TMPDIR, then the Termux default, then any writable
# temp dir — CI runners have neither TMPDIR set nor the Termux path.
if [ -n "${TMPDIR:-}" ] && [ -d "$TMPDIR" ] && [ -w "$TMPDIR" ]; then
    :
elif [ -d /data/data/com.termux/files/usr/tmp ] && [ -w /data/data/com.termux/files/usr/tmp ]; then
    TMPDIR=/data/data/com.termux/files/usr/tmp
else
    TMPDIR=$(mktemp -d 2>/dev/null || echo ".")
fi
export TMPDIR
mkdir -p "$TMPDIR"

BIN=./nonameos
if [[ ! -x "$BIN" ]]; then
    echo "smoke: $BIN not built — run make first" >&2
    exit 1
fi

PASS=0
FAIL=0

strip() { sed 's/\x1b\[[0-9;]*[A-Za-z]//g'; }

# assert_has <label> <input> <needle>
assert_has() {
    local label="$1" input="$2" needle="$3" out
    out=$(printf '%b' "${input}\nexit\n" | HOME="$TMPDIR/smoke-home" timeout 20 "$BIN" 2>&1 | strip || true)
    if printf '%s\n' "$out" | grep -qF -- "$needle"; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1))
        printf 'smoke FAIL: %s (want: %s)\n' "$label" "$needle"
    fi
}

# assert_lacks <label> <input> <needle>
assert_lacks() {
    local label="$1" input="$2" needle="$3" out
    out=$(printf '%b' "${input}\nexit\n" | HOME="$TMPDIR/smoke-home" timeout 20 "$BIN" 2>&1 | strip || true)
    if printf '%s\n' "$out" | grep -qF -- "$needle"; then
        FAIL=$((FAIL + 1))
        printf 'smoke FAIL: %s (unexpected: %s)\n' "$label" "$needle"
    else
        PASS=$((PASS + 1))
    fi
}

rm -rf "$TMPDIR/smoke-home"
mkdir -p "$TMPDIR/smoke-home"

# --- basics ---
assert_has "version banner"    "version"                     "NoNameOS v4.2.0"
assert_has "unknown command"   "definitelyNotACmd"           "command not found: definitelyNotACmd"

# --- v4.1.0 commands (must be dispatched, not "command not found") ---
assert_has "roulette"          "roulette"                    "Russian Roulette"
assert_has "slotmachine"       "slotmachine"                 "Slot Machine"
assert_has "towers board"      "towers"                      "TOWERS OF HANOI"
assert_has "poker"             "poker"                       "FIVE-CARD DRAW POKER"
assert_has "darts"             "darts"                       "You scored"
assert_has "bowling"           "bowling"                     "You knocked down"
assert_has "emoticon mood"     "emoticon love"               "<3"
assert_has "asciiart"          "asciiart"                    "ASCII Art Generator"
assert_has "qr usage"          "qr"                          "Usage: qr <text>"
assert_has "qr text"           "qr https://github.com"       "QR Code Generator"
assert_has "barcode"           "barcode abc"                 "Barcode Generator"
assert_lacks "no dead solitaire" "help"                      "solitaire"

# --- shell plumbing ---
assert_has "write and cat"     "echo /p1 hello && cat /p1"   "hello"
# cat must be verbatim in pipes: "a b c" is 1 line, 3 words, 5 chars — the
# injected trailing newline used to make this 6.
assert_has "pipe into wc"      "echo /p2 a b c && cat /p2 | wc" "1  3  5"
assert_has "piped digest matches file" "echo /p3 hi there && sha256sum /p3 && cat /p3 | sha256sum" "9b96a1fe1d548cbbc960cc6a0286668fd74a763667b06366fb2324269fcabaa4"
assert_has "glob"              "ls /e*"                      "geometry"
assert_has "vfs mkdir"         "mkdir /smoke && ls /"        "smoke"
assert_has "help lists new"    "help"                       "towers"

# --- games that existed before must still dispatch ---
assert_has "rps"               "rps"                         "ROCK PAPER SCISSORS"
assert_has "blackjack"         "blackjack"                   "BLACKJACK"
assert_lacks "crypto stays"    "crypto"                      "command not found"


# --- v4.2.0: sha256sum + stat ---
hash_out=$(printf 'echo /s.txt hello world\nsha256sum /s.txt\nexit\n' | HOME="$TMPDIR/smoke-home" timeout 20 "$BIN" 2>&1 | strip)
if printf '%s\n' "$hash_out" | grep -q 'b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9'; then
  PASS=$((PASS + 1))
else
  FAIL=$((FAIL + 1)); printf 'smoke FAIL: sha256sum digest mismatch\n'
fi
assert_has "stat file"   "echo /s2.txt a b c\nstat /s2.txt"   "Words:"
assert_has "stat dir"    "mkdir /statdir\nstat /statdir"      "Type:   directory"
assert_has "stat missing" "stat /nope"                        "no such file"
assert_has "boot tip"    "version"                            "tip:"
assert_has "v4.2.0 banner" "version"                         "NoNameOS v4.2.0"

echo
echo "smoke: $PASS passed, $FAIL failed"
[[ $FAIL -eq 0 ]]
