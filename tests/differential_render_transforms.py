#!/usr/bin/env python3
"""Refresh physics/shadow transforms and wheels with every helper real.

Usage: differential_render_transforms.py entities.json [rebuilt.exe]
Independent guarded-memory model, both passes, repeated/permuted car lists,
setup IDs distinct from list slots, suspension clamps and four x87 modes.
Fixtures use eight valid cars/setup rows and type indices 0..13; no rendering
or full physics/frame lifecycle is simulated.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_FPCW
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting
from differential_auto_steering import signed, target_angle


ROW = 0xFC


def mul(a, b):
    return signed((a * b) >> 16)


def get32(buffer, offset):
    return struct.unpack_from('<i', buffer, offset)[0]


def put32(buffer, offset, value):
    struct.pack_into('<i', buffer, offset, signed(value))


class RenderTransforms(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        self.initial_image = self.read(self.base, self.size)
        self.coefficient, = struct.unpack('<d', self.read(self.addr(0x511300), 8))
        self.observed = {self.addr(a): (a, n) for a, n in
                         ((0x456BE0, 1), (0x4063F0, 1), (0x469680, 1))}

    def hook(self, u, address, size, data):
        if address in self.observed:
            original, nargs = self.observed[address]
            self.trace.append((original, self.args(nargs)))
        super().hook(u, address, size, data)

    def run_transforms(self, seed, count, ride_enabled, rounding):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, self.initial_image)
        self.u.mem_write(STACK, bytes(0x10000))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        for address in (0x53A3A8, 0x53AD10):
            self.u.mem_write(self.addr(address) - 16, rnd.randbytes(8 * ROW + 32))
        # Populate logical rows last so overlapping linker layouts cannot
        # influence their initial data through neighbouring guard regions.
        for address in (0x53A3A8, 0x53AD10):
            self.u.mem_write(self.addr(address), rnd.randbytes(8 * ROW))
        self.put(self.addr(0x53C9A4), '<I', HEAP + 0x1000)
        self.put(self.addr(0x52AFA0) + 0x20, '<I', ride_enabled << 22)
        ids = list(range(8))
        rnd.shuffle(ids)
        order = list(range(8))
        rnd.shuffle(order)
        if seed % 3 == 0:
            order[1::2] = order[0::2]
        self.put(HEAP + 0xD000, '<8h', *order)
        factors = [rnd.randrange(-0x30000, 0x30001) for _ in range(14)]
        extra = [rnd.randrange(-0x30000, 0x30001) for _ in range(14)]
        self.put(self.addr(0x5199C8), '<14i', *factors)
        self.put(self.addr(0x519A00), '<14i', *extra)
        types, subtypes, setups, suspensions, rotations = [], [], [], [], []
        for i in range(8):
            typ = (seed + i * 3) % 14
            types.append(typ)
            self.put(self.addr(0x542630) + i * 0x24, '<B', typ)
            setup = HEAP + 0xA000 + i * 0x400
            self.put(self.addr(0x588990) + i * 4, '<I', setup)
            values = [(0, 65535, 65536, 65537, 131072, -65536, -2**31, 2**31-1)[(seed + i + j) % 8]
                      for j in range(4)]
            setups.append(values)
            self.put(setup + 0x240, '<4i', *values)
            travel = [values[(j + 2) % 4] for j in range(4)]
            suspensions.append(travel)
            self.put(setup + 0x250, '<4i', *travel)
            spins = [(-32768, -1, 0, 1, 32767)[(seed + i + j) % 5] for j in range(4)]
            rotations.append(spins)
            self.put(self.addr(0x53A230) + i * 8, '<4h', *spins)
        for i in range(8):
            car = HEAP + 0x1000 + i * 0xC24
            node = HEAP + 0x8000 + i * 0x200
            self.put(car + 0xB1A, '<b', ids[i])
            subtype = types[ids[i]] if (seed + i) % 2 else (types[ids[i]] + 1) % 14
            subtypes.append(subtype)
            self.put(car + 0xB1B, '<B', subtype)
            self.put(car + 0xB70, '<i', (seed + i) % 2)
            self.put(car + 0x958, '<i', (0, -65536, -1, 65536)[(seed + i) % 4])
            self.put(car + 0xA8C, '<i', rnd.randrange(-0x20000, 0x20001))
            self.put(car + 0xB14, '<h', (-32768, -1, 0, 1, 32767)[(seed + i) % 5])
            self.put(car + 0x71C, '<I', node)
            for base in (0, 0x40):
                self.put(car + base, '<16i', *(rnd.randrange(-0x1000000, 0x1000000) for _ in range(16)))
            self.put(node + 0xA8, '<3i', *(rnd.randrange(-65536, 65537) for _ in range(3)))
            self.put(car + 0x978, '<4i', *(rnd.randrange(-0x18000, 0x18001) for _ in range(4)))
            for offset, n in ((0x3C0, 12), (0x48C, 3), (0x8DC, 4), (0x6FC, 8), (0x928, 4), (0x938, 4)):
                self.put(car + offset, '<' + 'i' * n, *(rnd.randrange(-0x20000, 0x20001) for _ in range(n)))
        expected_heap = bytearray(self.read(HEAP, 0x10000))
        expected_physics = bytearray(self.read(self.addr(0x53A3A8), ROW * 8))
        expected_shadow = bytearray(self.read(self.addr(0x53AD10), ROW * 8))
        spans = {a: self.read(self.addr(a) - 16, ROW * 8 + 32) for a in (0x53A3A8, 0x53AD10)}
        tables = [(a, self.read(self.addr(a), n)) for a, n in
                  ((0x5199C8, 56), (0x519A00, 56), (0x53A230, 64), (0x588990, 32), (0x542630, 8 * 0x24))]
        trace = []
        selected = list(reversed(order[:max(count, 0)]))
        for i in selected:
            car, row = 0x1000 + i * 0xC24, i * ROW
            expected_physics[row:row + ROW] = expected_shadow[row:row + ROW]
            height = get32(expected_heap, car + 0x958)
            for offset in (0x34, 0x74):
                put32(expected_heap, car + offset, get32(expected_heap, car + offset) + height)
            factor = factors[subtypes[i]] + (extra[types[ids[i]]] if types[ids[i]] != subtypes[i] else 0)
            up = [mul(get32(expected_heap, car + 0x10 + j * 4), factor) for j in range(3)]
            if ride_enabled:
                node = get32(expected_heap, car + 0x71C) - HEAP
                ride = mul(get32(expected_heap, car + 0xA8C), 0x8000)
                up = [signed(v + mul(get32(expected_heap, node + 0xA8 + j * 4), ride)) for j, v in enumerate(up)]
            for base in (0, 0x40):
                for component in (0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14):
                    put32(expected_shadow, row + base + component * 4,
                          get32(expected_heap, car + base + component * 4))
                for j in range(3):
                    offset = row + base + 0x30 + j * 4
                    put32(expected_shadow, offset, get32(expected_shadow, offset) + up[j])
            expected_shadow[row + 0xE0:row + 0xEC] = expected_heap[car + 0x48C:car + 0x498]
            expected_shadow[row + 0xEC:row + ROW] = expected_heap[car + 0x8DC:car + 0x8EC]
            trace.extend(((0x456BE0, (ids[i],)), (0x4063F0, (6,))))
        for i in selected:
            car, row = 0x1000 + i * 0xC24, i * ROW
            special = get32(expected_heap, car + 0xB70) != 0
            if not special:
                trace.append((0x469680, (i,)))
            for wheel in range(3, -1, -1):
                dest = row + 0x80 + wheel * 24
                source = car + 0x3C0 + wheel * 12
                expected_shadow[dest:dest + 12] = expected_heap[source:source + 12]
                if special:
                    continue
                angle = target_angle(mul(0x50000, setups[i][wheel]), self.coefficient, rounding)
                steering, = struct.unpack_from('<h', expected_heap, car + 0xB14)
                struct.pack_into('<3i', expected_shadow, dest + 12,
                                 angle * 0x1680, steering * 0x1680 if wheel < 2 else 0,
                                 rotations[i][wheel] * 0x1680)
                total = min(signed(suspensions[ids[i]][wheel] + get32(expected_heap, car + 0x978 + wheel * 4)), 65536)
                x = get32(expected_shadow, dest) + get32(expected_heap, car + 0x6FC + wheel * 8)
                y = (get32(expected_shadow, dest + 4) + get32(expected_heap, car + 0x928 + wheel * 4) +
                     mul(signed(65536 - total), get32(expected_heap, car + 0x938 + wheel * 4)) +
                     get32(expected_heap, car + 0x700 + wheel * 8))
                put32(expected_shadow, dest, x)
                put32(expected_shadow, dest + 4, y)
                trace.append((0x469680, (ids[i],)))
        self.trace = []
        self.u.reg_write(UC_X86_REG_FPCW, 0x27F | rounding)
        self.invoke(0x42AF50, [HEAP + 0xD000, count & 0xFFFFFFFF])
        actual = self.read(HEAP, 0x10000)
        assert actual == expected_heap, (seed, count, ride_enabled, rounding, 'car/input/guard mismatch')
        assert self.trace == trace, (seed, count, ride_enabled, rounding, 'real helper trace mismatch')
        writes = {self.addr(0x53A3A8): bytes(expected_physics), self.addr(0x53AD10): bytes(expected_shadow)}
        for a, before in spans.items():
            start = self.addr(a) - 16
            expected = bytearray(before)
            for pointer, data in writes.items():
                lo, hi = max(start, pointer), min(start + len(expected), pointer + len(data))
                if lo < hi:
                    expected[lo - start:hi - start] = data[lo - pointer:hi - pointer]
            actual_row = self.read(start, len(expected))
            assert actual_row == expected, (seed, count, ride_enabled, rounding, hex(a), 'row/guard mismatch',
                [(hex(i - 16), x, y) for i, (x, y) in enumerate(zip(actual_row, expected)) if x != y][:16])
        assert all(self.read(self.addr(a), len(data)) == data for a, data in tables)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 12
        return actual, self.read(self.addr(0x53A3A8), ROW * 8), self.read(self.addr(0x53AD10), ROW * 8), self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = RenderTransforms(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = RenderTransforms(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = 0
    for case in itertools.product(range(24), range(-1, 9), (0, 1), (0, 0x400, 0x800, 0xC00)):
        if original.run_transforms(*case) != rebuilt.run_transforms(*case):
            print('FAIL render transforms', case)
            return 1
        cases += 1
    print(f'{cases} render-transform cases: independent two-pass matrices/wheels model, guarded cars/rows/tables, repeated lists and four x87 modes; actual helpers, no mocks')
    return 0


if __name__ == '__main__':
    sys.exit(main())
