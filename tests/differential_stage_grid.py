#!/usr/bin/env python3
"""Compare all seven original/rebuilt countdown pixel grids and draw calls.

Usage: differential_stage_grid.py entities.json [rebuilt.exe]
The complete grid routine runs, with only viewport mode and FillRect mocked.
The grids come independently from each executable, never from shared fixtures.
"""
import itertools
import json
from pathlib import Path
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP


class Grid(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {
            self.addr(0x4a5e40): (16, self.fill),
            self.addr(0x411880): (0, self.mode),
        }

    def mode(self):
        self.u.reg_write(UC_X86_REG_EAX, self.split)

    def run(self, resolution, pattern, split):
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, bytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        self.put(self.addr(0x520b74), '<I', HEAP + 0x1000)
        self.put(HEAP + 0x1000, '<2I', *resolution)
        sp = STACK + 0xff00
        self.put(sp, '<3I', STOP, 0, pattern)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace, self.split = [], split
        self.u.emu_start(self.addr(0x477f70), STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP
        return self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Grid(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Grid(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = 0
    for args in itertools.product([(640, 480), (800, 600), (1024, 768)], range(7), [0, 1]):
        a, b = original.run(*args), rebuilt.run(*args)
        if a != b:
            print('FAIL countdown pattern:', args, 'draw calls:', len(a), len(b))
            for n, pair in enumerate(itertools.zip_longest(a, b)):
                if pair[0] != pair[1]:
                    print('call', n, 'original:', pair[0], 'rebuilt:', pair[1])
                    break
            return 1
        cases += 1
    print(f'{cases} countdown cases: all seven pixel patterns, rectangles and colours identical')
    return 0


if __name__ == '__main__':
    sys.exit(main())
