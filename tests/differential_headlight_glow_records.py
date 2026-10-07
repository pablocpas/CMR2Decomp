#!/usr/bin/env python3
"""Run create/spawn/update/interpolate on the original's shared glow records.

Only glow allocation, ground queries, RNG and particle creation are controlled.
Glow setters and all record bodies execute normally. Compare the complete record
block, car timers, glow allocations and emitted particles after every phase;
separate allocations for interior views must fail even if byte relocation agrees.
"""
import json
import math
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK

CAR, MATRIX, NODE, GLOWS = HEAP + 0x1000, HEAP + 0x4000, HEAP + 0x5000, HEAP + 0x8000
RECORDS, TIMERS = 0x58e4c8, 0x58e4a8
TARGETS = (0x47d510, 0x47d5a0, 0x47dd70, 0x47e1e0)
SQRT = struct.pack('<4096H', *(math.isqrt((8 + 16 * i) * 65536) for i in range(4096)))


class Glows(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x4ae2f0, 13), (0x42b5f0, 1), (0x4932b0, 6),
                               (0x4c6676, 0), (0x4b07a0, 9)):
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        args = list(self.args(nargs))
        value = 0
        if address == 0x4ae2f0:
            value = GLOWS + self.allocated * 0x5c
            self.allocated += 1
            assert self.allocated <= 100
            # This allocator is called with unused, uninitialised vectors.
            args[10] &= 255
            self.trace.append(('allocate', tuple(args[i] for i in (0, 4, 5, 6, 7, 8, 9, 10, 12))))
        elif address == 0x42b5f0:
            self.trace.append(('car', args[0]))
            value = CAR
        elif address == 0x4932b0:
            position = struct.unpack('<3i', self.read(args[0], 12))
            self.trace.append(('ground', position))
            self.put(args[1], '<3i', -0x1000, 0xff00, 0x1000)
            self.put(args[2], '<h', 1)
            self.put(args[3], '<h', 2)
            self.put(args[4], '<H', 4)
            value = (position[0] // 65536) * 0x1000 + (position[2] // 65536) * 0x800
        elif address == 0x4c6676:
            self.seed = (self.seed * 214013 + 2531011) & 0xffffffff
            value = (self.seed >> 16) & 32767
            self.trace.append(('rand', value))
        elif address == 0x4b07a0:
            args[1] = self.read(args[1], 12)
            args[2] = self.read(args[2], 12)
            assert args[5] == 0 and args[7] == 0
            args[6] &= 255
            args[8] &= 255
            self.trace.append(('particle', tuple(args)))
        self.u.reg_write(UC_X86_REG_EAX, value & 0xffffffff)
        self.u.reg_write(UC_X86_REG_ECX, 0xa5a5a5a5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5a5a5a5a)

    def setup(self, seed):
        self.reset()
        self.seed, self.allocated = seed, 0
        self.u.mem_write(self.addr(0x6e0ef4), SQRT)
        self.put(CAR + 0x750, '<I', MATRIX)
        self.put(CAR + 0x720, '<I', NODE)
        self.put(NODE + 0x17c, '<B', 17)
        self.put(CAR + 0x360, '<6i', 65536, 0, 0, 0, 65536, 0)
        self.put(CAR + 0x770, '<3i', 65536, 0x20000, 0x30000)
        self.put(CAR + 0x270, '<6i', -0x20000, 0x10000, -0x30000,
                 0x20000, 0x14000, -0x30000)
        self.put(CAR + 0x408, '<3i', 0x4000, 0x3000, -0x2000)

    def snapshot(self):
        return (self.read(self.addr(RECORDS), 100 * 0x5c),
                self.read(self.addr(TIMERS), 32), self.read(HEAP, 0x10000), list(self.trace))

    def call(self, address, args):
        self.invoke(address, args)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff00 + 4 * (len(args) + 1)
        return self.snapshot()

    def occupy(self, count):
        for i in range(count):
            record = self.addr(RECORDS) + i * 0x5c
            self.put(record, '<9i', 0x2000, 0, -0x1000, 0x10000 + i * 257,
                     0x8000, 0x20000, 0, 0x10000, 0)
            self.put(record + 0x24, '<i', 0x10000)
            self.u.mem_write(record + 0x28, self.read(record + 0xc, 24))
            self.put(record + 0x40, '<3i', 0x10000, 0, 0x40000 + i * 0x100)
            self.put(record + 0x54, '<IB', 1, i % 8)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    a = Glows(ROOT / 'cmr2bin/CMR2.exe')
    b = Glows(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = phases = particles = 0
    for occupied in (0, 1, 50, 99, 100):
        for step in (0x1000, 0x10000, 0x40000, 0x320000):
            for factor in (0, 0x8000, 0x10000):
                seed = cases + 17
                a.setup(seed); b.setup(seed)
                operations = [(0x47d510, [])]
                for address, args in operations:
                    aa, bb = a.call(address, args), b.call(address, args)
                    if aa != bb:
                        print('FAIL', hex(address), 'initialisation', occupied, step, factor)
                        print('record block equal:', aa[0] == bb[0], 'heap equal:', aa[2] == bb[2])
                        return 1
                    phases += 1
                assert a.allocated == b.allocated == 100
                a.occupy(occupied); b.occupy(occupied)
                for fixture in (a, b):
                    fixture.put(fixture.addr(0x519c8c), '<i', step)
                operations = [(0x47d5a0, [0]), (0x47d5a0, [0]), (0x47e1e0, [factor])]
                for _ in range(3):
                    operations.extend(((0x47dd70, []), (0x47e1e0, [factor])))
                for address, args in operations:
                    aa, bb = a.call(address, args), b.call(address, args)
                    if aa != bb:
                        print('FAIL', hex(address), occupied, step, factor)
                        print('record block equal:', aa[0] == bb[0], 'heap equal:', aa[2] == bb[2])
                        return 1
                    phases += 1
                particles += sum(event[0] == 'particle' for event in a.trace)
                cases += 1
    assert particles > 100, 'update branches were not exercised'
    print(cases, 'lifecycles,', phases, 'phase comparisons,', particles,
          'particles: identical shared records, timers and glow output')
    return 0


if __name__ == '__main__':
    sys.exit(main())
