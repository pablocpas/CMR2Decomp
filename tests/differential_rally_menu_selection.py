#!/usr/bin/env python3
"""Whole rally panel against original, with actual menu/game queries.

Usage: differential_rally_menu_selection.py entities.json [rebuilt.exe]
Text, formatting, rendering, weather provider and final preview are controlled
leaves. Their ordered arguments, text, positions and colours must agree.
Distinct menu tags 0/1 detect the old wrong preview selection independently.
This does not validate the implementations of those controlled leaves.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK


class RallyPanel(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {
            self.addr(0x4a3c60): (4, self.text),
            self.addr(0x405620): (0, self.format),
            self.addr(0x40b880): (24, self.font),
            self.addr(0x407520): (4, self.weather),
            self.addr(0x50a880): (8, self.preview),
        }

    def string(self, address):
        data = bytearray()
        for i in range(260):
            value = self.read(address + i, 1)
            if value == b'\0':
                return bytes(data)
            data.extend(value)
        raise AssertionError('unterminated string')

    def text(self):
        index, = self.args(1)
        pointer = HEAP + 0x7000 + index * 32
        self.u.mem_write(pointer, f'text_{index}'.encode() + b'\0')
        self.trace.append(('string', index))
        self.u.reg_write(UC_X86_REG_EAX, pointer)

    def format(self):
        dest, fmt = self.args(2)
        pattern = self.string(fmt)
        if b'%s' in pattern:
            value = self.string(self.args(3)[2])
            result = pattern % value
        elif b'%d' in pattern:
            value = self.args(3)[2]
            value = value if value < 0x80000000 else value - 0x100000000
            result = pattern % value
        else:
            assert b'%' not in pattern, pattern
            value = None
            result = pattern
        assert dest == self.addr(0x663b60) and len(result) < 260
        self.u.mem_write(dest, result + b'\0')
        self.trace.append(('format', pattern, value))
        self.u.reg_write(UC_X86_REG_EAX, len(result))

    def font(self):
        index, text, x, y, colour, flags = self.args(6)
        self.trace.append(('font', index, self.string(text), x, y,
                           self.read(colour, 4), flags))

    def weather(self):
        selected, = self.args(1)
        assert selected == self.selected, ('weather selection', selected, self.selected)
        self.trace.append(('weather', selected))
        self.u.reg_write(UC_X86_REG_EAX, HEAP + 0x5000)

    def preview(self):
        selected, kind = self.args(2)
        self.trace.append(('preview', selected, kind))
        self.u.reg_write(UC_X86_REG_EAX, 0)

    def run(self, resolution, stage, mode, country, seed, selected):
        self.reset()
        r = random.Random(seed)
        self.selected = selected
        self.put(self.addr(0x520b74), '<I', HEAP + 0x1000)
        self.put(HEAP + 0x1000, '<2I', *resolution)
        self.put(self.addr(0x52afa0) + 0x14, '<I', mode << 3)
        self.put(self.addr(0x52f2ac), '<I', country | stage << 5)
        self.put(HEAP + 0x5000, '<I', seed % 5)
        menu = self.addr(0x82b668)
        self.u.mem_write(menu, r.randbytes(0x1e0))
        tags = [0, 1, 2, 3, 4, 5]
        r.shuffle(tags)
        self.put(menu + 6, '<b', len(tags))
        for i, tag in enumerate(tags):
            self.put(menu + 0x14 + i * 20 + 8, '<h', tag)
            self.put(menu + 0x14 + i * 20 + 11, '<B',
                     selected if tag == 0 else (selected + tag) % 8)
        immutable = [(menu, 0x1e0), (HEAP + 0x1000, 8),
                     (self.addr(0x52afa0) + 0x14, 4),
                     (self.addr(0x52f2ac), 4), (HEAP + 0x5000, 4)]
        before = [self.read(p, n) for p, n in immutable]
        dest = self.addr(0x663b60)
        self.u.mem_write(dest - 16, bytes([0xa5]) * 292)
        self.invoke(0x50c130, [seed * 0x1234567 & 0xffffffff])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff08, 'ret 4 ABI'
        assert before == [self.read(p, n) for p, n in immutable], 'input changed'
        assert self.read(dest - 16, 16) == bytes([0xa5]) * 16
        assert self.read(dest + 260, 16) == bytes([0xa5]) * 16
        fonts = [call for call in self.trace if call[0] == 'font']
        count = 1 if stage == 10 or mode in (2, 3, 8, 9, 10) else 2
        assert len(fonts) == count + 4, ('stage count', len(fonts), count)
        for i in range(count):
            assert fonts[3 + i][2] == str(stage + i + 1).encode()
        return self.trace, self.string(dest)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = RallyPanel(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = RallyPanel(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0
    for case in itertools.product([(640, 480), (1024, 768)], [0, 9, 10],
                                  [0, 2, 3, 8, 9, 10, 12], [0, 7], range(3), [0, 1, 2]):
        a, b = original.run(*case), rebuilt.run(*case)
        assert a[0][-1] == ('preview', case[-1], 1), 'original independent selection model'
        if a != b:
            print('FAIL rally panel', case)
            for x, y in itertools.zip_longest(a[0], b[0]):
                if x != y:
                    print('original:', x, 'rebuilt:', y)
            return 1
        assert b[0][-1] == ('preview', case[-1], 1), 'rebuilt independent selection model'
        count += 1
    print(count, 'rally panel cases: identical text/render/selection traces, immutable inputs, buffer guards and ret 4')
    return 0


if __name__ == '__main__':
    sys.exit(main())
