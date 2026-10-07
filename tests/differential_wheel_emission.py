#!/usr/bin/env python3
"""Run original/rebuilt wheel dust and splash bodies, including RNG ordering.

Settings, RNG, wheel-speed queries, tyre wear and particle creation are controlled
leaves. Fixed-point arithmetic and the view-matrix helper execute normally.
Compare every RNG result, wear call and particle's position, velocity and colour;
speed queries return a stable value (the original ABS macro calls them twice).
"""
import json
from pathlib import Path
import random
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting

CAR, NODE, RESULTS = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x4000
TARGETS = (0x45b580, 0x45c820)


class Wheels(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x42b5f0, 1), (0x465d70, 2), (0x4c6676, 0),
                               (0x406910, 0), (0x460bf0, 1), (0x460c10, 1),
                               (0x42b600, 3), (0x4074f0, 0), (0x45c750, 4),
                               (0x41f3d0, 1), (0x41b390, 0), (0x4b07a0, 9)):
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        args = list(self.args(nargs))
        value = 0
        if address == 0x42b5f0:
            value = CAR
        elif address == 0x465d70:
            value = self.slip
        elif address == 0x4c6676:
            value = self.rng.randrange(32768)
            self.trace.append(('rand', value))
        elif address == 0x406910:
            value = self.country
        elif address == 0x460bf0:
            value = self.weather
        elif address == 0x460c10:
            value = self.rain
        elif address == 0x42b600:
            assert args[0] == CAR
            value = self.speed
        elif address == 0x4074f0:
            value = 8
        elif address == 0x41f3d0:
            value = self.finished
        elif address == 0x41b390:
            value = RESULTS
        elif address == 0x45c750:
            self.trace.append(('wear', tuple(args)))
        elif address == 0x4b07a0:
            args[1] = self.read(args[1], 12)
            args[2] = self.read(args[2], 12)
            args[5] = self.read(args[5], 4)
            args[6] &= 255
            args[7] = self.read(args[7], 4)
            args[8] &= 255
            self.trace.append(('particle', tuple(args)))
        self.u.reg_write(UC_X86_REG_EAX, value & 0xffffffff)
        self.u.reg_write(UC_X86_REG_ECX, 0xa5a5a5a5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5a5a5a5a)

    def run(self, target, case):
        self.reset()
        (seed, car, surfaces, material, self.country, self.weather, self.rain,
         self.speed, self.slip, finish, reverse, ground) = case
        self.rng = random.Random(seed)
        self.finished = finish == 1
        self.put(CAR + 0x720, '<I', NODE)
        self.put(NODE + 0x17c, '<B', 17)
        self.put(RESULTS + 4, '<I', RESULTS + 0x100)
        self.put(RESULTS + 0x100 + car * 8, '<B', 9 if finish == 2 else 0)
        self.put(CAR + 0xb1e, '<B', 7 if reverse else 2)
        self.put(CAR + 0xaae, '<4h', *surfaces)
        self.put(CAR + 0xac6, '<4h', *([material] * 4))
        self.put(CAR + 0xbac, '<4i', *(bool(ground & (1 << w)) for w in range(4)))
        self.put(CAR + 0x2d0, '<3i', 124234, 327644, -65434)
        self.put(CAR + 0x360, '<3i', -65536 if self.slip < 0 else 65536, 0, 0)
        self.put(self.addr(0x538d2c) + 4 + car * 100 + 24, '<3i', 65536, 0, 0)
        self.put(CAR + 0x6b4, '<6i', 52344, -1213, 12345, -42311, 1234, 16566)
        self.put(CAR + 0x6cc, '<6i', 12354, 1454, -65533, 16346, 4344, 62345)
        for wheel in range(4):
            self.put(CAR + 0x42c + wheel * 12, '<3i',
                     182745 + wheel * 86454, -24234 + wheel * 23143, -334643 + wheel * 523432)
            self.put(CAR + 0x870 + wheel * 4, '<i', self.slip - wheel * 25345)
            self.put(CAR + 0x880 + wheel * 4, '<i', self.slip + wheel * 7532)
            self.put(self.addr(0x543200) + car * 48 + wheel * 12, '<3i',
                     34567 + wheel * 73646, 877354 + wheel * 333, 423451 - wheel * 111765)
            self.put(self.addr(0x543400) + car * 48 + wheel * 12, '<3i',
                     32684 + wheel * 76746, 876943 + wheel * 294, 423258 - wheel * 111474)
            self.put(self.addr(0x543120) + car * 4 + wheel, '<B', 32 + wheel)
        before = self.read(HEAP, 0x10000)
        self.invoke(target, [car])
        assert self.read(HEAP, 0x10000) == before, 'unexpected heap write'
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff08
        return self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Wheels(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Wheels(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    rng = random.Random(99412)
    for target in TARGETS:
        # Numerator-before-denominator regression: wet trailing wheels in reverse.
        cases = [(4, 0, (15, 15, 15, 15), 2, 0, 2, 0, 0x230000, -65536, 0, 1, 15)]
        for seed in range(1500):
            surfaces = tuple(rng.choice((0, 6, 9, 10, 11, 13, 14, 15, 17, 19, 20, 21,
                                         22, 23, 24, 25, 42)) for _ in range(4))
            cases.append((seed, rng.choice((0, 0, 0, 1, 7)), surfaces,
                          rng.choice((2, 3, 6, 8, 10, 0x11, 0x12, 0x1b, 0x1c, 0x48, 0x5b, 0x5c, 0x5d)),
                          rng.randrange(9), rng.randrange(3), rng.choice((0, 1, 65536, 51200, -65536)),
                          rng.choice((0, 1, 0x1dffff, 0x1e0000, 0x1e0001, 0x230000,
                                      0x320000, 0x640000, -0x350000)),
                          rng.choice((0, 0xccc, 0xe665, 0xe666, 0x20000, -0x20000)),
                          rng.randrange(3), rng.randrange(2), rng.randrange(16)))
        emitted = 0
        for case in cases:
            a, b = original.run(target, case), rebuilt.run(target, case)
            if a != b:
                print('FAIL', hex(target), case)
                for x, y in zip(a, b):
                    if x != y:
                        print('original:', x, '\nrebuilt:', y)
                        break
                return 1
            emitted += sum(event[0] == 'particle' for event in a)
        assert emitted > 100, 'emission branches were not exercised'
        print(hex(target), len(cases), 'cases:', emitted, 'identical particles and RNG/wear traces')
    return 0


if __name__ == '__main__':
    sys.exit(main())
