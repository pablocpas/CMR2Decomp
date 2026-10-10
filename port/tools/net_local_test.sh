#!/usr/bin/env bash
# Two OpenCMR2 instances on this machine, to try network play: host a game
# in one window and join it from the other.
#
#   tools/net_local_test.sh [--gdb] [--size WxH] [--fresh] [--data DIR]
#
# Each instance gets its own user directory (settings, saves, log) under
# $NET_TEST_DIR (default /tmp/opencmr2-net/A and B), copied from yours on the
# first run (--fresh copies again), windowed at --size (default 1024x768).
# The first instance hosts on the usual UDP port; the second, finding it
# taken, uses another one and finds the first by broadcast.
# Both keep running when they lose the focus (video.run_in_background).
# --gdb runs each under gdb and prints a backtrace if it crashes.
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BIN=${OPENCMR2_BIN:-$ROOT/build/linux-x86/src/opencmr2}
SOURCE=${XDG_DATA_HOME:-$HOME/.local/share}/OpenCMR2/OpenCMR2
WORK=${NET_TEST_DIR:-${TMPDIR:-/tmp}/opencmr2-net}
SIZE=1024x768
GDB=0
FRESH=0
DATA=

while [ $# -gt 0 ]; do
    case "$1" in
    --gdb) GDB=1 ;;
    --fresh) FRESH=1 ;;
    --size) SIZE=$2; shift ;;
    --data) DATA=$2; shift ;;
    -h|--help) sed -n '2,12p' "$0"; exit 0 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
    shift
done
[ -x "$BIN" ] || { echo "no executable at $BIN (build first, or set OPENCMR2_BIN)" >&2; exit 1; }
WIDTH=${SIZE%x*}
HEIGHT=${SIZE#*x}

# Copies the settings and configuration, then makes the copy windowed.
prepare() {
    local dir=$1
    if [ "$FRESH" = 1 ] || [ ! -e "$dir/opencmr2.ini" ]; then
        rm -rf "$dir"
        mkdir -p "$dir"
        [ -f "$SOURCE/opencmr2.ini" ] && cp "$SOURCE/opencmr2.ini" "$dir/"
        [ -d "$SOURCE/Configuration" ] && cp -r "$SOURCE/Configuration" "$dir/"
    fi
    python3 - "$dir" "$WIDTH" "$HEIGHT" "$DATA" <<'EOF'
import os, re, struct, sys
dir, width, height, data = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]

def set_key(lines, section, key, value):
    out, current, done = [], None, False
    for line in lines:
        m = re.match(r'\s*\[(\w+)\]', line)
        if m:
            if current == section and not done:
                out.append(f'{key} = {value}\n')
                done = True
            current = m.group(1).lower()
        elif current == section and re.match(rf'\s*{key}\s*=', line):
            line, done = f'{key} = {value}\n', True
        out.append(line)
    if not done:
        if current != section:
            out.append(f'\n[{section}]\n')
        out.append(f'{key} = {value}\n')
    return out

ini = os.path.join(dir, 'opencmr2.ini')
lines = open(ini).readlines() if os.path.exists(ini) else []
lines = set_key(lines, 'video', 'fullscreen', '0')
lines = set_key(lines, 'video', 'run_in_background', '1')   # both windows keep running
# Without vsync: a covered window would otherwise stop getting frames from
# the compositor (Wayland) and stall.
lines = set_key(lines, 'video', 'vsync', '0')
if data:
    lines = set_key(lines, 'paths', 'data', data)
open(ini, 'w').writelines(lines)

# The game's own resolution and fullscreen bit (GameInfo.rcf).
rcf = os.path.join(dir, 'Configuration', 'GameInfo.rcf')
if os.path.exists(rcf):
    b = bytearray(open(rcf, 'rb').read())
    struct.pack_into('<III', b, 0x24, width, height, 32)
    flags = struct.unpack_from('<I', b, 0x30)[0] & ~1
    struct.pack_into('<I', b, 0x30, flags)
    open(rcf, 'wb').write(b)
EOF
}

run() {
    local name=$1
    local home=$WORK/$name
    prepare "$home/OpenCMR2/OpenCMR2"
    if [ "$GDB" = 1 ]; then
        XDG_DATA_HOME=$home gdb -q -batch -ex run -ex 'bt 25' --args "$BIN" 2>&1 | sed -u "s/^/[$name] /"
    else
        XDG_DATA_HOME=$home "$BIN" 2>&1 | sed -u "s/^/[$name] /"
    fi
}

echo "user directories: $WORK/A, $WORK/B (logs: .../OpenCMR2/OpenCMR2/opencmr2.log)"
run A &
first=$!
sleep 2     # the first one takes the default port
run B &
second=$!
trap 'kill $first $second 2>/dev/null' INT TERM
wait
