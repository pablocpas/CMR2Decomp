#!/usr/bin/env python3
"""Compare the full engine-speed body, startup phases and rev-limit side effects."""
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

ADDRESS = 0x4380c0


class Engine(Drawing):
    def __init__(self, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.code = code
        self.callbacks = {}
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)

    def run(self, seed):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        self.put(0x6e2ef4, '<4096i',
                 *(int(math.sin(i * math.tau / 4096) * 65536) for i in range(4096)))
        self.put(0x519c8c, '<i', (0, 64, 32768, 65536, 100000)[seed % 5])
        car = HEAP + 0x1000
        self.put(0x53cc18, '<I', car)
        self.put(car + 0xb1e, '<b', seed % 7)
        self.put(car + 0xb84, '<i', (seed // 63) % 2)
        self.put(car + 0xb4c, '<i', (seed // 126) % 3)
        timer = (-32768, -1, 0, 1, 999, 1000, 1001, 25000, 32767)[(seed // 7) % 9]
        self.put(car + 0xafe, '<h', timer)
        self.put(car + 0x1d8, '<i', (seed // 3) % 2)
        self.put(car + 0x7b4, '<i', (0, 1, 32768, 65536)[(seed // 5) % 4])
        self.put(car + 0x780, '<i', rnd.randrange(-20000, 20001))
        limit = (0, 10000, 32768, 65536, 131072)[seed % 5]
        self.put(car + 0x794, '<i', limit)
        speed = (-1, 0, limit, limit + 0xcccc, limit + 0xcccd,
                 limit + 0x1cccc, 300000)[(seed // 9) % 7]
        self.put(car + 0x7a4, '<i', speed)
        self.put(car + 0x7bc, '<7i',
                 *(rnd.randrange(32768, 131073) for _ in range(7)))
        self.put(car + 0x860, '<4i',
                 *(rnd.randrange(-0x300000, 0x300001) for _ in range(4)))
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<I', STOP)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.u.emu_start(ADDRESS if self.code is None else SCRATCH, STOP, count=200000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'body did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 4, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        return self.read(HEAP, 0x10000), self.read(0x519c8c, 4), self.read(0x53cc18, 4)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=600)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    code = object_body(args.object, ADDRESS)
    original, rebuilt = Engine(), Engine(code)
    for seed in range(args.cases):
        expected, actual = original.run(seed), rebuilt.run(seed)
        if expected != actual:
            offsets = [hex(i - 0x1000) for i, (a, b) in enumerate(zip(expected[0], actual[0])) if a != b]
            print(f'FAIL engine speed seed {seed}: guarded car offsets {offsets[:24]}')
            return 1
    damaged = bytearray(code)
    at = damaged.find(bytes.fromhex('ba eb 51 00 00'))
    assert at >= 0, 'excess-speed multiplier missing'
    struct.pack_into('<I', damaged, at + 1, 0)
    negative = Engine(bytes(damaged))
    assert any(original.run(seed) != negative.run(seed) for seed in range(64)), 'negative control passed'
    print(f'PASS engine speed: {args.cases} cases, guarded car, startup boundaries, rev limits and ABI identical; negative control detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
