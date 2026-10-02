#!/usr/bin/env python3
"""Run original/rebuilt car reset basis with controlled terrain providers.

Usage: differential_car_basis.py entities.json [rebuilt.exe]
Only terrain queries, scene updates and effect providers are intercepted. The
complete reset body and its fixed point arithmetic execute unchanged; compare
the full guarded car, render matrices and provider order on flat/sloped ground.
"""
import json
import math
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP


class Basis(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in [(0x4930e0, 2), (0x42de20, 0), (0x43eef0, 1),
                               (0x4ac480, 2), (0x42f820, 0), (0x460c80, 1),
                               (0x4781d0, 2), (0x4789b0, 1)]:
            self.callbacks[self.addr(address)] = (nargs * 4, self.provider(address, nargs))

    def provider(self, address, nargs):
        def call():
            args = self.args(nargs)
            if address == 0x4ac480:
                self.trace.append((address, args[0], self.read(args[1], 12)))
            else:
                self.trace.append((address, args))
            if address == 0x460c80:
                self.u.reg_write(UC_X86_REG_EAX, 0)
        return call

    def run(self, seed):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        # Runtime-populated square-root lookup table, shared by both images.
        self.put(self.addr(0x6e0ef4), '<4096H',
                 *(int(math.sqrt((8 + 16*i) / 65536) * 65536) for i in range(4096)))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        car = HEAP + 0x1000
        self.put(self.addr(0x53cc18), '<I', car)
        for offset, target in [(0x720, HEAP + 0x3000), (0x750, HEAP + 0x4000)]:
            self.put(car + offset, '<I', target)
        if seed < 4:
            right = [(65536, 0, 0), (0, 0, 65536), (-65536, 0, 0), (0, 0, -65536)][seed]
            normal = (0, 65536, 0)
        else:
            normal = (rnd.randrange(-30000, 30000), 65536, rnd.randrange(-30000, 30000))
            length = math.sqrt(sum(v*v for v in normal))
            normal = tuple(int(v * 65536 / length) for v in normal)
            right = (rnd.randrange(-65536, 65537), rnd.randrange(-10000, 10001), rnd.randrange(-65536, 65537))
        self.put(car + 0x360, '<3i', *right)
        self.put(car + 0x48c, '<3i', *normal)
        self.put(car + 0x2d0, '<3i', *(rnd.randrange(-0x1000000, 0x1000000) for _ in range(3)))
        self.put(car + 0x8dc, '<i', rnd.randrange(-65536, 65536))
        self.put(car + 0x274, '<i', rnd.randrange(-65536, 65536))
        sp = STACK + 0xff00
        self.put(sp, '<2I', STOP, car)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(self.addr(0x43e680), STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'reset did not return'
        return self.read(HEAP, 0x10000), self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Basis(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Basis(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    for seed in range(300):
        a, b = original.run(seed), rebuilt.run(seed)
        if a != b:
            different = [i for i, (x, y) in enumerate(zip(a[0], b[0])) if x != y]
            print('FAIL reset seed', seed, 'car offsets:', [hex(i - 0x1000) for i in different[:20]])
            for offset in [0x360, 0x36c, 0x378]:
                print(hex(offset), 'original:', struct.unpack_from('<3i', a[0], 0x1000+offset),
                      'rebuilt:', struct.unpack_from('<3i', b[0], 0x1000+offset))
            if a[1] != b[1]:
                print('provider traces:', a[1], b[1])
            return 1
    print('300 reset cases: identical guarded car/matrices and provider traces on flat/sloped terrain')
    return 0


if __name__ == '__main__':
    sys.exit(main())
