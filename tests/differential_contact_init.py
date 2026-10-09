#!/usr/bin/env python3
"""Contact initialization with actual game-state, car and handling helpers.

Usage: differential_contact_init.py entities.json [rebuilt.exe]
Checks complete guarded memory, contact flags/grip and all five pointer caches
against an independent model, including untouched slots and real helper traces.
No provider is mocked; fixtures cover counts -1..8, game modes and rally states.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting


TABLES = ((0x592278, 0), (0x5922BC, 1), (0x592344, 4),
          (0x59229C, 8), (0x5922FC, 12))


class Initialize(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        self.initial_image = self.read(self.base, self.size)
        self.observed = {self.addr(a): (a, nargs) for a, nargs in (
            (0x405D80, 0), (0x4074F0, 0), (0x42B5F0, 1),
            (0x457E10, 2), (0x457E00, 1))}

    def hook(self, u, address, size, data):
        if address in self.observed:
            original, nargs = self.observed[address]
            self.trace.append((original, self.args(nargs)))
        super().hook(u, address, size, data)

    def run_init(self, seed, count, mode, rally):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, self.initial_image)
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        contacts = HEAP + 0x1000
        self.put(self.addr(0x592734), "<I", contacts)
        self.put(self.addr(0x592738), "<i", count)
        self.put(self.addr(0x52AFA0) + 0x14, "<I", (mode << 3) | (seed % 8))
        self.put(self.addr(0x52F2AC), "<I", ((seed * 7919) & ~0xC000) | (rally << 14))
        ids = list(range(8))
        rnd.shuffle(ids)
        cars, offsets = [], []
        for i in range(8):
            car = HEAP + 0x4000 + i * 0xC24
            cars.append(car)
            self.put(self.addr(0x53ABA8) + i * 4, "<I", car)
            self.put(car + 0xB1A, "<b", ids[i])
            data = HEAP + 0xC000 + i * 0x100
            self.put(self.addr(0x5429c8) + i * 4, "<I", data)
            offset = 0x20 + ((seed + i * 7) % 10) * 4
            self.put(data + 2, "<H", offset)
            offsets.append(data + offset)
        for address, _ in TABLES:
            self.u.mem_write(self.addr(address), rnd.randbytes(32))
        spans = {address: self.read(self.addr(address) - 16, 64) for address, _ in TABLES}
        before = self.read(HEAP, 0x10000)
        expected = bytearray(before)
        special = mode in (5, 6, 7)
        for i in range(max(count, 0)):
            record = 0x1000 + i * 0x2A4
            struct.pack_into("<5i", expected, record + 0x250, 0x9999, *([65536] * 4))
            struct.pack_into("<2i", expected, record + 0x294, int(special and i != 0), 1)
        trace = [(0x405D80, ())] * (1 if mode == 5 else 2 if mode == 6 else 3)
        if special and count > 0:
            trace.append((0x4074F0, ()))
        for i in range(max(count, 0)):
            trace.extend(((0x42B5F0, (i,)), (0x457E10, (cars[i], 1)),
                          (0x457E00, (ids[i],)), (0x457E00, (ids[i],))))
        self.trace = []
        self.invoke(0x494BB0, [])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 4
        assert self.read(HEAP, 0x10000) == expected, (seed, count, mode, rally, "contact/guard mismatch")
        assert self.trace == trace, (seed, count, mode, rally, "helper trace mismatch")
        # Account for neighbouring cache arrays inside each guarded span.
        writes = {self.addr(address) + i * 4: offsets[ids[i]] + displacement
                  for address, displacement in TABLES for i in range(max(count, 0))}
        for address, initial in spans.items():
            start = self.addr(address) - 16
            wanted = bytearray(initial)
            for pointer, value in writes.items():
                offset = pointer - start
                if 0 <= offset <= len(wanted) - 4:
                    struct.pack_into("<I", wanted, offset, value)
            assert self.read(start, len(wanted)) == wanted, (seed, count, mode, rally, hex(address), "cache/guard mismatch")
        assert self.read(self.addr(0x592734), 4) == struct.pack("<I", contacts)
        assert self.read(self.addr(0x592738), 4) == struct.pack("<i", count)
        return self.read(HEAP, 0x10000), [self.read(self.addr(a), 32) for a, _ in TABLES], self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Initialize(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Initialize(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities)
    count = 0
    for case in itertools.product(range(8), range(-1, 9), (0, 4, 5, 6, 7, 8, 12, 127), range(4)):
        if original.run_init(*case) != rebuilt.run_init(*case):
            print("FAIL contact initialization", case)
            return 1
        count += 1
    print(f"{count} contact initializations: independent guarded records and pointer caches; actual helpers and traces, no mocks")
    return 0


if __name__ == "__main__":
    sys.exit(main())
