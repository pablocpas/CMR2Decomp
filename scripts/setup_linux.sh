#!/usr/bin/env bash
# One-shot setup of the matching environment on Debian/Ubuntu (cloud sessions,
# WSL, CI-like boxes): Wine, MSVC6 + SP3 compiler passes, the original EXE,
# reccmp, a first build/measure, and the fastcmp metadata.
#
#   scripts/setup_linux.sh            # idempotent; re-running skips done steps
#
# The toolchain lives OUTSIDE the repository (default ../msvc600): reccmp scans
# the whole repo as its source root, and stray headers there change the results.
# Afterwards:
#   export CMR2_MSVC_ROOT=<printed path> WINEPREFIX=<printed path>
#   python3 scripts/match.py --list            # what to work on
#   python3 scripts/match.py 0xADDR            # compile + diff + TU check, ~1 s
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TOOLS="${CMR2_TOOLS:-$(dirname "$ROOT")}"
MSVC="$TOOLS/msvc600/VC98"
export CMR2_MSVC_ROOT="$MSVC" WINEPREFIX="${WINEPREFIX:-$ROOT/scripts/wineprefix}" WINEDEBUG=-all
cd "$ROOT"

SUDO=""; [ "$(id -u)" -ne 0 ] && SUDO="sudo"
if ! command -v wine >/dev/null; then
  echo "== installing wine"
  $SUDO dpkg --add-architecture i386
  $SUDO apt-get update -qq
  DEBIAN_FRONTEND=noninteractive $SUDO apt-get install -y -qq --no-install-recommends \
    wine wine32:i386 wine64 p7zip-full cabextract >/dev/null
fi

git submodule update --init --recursive

if [ ! -f "$MSVC/Bin/CL.EXE" ]; then
  echo "== fetching MSVC 6.0"
  git clone -q https://github.com/itsmattkc/msvc600 "$TOOLS/msvc600"
  git -C "$TOOLS/msvc600" checkout -q 001c4bafdcf2ef4b474d693acccd35a91e848f40
fi
python3 scripts/fetch_vc6sp3.py --cache "$TOOLS/vc6sp3" --msvc-root "$MSVC" | tail -1

if [ ! -f cmr2bin/CMR2.exe ]; then
  echo "== fetching the original CMR2.exe"
  mkdir -p cmr2bin
  curl -fsSL -o cmr2bin/CMR2.exe \
    https://raw.githubusercontent.com/CMR2Decomp/SilentPatchCMR2/refs/heads/master/game-bin/CMR2.exe
fi
python3 -m pip install -q -r scripts/requirements.txt 2>&1 | grep -v "root user" || true

echo "== build + measure (about 2 minutes)"
python3 scripts/build.py | tail -1
[ -f reccmp-user.yml ] || reccmp-project detect --what original --search-path cmr2bin >/dev/null
[ -f reccmp-build.yml ] || reccmp-project detect --what recompiled --search-path build >/dev/null
python3 scripts/measure.py 2>&1 | tail -1
python3 scripts/prepare_fastcmp.py
# measure.py rewrites tracked reports; a fresh checkout should not look dirty
git diff --quiet -- CMR2PROGRESS/bytes.json || echo "note: CMR2PROGRESS differs from HEAD (see git diff)"

echo
echo "export CMR2_MSVC_ROOT=$MSVC WINEPREFIX=$WINEPREFIX"
