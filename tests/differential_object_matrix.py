#!/usr/bin/env python3
"""Rebuild an object's matrix with all rotation/position/query helpers real.

Usage: differential_object_matrix.py entities.json [rebuilt.exe]
Independent affine fixed-point model, mirrored axes, game-mode split vectors,
full guarded object/reference/table memory and four 53-bit x87 rounding modes.
Fixtures use distinct buffers, four valid object slots and affine matrices;
this does not exercise the entire stage-object update/render lifecycle.
"""
import itertools
import json
import math
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_FPCW
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting
from differential_auto_steering import signed, target_angle
from matching_entities import entity_address


SINES = [round(math.sin(i * math.tau / 4096) * 65536) for i in range(4096)]


def product(a, b):
    return [signed(sum(signed((a[i * 4 + k] * b[k * 4 + j]) >> 16)
                       for k in range(4))) for i in range(4) for j in range(4)]


def identity():
    return [65536 if i % 5 == 0 else 0 for i in range(16)]


class ObjectMatrix(Lighting):
    def __init__(self, path, entities=None):
        if entities is not None:
            entities = dict(entities)
            entities['0x590d8c'] = [entity_address(entities, 0x590D8C, 0x590D7C),
                                    'g_carPartStateTables.modes']
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        self.put(self.addr(0x6E2EF4), '<4096i', *SINES)
        self.initial_image = self.read(self.base, self.size)
        self.coefficient, = struct.unpack('<d', self.read(self.addr(0x511300), 8))
        self.rotate = self.addr(0x422E70)
        self.position = self.addr(0x4869E0)

    def hook(self, u, address, size, data):
        if address == self.rotate:
            matrix, angle = self.args(2)
            actual = struct.unpack('<16i', self.read(matrix, 64))
            assert list(actual) == self.mirrored, 'matrix before real rotation'
            assert angle & 65535 == self.angle, 'tilt angle before real rotation'
            self.trace.append((0x422E70, angle & 65535))
        elif address == self.position:
            obj, reference = self.args(2)
            assert (obj, reference) == (HEAP + 0x1000, HEAP + 0x3000)
            assert list(struct.unpack('<16i', self.read(obj + 8, 64))) == self.rotated
            self.trace.append((0x4869E0,))
        super().hook(u, address, size, data)

    def run_matrix(self, seed, index, mode, gate, rounding, entry):
        self.u.mem_write(self.base, self.initial_image)
        self.u.mem_write(STACK, bytes(0x10000))
        rnd = random.Random(seed)
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        obj, reference = HEAP + 0x1000, HEAP + 0x3000
        source = identity()
        for row in range(3):
            for column in range(3):
                source[row * 4 + column] = rnd.randrange(-65536, 65537)
        source[12:15] = [rnd.randrange(-0x1000000, 0x1000000) for _ in range(3)]
        self.put(reference, '<16i', *source)
        self.put(obj, '<B', index)
        self.put(self.addr(0x590D8C) + index, '<B', mode)
        owner = (seed + index) % 8
        self.put(self.addr(0x590EC0) + index, '<B', owner)
        self.put(self.addr(0x590D90) + owner * 4, '<I', HEAP + 0x5000)
        split = [[rnd.randrange(-0x100000, 0x100000) for _ in range(3)] for _ in range(5)]
        for i, vector in enumerate(split):
            self.put(HEAP + 0x5000 + i * 12, '<3i', *vector)
        scale = rnd.randrange(-65536, 0x100000)
        self.put(self.addr(0x590DB0) + index * 4, '<i', scale)
        self.put(self.addr(0x52F2AC), '<I', (1 if gate == 0 else 2) << 14)
        self.put(self.addr(0x52AFA0) + 0x14, '<I', (gate >= 2) << 18)
        self.put(self.addr(0x537DD0) + 4, '<I', HEAP + 0x6000)
        self.put(HEAP + 0x6000, '<B', 10 if gate == 2 else 11)
        self.put(self.addr(0x72D67C), '<i', seed)
        self.mirrored = source.copy()
        self.mirrored[0:3] = [x * (1 if mode == 2 else -1) for x in source[8:11]]
        self.mirrored[8:11] = [x * (-1 if mode == 2 else 1) for x in source[0:3]]
        self.angle = target_angle(10 * 65536 if mode == 0 else 0, self.coefficient, rounding) & 65535
        rotation = identity()
        sine = SINES[self.angle & 4095]
        cosine = SINES[(self.angle + 1024) & 4095]
        rotation[5], rotation[6], rotation[9], rotation[10] = cosine, sine, -sine, cosine
        unpositioned = self.mirrored.copy()
        unpositioned[12:15] = [0, 0, 0]
        self.rotated = product(rotation, unpositioned)
        self.rotated[12:15] = self.mirrored[12:15]
        split_mode = (3 if mode == 0 else 4 if mode == 2 else mode) if gate == 3 else mode
        translation = identity()
        translation[12:15] = split[split_mode]
        unpositioned = self.rotated.copy()
        unpositioned[12:15] = [0, 0, 0]
        final = product(translation, unpositioned)
        final[12:15] = [signed(x + y) for x, y in zip(final[12:15], source[12:15])]
        if entry == 0x4869E0:
            self.put(obj + 8, '<16i', *self.rotated)
        before = self.read(HEAP, 0x10000)
        expected = bytearray(before)
        struct.pack_into('<16i', expected, 0x1008, *final)
        struct.pack_into('<6i', expected, 0x1048, scale, 0x1999, 0, 0xA000, 0, 0x10000)
        table_addresses = ((0x590D8C, 4), (0x590EC0, 16), (0x590D90, 32), (0x590DB0, 256))
        tables = [(a, self.read(self.addr(a), n)) for a, n in table_addresses]
        self.trace = []
        self.u.reg_write(UC_X86_REG_FPCW, 0x27F | rounding)
        self.invoke(entry, [obj, reference])
        actual = self.read(HEAP, 0x10000)
        assert actual == expected, (seed, index, mode, gate, rounding, 'guarded object/reference mismatch')
        expected_trace = ([(0x422E70, self.angle)] if entry == 0x486910 else []) + [(0x4869E0,)]
        assert self.trace == expected_trace
        assert self.read(self.addr(0x72D67C), 4) == struct.pack('<i', seed + (2 if entry == 0x486910 else 1))
        assert all(self.read(self.addr(a), len(data)) == data for a, data in tables)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 12
        return actual, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = ObjectMatrix(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = ObjectMatrix(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = 0
    for case in itertools.product(range(32), range(4), range(3), range(4),
                                  (0, 0x400, 0x800, 0xC00), (0x486910, 0x4869E0)):
        if original.run_matrix(*case) != rebuilt.run_matrix(*case):
            print('FAIL object matrix', case)
            return 1
        cases += 1
    print(f'{cases} object matrices: independent affine/tilt/split model, guarded objects and tables, actual helpers and four x87 modes; no mocks')
    return 0


if __name__ == '__main__':
    sys.exit(main())
