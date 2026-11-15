#!/data/data/com.termux/files/usr/bin/env bash
# Build and install NoNameOS without root, with a recorded uninstall path.
set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PREFIX_DIR="${PREFIX:-$HOME/.local}"
NO_BUILD=0
DRY=0
YES=0
UNINSTALL=0
FORCE=0
MENU=1
STATE_DIR="${XDG_STATE_HOME:-$HOME/.local/state}/nonameos"
MANIFEST="$STATE_DIR/installed.tsv"

usage() {
    cat <<EOF
Usage: ./install.sh [options]

Install:
  --prefix DIR   install prefix (default: \$PREFIX, else ~/.local)
  --no-build     install the existing binary without running make
  --dry-run      print what would change

Uninstall:
  --uninstall    remove the installed binary recorded by this installer
  --force        remove it even if it changed since install
  --yes          skip the uninstall confirmation
  --no-menu      never show the interactive fzf menu

  -h, --help     show this help

Installs nonameos -> <prefix>/bin/nonameos
EOF
}

die() { printf 'error: %s\n' "$*" >&2; exit 1; }
info() { printf '\033[38;2;120;200;255m[nonameos]\033[0m %s\n' "$*"; }
warn() { printf '\033[38;2;230;170;70m[nonameos]\033[0m %s\n' "$*" >&2; }
have() { command -v "$1" >/dev/null 2>&1; }

while [ "$#" -gt 0 ]; do
    case "$1" in
        --prefix) shift; [ "$#" -gt 0 ] || die "--prefix needs a directory"; PREFIX_DIR="$1" ;;
        --prefix=*) PREFIX_DIR="${1#*=}" ;;
        --no-build) NO_BUILD=1 ;;
        --dry-run) DRY=1 ;;
        --uninstall) UNINSTALL=1 ;;
        --force) FORCE=1 ;;
        --yes|-y) YES=1 ;;
        --no-menu) MENU=0 ;;
        -h|--help) usage; exit 0 ;;
        *) die "unknown option: $1 (try --help)" ;;
    esac
    shift
done

case "$PREFIX_DIR" in
    /*) ;;
    *) PREFIX_DIR="$PWD/$PREFIX_DIR" ;;
esac
BIN_DIR="$PREFIX_DIR/bin"
TARGET="$BIN_DIR/nonameos"

hash_file() {
    if have sha256sum; then sha256sum "$1" | cut -d' ' -f1
    elif have shasum; then shasum -a 256 "$1" | cut -d' ' -f1
    elif have cksum; then cksum "$1" | awk '{print $1 "-" $2}'
    else printf 'unknown'
    fi
}

if [ "$UNINSTALL" = "1" ]; then
    [ -f "$MANIFEST" ] || { info "no installed binary recorded"; exit 0; }
    if [ "$YES" = "0" ] && [ "$DRY" = "0" ]; then
        if [ -t 0 ]; then
            printf 'Remove the NoNameOS binary recorded in %s? [y/N] ' "$MANIFEST"
            read -r reply
            case "$reply" in [Yy]*) ;; *) echo "Canceled."; exit 0 ;; esac
        else
            die "use --yes with --uninstall in a non-interactive shell"
        fi
    fi
    kept=0
    while IFS=$'\t' read -r path old_hash; do
        [ -n "$path" ] || continue
        [ -e "$path" ] || continue
        if [ "$FORCE" = "0" ] && [ "$(hash_file "$path")" != "$old_hash" ]; then
            warn "keeping modified binary: $path (use --force to remove)"
            kept=$((kept + 1)); continue
        fi
        if [ "$DRY" = "1" ]; then
            printf '  would remove: %s\n' "$path"
        else
            rm -f "$path"
            printf '  removed: %s\n' "$path"
        fi
    done < "$MANIFEST"
    if [ "$DRY" = "0" ] && [ "$kept" -eq 0 ]; then
        rm -f "$MANIFEST"
        rmdir "$STATE_DIR" 2>/dev/null || true
    fi
    info "uninstall complete"
    exit 0
fi

if [ "$DRY" = "1" ]; then
    printf '  would install %s -> %s\n' "$REPO_DIR/nonameos" "$TARGET"
    exit 0
fi

if [ "$MENU" = "1" ] && [ "$NO_BUILD" = "0" ] && [ -t 0 ] && [ -t 1 ] && have fzf; then
    choice=$(printf 'Build and install\nInstall existing binary\nChange install prefix\nCancel\n' \
        | fzf --reverse --height=40% --border --prompt='NoNameOS install> ') || choice="Cancel"
    case "$choice" in
        "Build and install") ;;
        "Install existing binary") NO_BUILD=1 ;;
        "Change install prefix")
            printf 'Install prefix [%s]: ' "$PREFIX_DIR"
            read -r new_prefix
            if [ -n "$new_prefix" ]; then
                PREFIX_DIR="$new_prefix"
                case "$PREFIX_DIR" in /*) ;; *) PREFIX_DIR="$PWD/$PREFIX_DIR" ;; esac
                BIN_DIR="$PREFIX_DIR/bin"; TARGET="$BIN_DIR/nonameos"
            fi
            ;;
        *) echo "Canceled."; exit 0 ;;
    esac
fi

if [ "$NO_BUILD" = "0" ]; then
    if [ ! -x "$REPO_DIR/nonameos" ] || [ "$REPO_DIR/NoNameOS.cpp" -nt "$REPO_DIR/nonameos" ]; then
        have make || die "make is required (or use --no-build with an existing binary)"
        info "building with make"
        make -C "$REPO_DIR"
    else
        info "binary is up to date; skipping build"
    fi
fi
[ -x "$REPO_DIR/nonameos" ] || die "missing binary; run without --no-build"

mkdir -p "$BIN_DIR" || die "cannot create $BIN_DIR"
if [ -f "$TARGET" ] && cmp -s "$REPO_DIR/nonameos" "$TARGET"; then
    info "unchanged -> $TARGET"
else
    if [ -e "$TARGET" ]; then
        bak="$TARGET.bak-$(date +%Y%m%d-%H%M%S)-$$"
        cp -p "$TARGET" "$bak"
        printf '  backup: %s\n' "$bak"
    fi
    install -m 0755 "$REPO_DIR/nonameos" "$TARGET"
    info "installed -> $TARGET"
fi
mkdir -p "$STATE_DIR"
printf '%s\t%s\n' "$TARGET" "$(hash_file "$TARGET")" > "$MANIFEST"
case ":$PATH:" in
    *":$BIN_DIR:"*) ;;
    *) printf 'Add %s to PATH to run it as nonameos.\n' "$BIN_DIR" ;;
esac
