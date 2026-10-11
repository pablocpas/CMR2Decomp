#!/usr/bin/env python3
"""Original/rebuilt variadic result rows: text, separators, guards and cdecl ABI."""
import itertools
import json
from pathlib import Path
import random
import struct
import sys
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ESI,
                              UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP)
from differential_stage_lighting import Lighting, ROOT, HEAP, STACK

SAVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def lower(text):
    special = {126: 163, 36: 169, 42: 174, 124: 177, 247: 188,
               211: 243, 123: 189, 125: 190, 153: 215}
    return bytes(special.get(c, c + 32 if 65 <= c <= 90 or 192 <= c <= 223 else c)
                 for c in text)


def trunc(n, d):
    return (-1 if n < 0 else 1) * (abs(n) // d)


class Rows(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        # Case conversion executes for real, including the Polish character map.
        self.callbacks = {self.addr(0x405620): (0, self.copy),
                          self.addr(0x40b880): (24, self.draw),
                          self.addr(0x40b5b0): (8, self.width),
                          self.addr(0x4a5e40): (16, self.bar)}

    def string(self, pointer):
        return self.read(pointer, 260).split(b'\0', 1)[0]

    def copy(self):
        dest, text = self.args(2)
        value = self.string(text)
        assert dest == self.addr(0x663b60)
        self.u.mem_write(dest, value + b'\0')
        self.u.reg_write(UC_X86_REG_EAX, len(value))

    def draw(self):
        font, text, x, y, colour, flags = self.args(6)
        assert font == 2 and colour == self.addr(0x519ecc) and flags == 0x11
        self.trace.append(('text', self.string(text), x, y))

    def width(self):
        font, text = self.args(2)
        assert font == 2 and text == self.addr(0x663b60)
        self.u.reg_write(UC_X86_REG_EAX, len(self.string(text)) * 7)

    def bar(self):
        context, rect, colour, layer = self.args(4)
        assert (context, rect, colour, layer) == (HEAP + 0x1150, self.addr(0x541cc0), self.addr(0x519ecc), 2)
        self.trace.append(('bar', self.rect(rect)))

    def run(self, resolution, count, repeated, seed):
        self.reset()
        heap = bytearray(random.Random(seed).randbytes(0x10000))
        struct.pack_into('<2I', heap, 0x1000, *resolution)
        texts = [b'STAGE TIME', b'', b'FINLAND', b'~$*|{}\xc0\xdf\xf7\xd3\x99', b'WINNER']
        for i, text in enumerate(texts):
            heap[0x4000 + i * 0x200:0x4000 + i * 0x200 + len(text) + 1] = text + b'\0'
        self.u.mem_write(HEAP, bytes(heap))
        self.put(self.addr(0x520b74), '<I', HEAP + 0x1000)
        self.put(self.addr(0x541cc0), '<4h', -125, -133, -147, -161)
        self.u.mem_write(self.addr(0x663b60), b'\xa5' * 260)
        pointers = [HEAP + 0x4000 + (0 if repeated else i % 5) * 0x200 for i in range(count)]
        x, y = 30, 67
        expected_trace = []
        expected_rect = [-125, y - trunc(resolution[1] * 30, 480), 2, trunc(resolution[1] * 41, 480)]
        scratch = bytearray(b'\xa5' * 260)
        for i, pointer in enumerate(pointers):
            offset = pointer - HEAP
            text = lower(bytes(heap[offset:offset + 260]).split(b'\0', 1)[0])
            heap[offset:offset + len(text)] = text
            expected_trace.append(('text', text, x, y))
            scratch[:len(text) + 1] = text + b'\0'
            if i + 1 < count:
                width = trunc(resolution[0] * 8, 640) + x + len(text) * 7
                expected_rect[0] = (width + 32768) % 65536 - 32768
                expected_trace.append(('bar', tuple(expected_rect)))
                x = width + trunc(resolution[0] * 8, 640) + 2
        saved = [0x12345678 + i for i in range(4)]
        for register, value in zip(SAVED, saved):
            self.u.reg_write(register, value)
        # A null first string still performs the row's geometry initialization.
        self.invoke(0x454df0, [30, 67, *(pointers or [0]), 0])
        assert self.trace == expected_trace, (resolution, count, repeated, self.trace, expected_trace)
        assert self.read(HEAP, 0x10000) == bytes(heap), 'whole heap guard/text mutations'
        assert self.read(self.addr(0x663b60), 260) == bytes(scratch)
        assert self.rect(self.addr(0x541cc0)) == tuple(expected_rect), ('rect', count, self.rect(self.addr(0x541cc0)), expected_rect)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff04, 'cdecl caller cleanup'
        assert [self.u.reg_read(r) for r in SAVED] == saved
        return self.trace, bytes(heap), bytes(scratch), tuple(expected_rect)

    def setup_prefix(self, country, stage, paused):
        """Execute the results setup through ghost name/splits initialization.

        Stop at the queue-action boundary; the five record mergers have their
        own whole-function harness, and the native probe covers the complete
        result menu and continue transition with actual assets.
        """
        self.reset()
        heap = bytearray(random.Random(country * 11 + stage).randbytes(0x10000))
        record = 0x6000 + 0x654 + (country * 11 + stage) * 8
        heap[record:record + 4] = b'ABC\0'
        value = struct.unpack_from('<I', heap, record + 4)[0]
        self.u.mem_write(HEAP, bytes(heap))
        self.u.mem_write(self.addr(0x519ee0), b'\xa5' * 4)
        self.u.mem_write(self.addr(0x541ab8), b'\xa5' * 40)

        def result(value):
            self.u.reg_write(UC_X86_REG_EAX, value)

        def copy():
            dest, text = self.args(2)
            assert dest == self.addr(0x519ee0) and text == HEAP + record
            self.u.mem_write(dest, self.string(text) + b'\0')
            result(3)

        def split():
            index, = self.args(1)
            assert index < 10
            self.trace.append(('split', index))
            result(index * 111)

        def pause():
            assert self.args(1) == (1,)
            self.trace.append(('pause',))
            result(0)

        def boundary():
            self.trace.append(('queue-boundary',))
            self.put(self.u.reg_read(UC_X86_REG_ESP), '<I', 0x30000000)

        self.callbacks = {self.addr(0x41f380): (0, lambda: result(255)),
                          self.addr(0x405fe0): (0, lambda: result(HEAP + 0x6000)),
                          self.addr(0x406910): (0, lambda: result(country)),
                          self.addr(0x406930): (0, lambda: result(stage)),
                          self.addr(0x405620): (0, copy),
                          self.addr(0x4d0220): (4, split),
                          self.addr(0x49ea90): (4, pause),
                          self.addr(0x4a0ba0): (0, boundary)}
        self.invoke(0x44a1b0, [paused])
        assert self.read(HEAP, 0x10000) == bytes(heap)
        assert self.read(self.addr(0x519ee0), 4) == b'abc\0'
        expected = [i * 111 for i in range(9)] + [value >> 7 & 65535]
        assert self.read(self.addr(0x541ab8), 40) == struct.pack('<10I', *expected)
        assert self.trace == [('split', i) for i in range(10)] + ([('pause',)] if paused == 0 else []) + [('queue-boundary',)]
        return bytes(heap), expected, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    rebuilt = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe'
    impl = [Rows(ROOT / 'cmr2bin/CMR2.exe'), Rows(rebuilt, entities)]
    count = 0
    for seed, (resolution, strings, repeated) in enumerate(itertools.product(
            [(640, 480), (800, 600), (1920, 1080)], range(9), [False, True])):
        a, b = [i.run(resolution, strings, repeated, seed) for i in impl]
        assert a == b
        count += 1
    impl = [Rows(ROOT / 'cmr2bin/CMR2.exe'), Rows(rebuilt, entities)]
    for country, stage, paused in itertools.product(range(8), range(11), [0, 1]):
        assert impl[0].setup_prefix(country, stage, paused) == impl[1].setup_prefix(country, stage, paused)
    print(f'{count} result text rows: original/rebuilt/model, complete heap, Polish text, cdecl ABI')
    print('176 results-setup prefixes: original/rebuilt/model, ghost name and ten splits')


if __name__ == '__main__':
    main()
