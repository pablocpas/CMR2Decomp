#!/usr/bin/env python3
"""Compare nearest local/network engine selection and complete sound call traces.

Vector and matrix helpers run from the original image. Only car/player providers
and audio leaves are intercepted. Cover empty lists, ties, shuffled network car
IDs, existing/restarted sounds, pitch clamps and changing attenuation queries.
"""
import argparse
import math
from pathlib import Path
import random
import struct

from differential_car_forces import object_body, SCRATCH
from differential_car_body_matrix import SAVED
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)

TARGETS = (0x47aa70, 0x47ad20)


class Engines(Drawing):
    def __init__(self, address, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.address, self.code = address, code
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)
        self.callbacks = {
            0x42b5f0: (4, self.car),
            0x42b700: (0, lambda: self.query('count', 0, self.count + self.locals)),
            0x4074f0: (0, lambda: self.query('locals', 0, self.locals)),
            0x409cb0: (4, self.present),
            0x40b010: (4, self.car_id),
            0x427d50: (8, self.attenuation),
            0x4b78a0: (4, self.playing),
            0x4086b0: (4, lambda: self.query('selection', 1, self.seed % 4)),
            0x40ee90: (4, lambda: self.query('archive', 1, (self.seed + 1) % 4)),
            0x4b7790: (24, self.play),
            0x427ab0: (8, self.in_range),
            0x427b70: (8, lambda: self.query('curve', 2, (self.seed*811) & 0xffff)),
            0x427e20: (12, lambda: self.query('scale-pan', 3, (self.seed*1313) & 0xffff)),
            0x4b79e0: (8, lambda: self.query('pan', 2, 0)),
            0x4b79a0: (8, lambda: self.query('volume', 2, 0)),
        }

    def query(self, kind, count, value):
        self.trace.append((kind, *self.args(count)))
        self.u.reg_write(UC_X86_REG_EAX, value & 0xffffffff)
        self.u.reg_write(UC_X86_REG_ECX, 0x12345678)
        self.u.reg_write(UC_X86_REG_EDX, 0x87654321)

    def car(self):
        index, = self.args(1)
        assert index < 7, ('car index', index)
        self.query('car', 1, HEAP + index*0x1000)

    def present(self):
        player, = self.args(1)
        self.query('present', 1, player < self.count)

    def car_id(self):
        player, = self.args(1)
        self.query('car-id', 1, self.ids[player])

    def attenuation(self):
        self.attenuations += 1
        # Changing returns catches reordered or missing provider calls.
        values = (0, 1, 32768, 65535, 65536, 49152)
        self.query('attenuation', 2, values[(self.seed + self.attenuations) % len(values)])

    def playing(self):
        handle, = self.args(1)
        self.query('playing', 1, (self.seed + handle) % 2)

    def play(self):
        self.plays += 1
        self.query('play', 6, 100 + self.plays)

    def in_range(self):
        pitch, curve = self.args(2)
        assert 2000 <= pitch <= 6500 and curve == 0x51ec50, (pitch, curve)
        self.query('range', 2, (self.seed + pitch) % 3 != 0)

    def run(self, seed):
        self.seed = seed
        self.count = seed % (6 if self.address == TARGETS[0] else 8)
        self.locals = 1 + (seed // 8) % 2
        rnd = random.Random(seed)
        self.ids = list(range(7))
        rnd.shuffle(self.ids)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        self.put(0x6e0ef4, '<4096H',
                 *(int(math.sqrt((8 + 16*i) / 65536) * 65536) for i in range(4096)))
        for i in range(7):
            car, matrix = HEAP + i*0x1000, HEAP + 0x8000 + i*0x80
            self.put(car + 0x750, '<I', matrix)
            for j in range(3):
                self.put(matrix + 16*j, '<4i',
                         *[65536 if k == j else 0 for k in range(3)], 0)
            position = (0, 0, 0) if seed % 11 == 0 else tuple(rnd.randrange(-0x200000, 0x200001) for _ in range(3))
            self.put(matrix + 0x30, '<4i', *position, 0)
            self.put(car + 0x798, '<i', rnd.choice((0, 65536, 131072, 262144)))
            self.put(car + 0x7ac, '<i', rnd.choice((0, 65536, 32768, 196608)))
        self.put(0x538d30 + 0x30, '<3i', 0, 0, 0)
        self.put(0x58ddc8 - 16, '<10i', *[1000+i for i in range(10)])
        self.put(0x58dda8, '<i', rnd.choice((0, 1, 32768, 65536)))
        self.put(0x58ddb4, '<i', rnd.randrange(1, 65535))
        self.put(0x51f27c, '<i', rnd.choice((0, 32768, 65536)))
        self.put(0x51f2d8, '<4i', 10000, 20000, 30000, 40000)
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<I', STOP)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace, self.attenuations, self.plays = [], 0, 0
        self.u.emu_start(self.address if self.code is None else SCRATCH, STOP, count=200000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'engine update did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 4, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        assert sum(t[0] == 'playing' for t in self.trace) == min(2, self.count), self.trace
        return self.read(HEAP, 0x10000), self.read(0x58ddc8 - 16, 40), self.trace


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=400)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    for address in TARGETS:
        code = object_body(args.object, address, 'StageObjects.cpp')
        original, rebuilt = Engines(address), Engines(address, code)
        for seed in range(args.cases):
            if original.run(seed) != rebuilt.run(seed):
                print(f'FAIL nearest engines {address:#x} seed {seed}: {original.trace} / {rebuilt.trace}')
                return 1
        damaged = bytearray(code)
        at = damaged.find(bytes.fromhex('b8 00 00 01 00'))
        assert at >= 0, 'identity distance multiplier missing'
        struct.pack_into('<I', damaged, at + 1, 32768)
        negative = Engines(address, bytes(damaged))
        assert any(original.run(seed) != negative.run(seed) for seed in range(1, 30)), 'negative control passed'
    print(f'PASS nearest car engines: {2*args.cases} cases, selections, sound calls, guarded memory and ABI identical; negative controls detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
