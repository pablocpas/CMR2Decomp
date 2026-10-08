#!/usr/bin/env python3
"""Compare the complete suspension/body-matrix pass with the original in Unicorn.

Only the cheat query is intercepted. Matrix helpers and fixed-point arithmetic
execute from the original image. Guarded memory, query order and the calling
convention must agree for rotated and degenerate bases as well as uneven wheels.
"""
import argparse
import math
from pathlib import Path
import random
import struct

from differential_car_forces import object_body, SCRATCH
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from unicorn.x86_const import (
    UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_EIP,
    UC_X86_REG_ESP, UC_X86_REG_EFLAGS,
)

ADDRESS = 0x432cc0
SAVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


class BodyMatrix(Drawing):
    def __init__(self, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.code = code
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)
        self.callbacks = {0x4063f0: (4, self.cheat)}

    def cheat(self):
        self.trace.append(self.args(1))
        self.u.reg_write(UC_X86_REG_EAX, self.seed & 1)
        # The body must not rely on caller-saved registers across this query.
        self.u.reg_write(UC_X86_REG_ECX, 0x12345678)
        self.u.reg_write(UC_X86_REG_EDX, 0x87654321)

    def run(self, seed):
        self.seed = seed
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        self.put(0x6e0ef4, '<4096H',
                 *(int(math.sqrt((8 + 16*i) / 65536) * 65536) for i in range(4096)))
        car, world, body = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x3100
        self.put(0x53cc18, '<I', car)
        self.put(car + 0x750, '<2I', world, body)
        for matrix in (world, body):
            yaw, tilt = rnd.uniform(-math.pi, math.pi), rnd.uniform(-0.6, 0.6)
            cy, sy, cz, sz = math.cos(yaw), math.sin(yaw), math.cos(tilt), math.sin(tilt)
            axes = ((cy*cz, cy*sz, -sy), (-sz, cz, 0), (sy*cz, sy*sz, cy))
            for i, axis in enumerate(axes):
                self.put(matrix + 16*i, '<3i', *(round(v * 65536) for v in axis))
        if seed % 11 == 0:
            for i in range(3):
                self.put(body + 16*i, '<3i', 0, 0, 0)
        if seed % 13 == 0:
            for i in range(3):
                self.put(world + 16*i, '<3i', 0, 0, 0)
        self.put(car + 0x2d0, '<3i', *(rnd.randrange(-0x200000, 0x200001) for _ in range(3)))
        self.put(car + 0x36c, '<3i', *(rnd.randrange(-65536, 65537) for _ in range(3)))
        for i, (x, z) in enumerate(((-60000, 100000), (60000, 100000),
                                   (-60000, -100000), (60000, -100000))):
            if seed % 17 == 0:
                x = z = 0
            elif seed % 7 == 0:
                x, z = -x, -z
            else:
                x += rnd.randrange(-10000, 10001)
                z += rnd.randrange(-10000, 10001)
            self.put(car + 0x210 + 12*i, '<3i', x, rnd.randrange(-30000, 30001), z)
            for offset in (0x988, 0x9d8, 0x9a8):
                value = rnd.randrange(-40000, 40001)
                if seed % 17 == 0:
                    value = 0
                self.put(car + offset + 4*i, '<i', value)
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<I', STOP)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(ADDRESS if self.code is None else SCRATCH, STOP, count=200000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'body did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 4, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        assert self.trace == [(6,)] * 4, self.trace
        return self.read(HEAP, 0x10000), self.trace


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=300)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    code = object_body(args.object, ADDRESS)
    original, rebuilt = BodyMatrix(), BodyMatrix(code)
    for seed in range(args.cases):
        expected, actual = original.run(seed), rebuilt.run(seed)
        if expected != actual:
            offsets = [hex(i) for i, (a, b) in enumerate(zip(expected[0], actual[0])) if a != b]
            print(f'FAIL seed {seed}: guarded heap offsets {offsets[:24]}')
            return 1
    # Prove that the comparison catches a wrong suspension averaging factor.
    damaged = bytearray(code)
    at = damaged.find(bytes.fromhex('b8 00 80 00 00'))
    assert at >= 0, 'averaging multiplier missing'
    struct.pack_into('<I', damaged, at + 1, 0x9000)
    negative = BodyMatrix(bytes(damaged))
    assert any(original.run(seed) != negative.run(seed) for seed in range(1, 5)), 'negative control passed'
    print(f'PASS body matrix: {args.cases} cases, guarded memory, cheat calls and ABI identical; negative control detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
