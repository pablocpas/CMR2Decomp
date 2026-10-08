#!/usr/bin/env python3
"""Compare the complete basis rotation, all axis branches and normalization.

No calls are mocked. Exercise null and nonzero vectors, 12-bit angle boundaries,
aliased angle storage, poisoned stack memory, heap guards and the calling ABI.
"""
import argparse
import math
from pathlib import Path
import random
import struct

from differential_car_forces import F, object_body, SCRATCH
from differential_car_body_matrix import SAVED
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)

ADDRESS = 0x429f20
SINES = [round(math.sin(i*math.tau/4096)*65536) for i in range(4096)]
ROOTS = [int(math.sqrt((8 + 16*i)/65536)*65536) for i in range(4096)]


class Rotation(Drawing):
    def __init__(self, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.code, self.callbacks = code, {}
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)

    def run(self, seed):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        self.put(0x6e2ef4, '<4096i', *SINES)
        self.put(0x6e0ef4, '<4096H', *ROOTS)
        values = [65536, 0, 0, 0, 65536, 0, 0, 0, 65536]
        if seed % 5 == 0:
            values = [0]*9
        elif seed % 5 in (1, 2):
            values = [rnd.randrange(-65536, 65537) for _ in range(9)]
        edges = (0, 1, 0x3ff, 0x400, 0x7ff, 0x800, 0xbff, 0xc00, 0xfff, 0x1000, 0xffff)
        angles = [edges[(seed // len(edges)**i) % len(edges)] for i in range(3)]
        if seed >= len(edges)**3:
            angles = [rnd.randrange(65536) for _ in range(3)]
        basis = HEAP + 0x1000 + (seed % 7 == 0)
        pointer = basis + 12*((seed // 13) % 3) if seed % 13 == 0 else HEAP + 0x2000
        self.put(basis, '<9i', *values)
        if pointer == HEAP + 0x2000:
            self.put(pointer, '<3H', *angles)
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<3I', STOP, basis, pointer)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.u.emu_start(ADDRESS if self.code is None else SCRATCH, STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'rotation did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 12, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        return self.read(HEAP, 0x10000)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=1600)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    code = object_body(args.object, ADDRESS, 'FixedPoint.cpp')
    original, rebuilt = Rotation(), Rotation(code)
    for seed in range(args.cases):
        if original.run(seed) != rebuilt.run(seed):
            print(f'FAIL basis rotation seed {seed}')
            return 1
    damaged = bytearray(code)
    neg = next(ins for ins in F.md.disasm(code, SCRATCH) if ins.mnemonic == 'neg')
    at = neg.address - SCRATCH
    damaged[at:at + neg.size] = b'\x90'*neg.size
    negative = Rotation(bytes(damaged))
    assert any(original.run(seed) != negative.run(seed) for seed in range(11, 40)), 'negative control passed'
    print(f'PASS basis rotation: {args.cases} cases, actual fixed-point helpers, angle aliases, guards and ABI identical; reversed-angle negative control detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
