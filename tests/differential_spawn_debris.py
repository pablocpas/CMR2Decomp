#!/usr/bin/env python3
"""Compare complete debris spawning, random calls, velocities and glass limits."""
import argparse
from pathlib import Path
import random
import struct

from differential_car_forces import object_body, SCRATCH
from differential_car_body_matrix import SAVED
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)

ADDRESS = 0x499750


class Debris(Drawing):
    def __init__(self, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.code = code
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)
        self.callbacks = {
            0x469680: (4, lambda: self.result('record', self.args(1), HEAP + 0x6000)),
            0x407e70: (0, lambda: self.result('blocked', (), self.seed % 19 == 0)),
            0x4c6676: (0, self.rand),
            0x4b07a0: (36, self.particle),
        }

    def result(self, name, args, value):
        self.trace.append((name, *args, value))
        self.u.reg_write(UC_X86_REG_EAX, value & 0xffffffff)
        self.u.reg_write(UC_X86_REG_ECX, 0x12345678)
        self.u.reg_write(UC_X86_REG_EDX, 0x87654321)

    def rand(self):
        edges = (0, 1, 16383, 16384, 32766, 32767)
        value = edges[(self.seed + self.draws) % len(edges)] if self.seed % 2 else self.rnd.randrange(32768)
        self.draws += 1
        self.result('rand', (), value)

    def particle(self):
        args = list(self.args(9))
        args[2] = struct.unpack('<3i', self.read(args[2], 12))
        args[8] &= 255  # Particle_Spawn declares the light argument as BYTE.
        self.result('particle', args, HEAP + 0x7000)

    def run(self, seed):
        self.seed, self.rnd, self.draws = seed, random.Random(seed), 0
        rnd = self.rnd
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        car = HEAP + 0x1000
        self.put(car + 0xb1a, '<b', seed % 8)
        self.put(car + 0xc0c, '<i', int(seed % 23 == 0))
        self.put(car + 0x71c, '<2I', HEAP + 0x3000, HEAP + 0x3500)
        self.put(HEAP + 0x3500 + 0x17c, '<B', seed % 256)
        for i in range(3):
            axis = [rnd.randrange(-100000, 100001) for _ in range(3)]
            self.put(HEAP + 0x4000 + 12*i, '<3i', *axis)
        self.put(HEAP + 0x5000, '<3i', *(rnd.randrange(-1000000, 1000001) for _ in range(3)))
        self.put(HEAP + 0x6469, '<B', (0, 7, 8, 253, 254, 255)[(seed // 7) % 6])
        count = (-65536, 0, 65536, 5*65536)[seed % 4]
        size = (0, 1, 65535, 65536, 500000)[(seed // 4) % 5]
        chance = (-1, 0, 32767, 32768, 65536, 65537)[(seed // 20) % 6]
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<7I', STOP, size, HEAP + 0x5000, car, HEAP + 0x4000,
                 count & 0xffffffff, chance & 0xffffffff)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(ADDRESS if self.code is None else SCRATCH, STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'spawner did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 28, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        return self.read(HEAP, 0x10000), self.trace


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=800)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    code = object_body(args.object, ADDRESS, 'CarEffects.cpp')
    original, rebuilt = Debris(), Debris(code)
    for seed in range(args.cases):
        if original.run(seed) != rebuilt.run(seed):
            print(f'FAIL debris seed {seed}: {original.trace} / {rebuilt.trace}')
            return 1
    damaged = bytearray(code)
    at = damaged.find(struct.pack('<I', 0x3333))
    assert at >= 0, 'extent constant missing'
    struct.pack_into('<I', damaged, at, 0x1999)
    negative = Debris(bytes(damaged))
    assert any(original.run(seed) != negative.run(seed) for seed in range(1, 40)), 'negative control passed'
    print(f'PASS debris spawning: {args.cases} cases, random calls, velocities, glass limits, guards and ABI identical; negative control detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
