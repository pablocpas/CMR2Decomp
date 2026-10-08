#!/usr/bin/env python3
"""Compare complete original/rebuilt debris drawing, including queued vertices."""
import argparse
import math
from pathlib import Path
import random
import struct

from differential_car_forces import object_body, SCRATCH
from differential_car_body_matrix import SAVED
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from unicorn.x86_const import (
    UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_EIP,
    UC_X86_REG_ESP, UC_X86_REG_EFLAGS,
)

ADDRESS = 0x499ac0


class Debris(Drawing):
    def __init__(self, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.code = code
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)
        self.callbacks = {0x4b3ae0: (8, self.light), 0x4bba00: (24, self.queue)}

    def light(self):
        dest, level = self.args(2)
        self.trace.append(('light', level))
        values = (0, 1, 127, 248, 249, 254, 255)
        self.put(dest, '<4B', *(values[(self.seed + 3*i) % len(values)] for i in range(3)), 0x73)
        self.u.reg_write(UC_X86_REG_EAX, 0xfedcba98)
        self.u.reg_write(UC_X86_REG_ECX, 0x12345678)
        self.u.reg_write(UC_X86_REG_EDX, 0x87654321)

    def queue(self):
        unused, a, b, c, texture, layer = self.args(6)
        self.trace.append(('queue', unused, *(self.read(p, 24) for p in (a, b, c)), texture, layer))

    def run(self, seed):
        self.seed = seed
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        self.put(0x6e0ef4, '<4096H',
                 *(int(math.sqrt((8 + 16*i) / 65536) * 65536) for i in range(4096)))
        self.u.mem_write(0x5928c0, rnd.randbytes(0x592d80 - 0x5928c0))
        self.put(0x5928c0, '<270i', *(rnd.randrange(-6554, 6555) for _ in range(270)))
        particle, kind, view, matrix = (HEAP + off for off in (0x1000, 0x2000, 0x3000, 0x4000))
        self.put(particle + 0x64, '<h', (seed % 30) * 256 + (seed // 30) % 2)
        self.put(particle + 0x40, '<I', matrix)
        self.put(particle + 0x4c, '<i', rnd.randrange(-65536, 131073))
        self.put(kind + 0x38, '<I', HEAP + 0x5000)
        self.put(particle + 0x1c, '<3i', *(rnd.randrange(-100000, 100001) for _ in range(3)))
        self.put(matrix + 0x30, '<3i', *(rnd.randrange(-100000, 100001) for _ in range(3)))
        camera = tuple(rnd.randrange(-500000, 500001) for _ in range(3))
        if seed % 5 == 0:
            camera = (0, 0, 0)
            self.put(particle + 0x1c, '<3i', 0, 0, 0)
            self.put(matrix + 0x30, '<3i', 0, 0, 0)
        if seed % 19 == 0:
            self.put(0x5928c0 + (seed % 30)*36, '<9i', *([0] * 9))
        self.put(view + 0x98 + 0x30, '<3i', *camera)
        for r in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(r, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<4I', STOP, particle, kind, view)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(ADDRESS if self.code is None else SCRATCH, STOP, count=200000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'body did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 16, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        assert [call[0] for call in self.trace] == ['light', 'queue'], self.trace
        return self.read(HEAP, 0x10000), self.read(0x5928c0, 0x592d80 - 0x5928c0), self.trace


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=300)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    code = object_body(args.object, ADDRESS, 'CarEffects.cpp')
    original, rebuilt = Debris(), Debris(code)
    for seed in range(args.cases):
        if original.run(seed) != rebuilt.run(seed):
            print(f'FAIL debris draw seed {seed}')
            return 1
    damaged = bytearray(code)
    at = damaged.find(bytes.fromhex('b8 00 80 01 00'))
    assert at >= 0, 'camera distance multiplier missing'
    struct.pack_into('<I', damaged, at + 1, 0x10000)
    negative = Debris(bytes(damaged))
    assert any(original.run(seed) != negative.run(seed) for seed in range(1, 5)), 'negative control passed'
    print(f'PASS debris draw: {args.cases} cases, guarded memory, lighting and queued vertices identical; negative control detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
