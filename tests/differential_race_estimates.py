#!/usr/bin/env python3
"""Estimate unfinished-driver times and ordering using all real helpers.

Usage: differential_race_estimates.py entities.json [rebuilt.exe]
Checks guarded order/position/time tables, finished prefixes, tie adjustments,
zero and signed packed progress, rally summary timing and actual helper calls.
Uses a separate integer arithmetic model; no query/conversion provider is mocked.
Fixtures use valid unique driver lists and nonzero race-distance denominators.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_FPCW
from differential_menu_list import ROOT, STACK
from differential_stage_lighting import Lighting
from matching_entities import entity_address


SPANS = ((0x53E17C, 8), (0x53E184, 8), (0x53D1DA, 8),
         (0x53D1B8, 32), (0x53E190, 48))


def signed(value):
    return ((value + 2**31) % 2**32) - 2**31


def divide(a, b):
    return (abs(a) // abs(b)) * (-1 if (a < 0) != (b < 0) else 1)


class Estimates(Lighting):
    def __init__(self, path, entities=None):
        if entities is not None:
            entities = dict(entities)
            entities["0x542c68"] = [entity_address(entities, 0x542C68, 0x542AE8),
                                   "g_stageArchiveTables.driverCount"]
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        self.initial_image = self.read(self.base, self.size)
        self.observed = {self.addr(a): (a, n) for a, n in (
            (0x458390, 0), (0x406990, 0), (0x421420, 0), (0x4589E0, 1),
            (0x40D4B0, 1), (0x40D480, 1), (0x407E90, 0))}

    def hook(self, u, address, size, data):
        if address in self.observed:
            original, nargs = self.observed[address]
            self.trace.append((original, self.args(nargs)))
        super().hook(u, address, size, data)

    def run_estimates(self, seed, count, initial, summary, rounding):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, self.initial_image)
        self.u.mem_write(STACK, bytes(0x10000))
        for address, size in SPANS:
            self.u.mem_write(self.addr(address) - 16, rnd.randbytes(size + 32))
        # Adjacent globals overlap each other's guard regions in the original
        # image. Initialize logical table contents after all guard buffers so
        # their initial values do not depend on the linker's placement.
        for address, size in SPANS:
            self.u.mem_write(self.addr(address), rnd.randbytes(size))
        self.put(self.addr(0x542C68), "<i", count)
        self.put(self.addr(0x53E18C), "<b", initial)
        laps = 1 + seed * 2
        self.put(self.addr(0x52F2AC), "<I", (laps << 16) | (summary << 27))
        self.put(self.addr(0x538A84), "<i", 80)
        elapsed = (0, 1, 99, 100, 1234, 12345)[seed]
        self.put(self.addr(0x53D1B0), "<i", elapsed)
        order = list(range(8))
        rnd.shuffle(order)
        self.put(self.addr(0x53DDA8), "<8B", *order)
        finished = list(reversed(order[count - initial:count]))
        for i, car in enumerate(finished):
            self.put(self.addr(0x53E17C) + i, "<B", car)
            self.put(self.addr(0x53E184) + car, "<B", i)
        progress = [(0, 1, 2, 64, 64, 65, 32768, 65535)[(i + seed) % 8] for i in range(8)]
        for i, value in enumerate(progress):
            self.put(self.addr(0x542E78) + i * 28 + 0x10, "<H", value)
        before = {a: self.read(self.addr(a) - 16, n + 32) for a, n in SPANS}
        unchanged = self.read(self.addr(0x542E78), 8 * 28)
        writes = {}
        trace = [(0x458390, ()), (0x406990, ()), (0x421420, ())]
        previous = 0
        current_slot = initial
        raw = (elapsed // 100) * 65536 + divide((elapsed % 100) * 65536, 100)
        for car in order[:count]:
            if car in finished:
                continue
            writes[self.addr(0x53E17C) + current_slot] = bytes([car])
            writes[self.addr(0x53E184) + car] = bytes([current_slot])
            writes[self.addr(0x53D1DA) + car] = b"\x01"
            current_slot += 1
            value = divide(signed(progress[car] << 16) * 65536, laps * 80 * 65536)
            trace.append((0x4589E0, (car,)))
            if value > 0:
                scaled = signed((divide(65536 * 65536, value) * raw) >> 16)
                time = divide(signed(scaled + 0x147) * 65536, 0x28F5C28)
                trace.extend(((0x40D4B0, (elapsed,)), (0x40D480, (scaled & 0xFFFFFFFF,))))
            else:
                time = elapsed * 2
            trace.append((0x407E90, ()))
            if summary:
                writes[self.addr(0x53E190) + 11 * 4] = struct.pack("<i", time)
            if previous != 0 and previous == value:
                time += 1
            writes[self.addr(0x53D1B8) + car * 4] = struct.pack("<i", time)
            previous = value
        self.trace = []
        self.u.reg_write(UC_X86_REG_FPCW, 0x27F | rounding)
        self.invoke(0x4483E0, [])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 4
        for address, size in SPANS:
            start = self.addr(address) - 16
            expected = bytearray(before[address])
            for pointer, data in writes.items():
                lo = max(start, pointer)
                hi = min(start + len(expected), pointer + len(data))
                if lo < hi:
                    expected[lo - start:hi - start] = data[lo - pointer:hi - pointer]
            assert self.read(start, len(expected)) == expected, (seed, count, initial, summary, hex(address), "table/guard mismatch")
        assert self.read(self.addr(0x542E78), len(unchanged)) == unchanged
        assert self.read(self.addr(0x53E18C), 1) == bytes([initial])
        assert self.trace == trace, (seed, count, initial, summary, "helper trace mismatch")
        return [self.read(self.addr(a), n) for a, n in SPANS], self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Estimates(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Estimates(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities)
    cases = 0
    for seed, count, summary, rounding in itertools.product(range(6), range(9), (0, 1), (0, 0x400, 0x800, 0xC00)):
        for initial in range(count + 1):
            case = seed, count, initial, summary, rounding
            if original.run_estimates(*case) != rebuilt.run_estimates(*case):
                print("FAIL race estimates", case)
                return 1
            cases += 1
    print(f"{cases} race estimates: independent guarded time/order tables, ties and summary timing; actual helpers and four x87 modes, no mocks")
    return 0


if __name__ == "__main__":
    sys.exit(main())
