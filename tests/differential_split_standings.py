#!/usr/bin/env python3
"""Compare the complete three-row split HUD with original x86.

Usage: differential_split_standings.py entities.json [rebuilt.exe]
Game-mode queries, sprintf, name formatting and drawing are controlled leaves.
Split lookup and time conversion execute for real. Formatting a rank must not
change the car's split record or the arguments used by later rows.
"""
import itertools
import json
from pathlib import Path
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK
from matching_entities import entity_address


class SplitStandings(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {
            self.addr(0x411880): (0, lambda: self.result(self.compact)),
            self.addr(0x405dc0): (0, lambda: self.result(0)),
            self.addr(0x4054b0): (4, lambda: self.result(0)),
            self.addr(0x404f20): (0, lambda: self.result(self.allow_empty)),
            self.addr(0x405620): (0, self.format_rank),
            self.addr(0x415750): (16, self.format_name),
            self.addr(0x40b5b0): (8, lambda: self.result(self.name_width)),
            self.addr(0x40b880): (24, self.font),
            self.addr(0x4a5e40): (16, self.fill),
            self.addr(0x415bd0): (36, self.time_text),
        }

    def member(self, address, owner):
        return address if self.entities is None else entity_address(self.entities, address, owner)

    def result(self, value):
        self.u.reg_write(UC_X86_REG_EAX, value)

    def string(self, address):
        for length in range(260):
            if self.read(address + length, 1) == b'\0':
                return self.read(address, length)
        raise AssertionError('unterminated text')

    def format_rank(self):
        dest, fmt, rank = self.args(3)
        assert self.string(fmt) == b'%1d'
        text = str(rank).encode()
        self.u.mem_write(dest, text + b'\0')
        self.trace.append(('rank', rank, text))
        self.result(len(text))

    def format_name(self):
        position, split, short, kind = self.args(4)
        assert split == self.split, ('name split changed after formatting rank', split, self.split)
        self.trace.append(('name', position, split, short, kind))
        self.u.mem_write(self.addr(0x663b60), f'driver_{position}_{short}'.encode() + b'\0')

    def font(self):
        index, text, x, y, colour, flags = self.args(6)
        self.trace.append(('font', index, self.string(text), x & 0xffff, y & 0xffff,
                           self.read(colour, 4), flags))

    def time_text(self):
        args = list(self.args(9))
        args[5] = self.read(args[5], 4)
        self.trace.append(('time', args))

    def hook(self, u, address, size, data):
        if address in (self.addr(0x455cc0), self.addr(0x455d40)):
            position, split = self.args(2)
            assert split == self.split, ('time lookup split changed', split, self.split)
            assert 0 <= position < 16, ('invalid ranking position', position)
        super().hook(u, address, size, data)

    def run(self, car, compact, timing, split, positions, resolution, name_width, allow_empty):
        self.reset()
        self.compact, self.split = compact, split
        self.name_width, self.allow_empty = name_width, allow_empty
        self.put(self.addr(0x520b74), '<I', HEAP + 0x1000)
        self.put(HEAP + 0x1000, '<2I', *resolution)
        self.put(HEAP + 0x2000, '<4h', 17, 23, 320, 240)
        self.put(self.addr(0x542604), '<B', timing)
        self.put(self.addr(0x536c94), '<6i', *([-1] * 6))
        self.put(self.addr(0x536c94) + car * 12, '<3i', *positions)
        runtime = self.addr(0x536d14)
        self.u.mem_write(runtime, bytes([0xa5]) * 0x1ac)
        record = self.member(0x536dfc, 0x536d14) + car * 0x48
        self.put(record, '<2i', next((p for p in positions if p != -1), -1), split)
        ranks = self.addr(0x541f98)
        self.u.mem_write(ranks, bytes(0x200))
        self.put(ranks + 0x1f4 + split, '<b', 16)
        for position in range(16):
            self.put(ranks + 0x14 + split * 16 + position, '<b', 15 - position)
            self.put(ranks + 0x154 + split * 16 + position, '<b', position)
            self.put(self.addr(0x542198) + (split * 16 + position) * 4,
                     '<i', (30 + position + 7 * split) * 65536)
        immutable = [(runtime, 0x1ac), (ranks, 0x200), (HEAP + 0x1000, 8),
                     (HEAP + 0x2000, 8), (self.addr(0x536c94), 24)]
        before = [self.read(p, n) for p, n in immutable]
        self.invoke(0x414ed0, [car, HEAP + 0x2000])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff0c, 'ret 8 ABI'
        assert before == [self.read(p, n) for p, n in immutable], 'HUD modified timing inputs'
        return self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = SplitStandings(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = SplitStandings(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0
    positions = [(0, 1, 2), (7, 8, 9), (13, 14, 15), (-1, 0, 1), (-1, -1, -1)]
    for case in itertools.product(range(2), range(2), range(2), [1, 4, 8], positions,
                                  [(640, 480), (1280, 720)], [0, 999], range(2)):
        try:
            left, right = original.run(*case), rebuilt.run(*case)
            assert left == right, ('different HUD calls', left, right)
        except AssertionError as error:
            print('FAIL split standings', case, error)
            return 1
        count += 1
    print(count, 'split HUD cases: identical three-row text, times, drawing, input guards and ret 8')
    return 0


if __name__ == '__main__':
    sys.exit(main())
