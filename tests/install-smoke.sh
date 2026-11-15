#!/data/data/com.termux/files/usr/bin/env bash
# Installer CI: install the built binary, rerun, then uninstall.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="${TMPDIR:-/tmp}/nonameos-install-smoke.$$"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/home" "$TMP/state" "$TMP/prefix"
export HOME="$TMP/home" XDG_STATE_HOME="$TMP/state"
bash "$ROOT/install.sh" --no-build --no-menu --prefix "$TMP/prefix" >/dev/null
test -x "$TMP/prefix/bin/nonameos"
test -f "$XDG_STATE_HOME/nonameos/installed.tsv"
bash "$ROOT/install.sh" --no-build --no-menu --prefix "$TMP/prefix" >/dev/null
bash "$ROOT/install.sh" --uninstall --yes >/dev/null
test ! -e "$TMP/prefix/bin/nonameos"
test ! -e "$XDG_STATE_HOME/nonameos/installed.tsv"
echo "installer smoke OK"
