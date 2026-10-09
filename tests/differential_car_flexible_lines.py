#!/usr/bin/env python3
"""Guarded car line states: reset, spring integration and ground resolution.

Original/rebuilt execute real fixed-point and rotation helpers. Terrain is a
controlled leaf. Independent integer models check complete heap writes, slot
selection, early returns, ground-query arguments and the callee ABI.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP,
                              UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP)
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from differential_body_patch import SQRT, mul, scale, dot
from differential_stage_lighting import add, signed
from differential_ground_contact import normalized

CAR, PARTS, WORLD = HEAP + 0x1000, HEAP + 0x2000, HEAP + 0x3000
DESCRIPTORS, STATES, POINTERS = HEAP + 0x5000, HEAP + 0x6000, HEAP + 0x7000
SAVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def sub(a, b):
    return signed(a - b)


class FlexibleLines(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {self.addr(0x490c90): (20, self.ground)}

    def ground(self):
        point, normal, triangle, surface, default_y = self.args(5)
        self.trace.append((struct.unpack('<3i', self.read(point, 12)), default_y))
        self.put(normal, '<3i', *self.normal)
        self.put(triangle, '<h', 7)
        self.put(surface, '<H', 3)
        self.u.reg_write(UC_X86_REG_EAX, self.height & 0xffffffff)

    def fixture(self, seed, car, index, grounded):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        self.put(self.addr(0x6e0ef4), '<4096H', *SQRT)
        self.put(self.addr(0x590d74), '<I', CAR)
        self.put(self.addr(0x590d78), '<I', PARTS)
        self.put(self.addr(0x590c6c), '<I', POINTERS)
        self.put(CAR + 0xb1a, '<B', car)
        self.put(CAR + 0x750, '<I', WORLD)
        self.car_position = [rnd.randrange(-0x40000, 0x40001) for _ in range(3)]
        self.put(CAR + 0x2d0, '<3i', *self.car_position)
        matrix = [0, 0, 65536, 0, 0, 65536, 0, 0, -65536, 0, 0, 0, 0, 0, 0, 65536]
        self.put(WORLD, '<16i', *matrix)
        for c in range(8):
            self.put(POINTERS + 4*c, '<I', STATES + 0x100*c)
            self.put(self.addr(0x590c00) + 4*c, '<I', DESCRIPTORS + 0x100*c)
        self.state = STATES + car*0x100 + index*0x3c
        self.descriptor = DESCRIPTORS + car*0x100 + index*0x20
        self.values = [rnd.randrange(-0x30000, 0x30001) for _ in range(14)] + [grounded]
        self.local = [rnd.randrange(-0x40000, 0x40001) for _ in range(3)]
        self.axis = [[65536, 0, 0], [0, 65536, 0], [0, 0, 65536]][seed % 3]
        self.length = rnd.randrange(0x1000, 0x20000)
        self.put(self.state, '<15i', *self.values)
        self.put(self.descriptor, '<7i4B', *self.local, *self.axis, self.length, 40, 80, 120, 160)
        self.step = [0, 4096, 32768, 65536][seed % 4]
        self.impulse = [rnd.randrange(-0x20000, 0x20001) for _ in range(3)]
        self.put(self.addr(0x519c8c), '<i', self.step)
        self.put(self.addr(0x590b50), '<3i', *self.impulse)
        self.trace = []
        self.before = self.read(HEAP, 0x10000)
        self.global_before = [(a, self.read(self.addr(a), n)) for a, n in
                              [(0x590d74, 4), (0x590d78, 4), (0x590c6c, 4),
                               (0x590c00, 32), (0x590b50, 12), (0x519c8c, 4)]]

    def invoke(self, entry, args):
        sp = STACK + 0xff00
        self.put(sp, '<' + 'I'*(len(args)+1), STOP, *(v & 0xffffffff for v in args))
        self.u.reg_write(UC_X86_REG_ESP, sp)
        for i, register in enumerate(SAVED):
            self.u.reg_write(register, 0x13570000 + i)
        self.u.emu_start(self.addr(entry), STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 4 + 4*len(args)
        assert all(self.u.reg_read(r) == 0x13570000+i for i, r in enumerate(SAVED))
        assert all(self.read(self.addr(a), len(data)) == data for a, data in self.global_before)

    def run(self, kind, seed, car, index, flag, normal_index=0):
        self.fixture(seed, car, index, flag)
        expected = bytearray(self.before)
        values = self.values.copy()
        expected_trace = []
        if kind == 'reset':
            values = [0]*14 + [flag]
            self.invoke(0x486630, [car, index, flag])
        elif kind == 'spring':
            if not flag:
                values[3:6] = values[0:3]
                projected = scale(self.axis, dot(self.impulse, self.axis))
                impulse = [sub(a, b) for a, b in zip(self.impulse, projected)]
                for velocity, position, component in [(12, 0, 0), (13, 2, 2)]:
                    values[velocity] = sub(values[velocity], impulse[component])
                    force = sub(0, add(mul(0x20000, values[position]),
                                       mul(0x3333, values[velocity])))
                    values[velocity] = add(values[velocity], mul(self.step, force))
                    values[position] = add(values[position], mul(self.step, values[velocity]))
                values[1] = 65536
            self.invoke(0x4854a0, [index | (seed << 8)])
        else:
            self.normal = [(0, 65536, 0), (65536, 0, 0), (0, 0, 65536)][normal_index]
            self.height = -0x10000 + 0x1000*seed
            if not flag:
                world = [-self.local[2], self.local[1], self.local[0]]
                position = [add(a, b) for a, b in zip(world, self.car_position)]
                expected_trace = [(tuple(position), position[1] & 0xffffffff)]
                position[1] = self.height
                direction = [-self.axis[2], self.axis[1], self.axis[0]]
                projection = scale(self.normal, dot(direction, self.normal))
                tangent = [sub(a, b) for a, b in zip(direction, projection)]
                if not any(tangent):
                    tangent = [sub(a, b) for a, b in zip([65536, 0, 0],
                              scale(self.normal, self.normal[0]))]
                if any(tangent):
                    tangent, _ = normalized(tangent)
                tangent = scale(tangent, self.length)
                values[0:3] = position
                values[9:12] = [add(a, b) for a, b in zip(position, tangent)]
                values[14] = 1
                struct.pack_into('<i', expected, PARTS-HEAP + 0x4c0 + 4*index, 1)
            self.invoke(0x484f40, [index | (seed << 8)])
        struct.pack_into('<15i', expected, self.state-HEAP, *values)
        actual = self.read(HEAP, 0x10000)
        assert actual == expected, (kind, seed, car, index, flag, normal_index, 'guarded heap/model')
        assert self.trace == expected_trace, (kind, 'terrain trace', self.trace, expected_trace)
        return actual, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = FlexibleLines(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = FlexibleLines(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = 0
    for kind in ['reset', 'spring', 'ground']:
        normals = range(3) if kind == 'ground' else range(1)
        for case in itertools.product(range(12), range(8), range(3), [0, 1, -1], normals):
            assert original.run(kind, *case) == rebuilt.run(kind, *case), (kind, case)
            cases += 1
    print(f'{cases} car line cases: independent integer models, guarded records/heap, real helpers, terrain leaf and callee ABI')
    return 0


if __name__ == '__main__':
    sys.exit(main())
