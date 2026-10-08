#!/usr/bin/env python3
"""Compare both marker sprites and player colour queries with the original.

Exercise left/right clipping, signed short coordinates and fixed-point scales.
The complete body executes; only player lookup and sprite submission are mocked.
"""
import argparse
from pathlib import Path
import random
import struct

from differential_car_forces import object_body, SCRATCH
from differential_car_body_matrix import SAVED
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)

ADDRESS = 0x413e00


class Marker(Drawing):
    def __init__(self, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.code = code
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)
        self.callbacks = {
            0x4a1a00: (0, lambda: self.query('local', 0, 123)),
            0x40b020: (4, lambda: self.query('find', 1, 3)),
            0x409d20: (4, lambda: self.query('id', 1, 456)),
            0x40b0a0: (4, lambda: self.query('colour', 1, HEAP + 0x9000)),
            0x4a3290: (36, self.sprite),
        }

    def query(self, kind, count, value):
        self.trace.append((kind, *self.args(count)))
        self.u.reg_write(UC_X86_REG_EAX, value)
        self.u.reg_write(UC_X86_REG_ECX, 0x12345678)
        self.u.reg_write(UC_X86_REG_EDX, 0x87654321)

    def run(self, seed):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        count, player = 1 + seed % 8, (seed // 8) % 7
        left = rnd.choice((-32768, -1000, 0, 100, 32700))
        for i in range(count):
            self.put(HEAP + 8*i, '<4h', left + min(i*3, 32767-left),
                     rnd.randrange(-32768, 32768), rnd.randrange(0, 1000),
                     rnd.randrange(0, 1000))
        scale = rnd.choice((0, 1, 32768, 65535, 65536, 131072, -65536))
        self.put(0x536c48 + 8*player, '<2i', rnd.randrange(count), scale)
        self.put(0x536c90, '<i', count)
        for global_, texture in ((0x537070, HEAP + 0x2000), (0x537074, HEAP + 0x3000)):
            self.put(global_, '<I', texture)
            self.put(texture + 0x11c, '<4h', rnd.randrange(-100, 101),
                     rnd.randrange(-100, 101), rnd.randrange(1, 256), rnd.randrange(1, 256))
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<3I', STOP, HEAP, player)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(ADDRESS if self.code is None else SCRATCH, STOP, count=20000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'marker did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 12, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        assert sum(t[0] == 'sprite' for t in self.trace) == 2, self.trace
        return self.read(HEAP, 0x10000), self.trace


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=400)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    code = object_body(args.object, ADDRESS, 'RallyData.cpp')
    original, rebuilt = Marker(), Marker(code)
    for seed in range(args.cases):
        if original.run(seed) != rebuilt.run(seed):
            print(f'FAIL race position marker seed {seed}: {original.trace} / {rebuilt.trace}')
            return 1
    damaged = bytearray(code)
    at = damaged.find(bytes.fromhex('6a 02'))
    assert at >= 0, 'sprite layer argument missing'
    damaged[at + 1] = 3
    negative = Marker(bytes(damaged))
    assert original.run(0) != negative.run(0), 'negative control passed'
    print(f'PASS race position marker: {args.cases} cases, clipping, sprite calls, guarded memory and ABI identical; negative control detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
