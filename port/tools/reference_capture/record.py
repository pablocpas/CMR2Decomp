#!/usr/bin/env python3
"""Run one prepared Windows fixture in a disposable Wine prefix."""
import argparse
import hashlib
import json
import os
import re
from pathlib import Path
import subprocess
import sys


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('prepared', type=Path)
    p.add_argument('side', choices=['original', 'decomp'])
    p.add_argument('--wineprefix', type=Path, required=True)
    p.add_argument('--timeout', type=int, default=180)
    p.add_argument('--fps', type=int, default=60)
    args = p.parse_args()
    if not 10 <= args.fps <= 1000:
        raise ValueError('FPS must be in 10..1000')
    output, prefix = args.prepared.resolve(), args.wineprefix.resolve()
    if not (prefix.is_relative_to(Path('/tmp')) or prefix.is_relative_to(output)):
        raise ValueError('use a disposable Wine prefix under /tmp or the prepared output')
    if not (prefix/'system.reg').exists():
        raise ValueError('initialize this disposable win32 prefix with wineboot first')
    run = output/args.side
    manifest = json.loads((output/'manifest.json').read_text())
    expected = manifest['runs'][args.side]
    for name, checksum in {'CMR2.exe': expected['executable_sha256'], 'capture.map': expected['address_map_sha256'],
                           'SPCMR2.dll': manifest['observer_sha256'], **expected.get('runtime_sha256', {})}.items():
        if sha(run/name) != checksum:
            raise ValueError(f'prepared fixture changed: {name}')
    if args.side == 'original' and sha(run/'SPCMR2_sp.dll') != manifest['silentpatch_sha256']:
        raise ValueError('SilentPatch DLL changed')
    if (run/'state.bin').exists() or (run/'capture.log').exists():
        raise ValueError('capture already exists; prepare a new output for a repeat')
    env = dict(os.environ, WINEPREFIX=str(prefix), WINEDEBUG='-all',
               WINEDLLOVERRIDES='ddraw,d3dimm=b;msvcrt,msvcrt40,msadp32.acm=n,b')
    env.pop('WAYLAND_DISPLAY', None)
    env.pop('CMR_CAPTURE_DECOMP', None)
    env.pop('CMR_CAPTURE_TICKS', None)
    env.pop('CMR_CAPTURE_FPS', None)
    env['CMR_CAPTURE_FPS'] = str(args.fps)
    def registry(key, name, value):
        subprocess.run(['wine', 'reg', 'add', key, '/v', name, '/d', value, '/f'], env=env, check=True, stdout=subprocess.DEVNULL)
    registry(r'HKCU\Software\Wine\AppDefaults\CMR2.exe', 'Version', 'win2k')
    if args.side == 'decomp':
        # Standalone decompilation relocates code; applying the original-address
        # SilentPatch would corrupt it. This side is an additional control.
        print('Standalone decompilation control: no SilentPatch; original side is the patched reference.', flush=True)
        env['CMR_CAPTURE_DECOMP'] = '1'
        path = 'Z:' + str(run).replace('/', '\\')
        if len(path) >= 100:
            raise ValueError('legacy registry path exceeds its 100-byte buffer; choose a shorter output path')
        for key, value in {'Sku_Type': 'EUROPE', 'Language': 'English', 'Install_Version': 'Full',
                           'Game_HDPath': path, 'Game_CDPath': path}.items():
            registry(r'HKLM\Software\Codemasters\Colin McRae Rally 2', key, value)
    with (run/'wine.log').open('x') as log:
        result = subprocess.run(['timeout', str(args.timeout)+'s', 'wine', 'CMR2.exe'], cwd=run, env=env, stdout=log, stderr=subprocess.STDOUT)
    capture_log = (run/'capture.log').read_text()
    completed = re.search(r'COMPLETE 500 ticks \((\d+) driving\)', capture_log)
    if result.returncode or not completed or not 0 < int(completed.group(1)) <= 500 or 'scenario country=0 stage=0 count=1' not in capture_log:
        raise ValueError(f'incomplete recording (exit {result.returncode}); inspect {run / "capture.log"} and wine.log')
    print(f'Complete recording: {run / "state.bin"} SHA256 {sha(run/"state.bin")}')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        print(f'record failed: {error}', file=sys.stderr)
        sys.exit(2)
