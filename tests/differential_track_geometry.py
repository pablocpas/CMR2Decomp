#!/usr/bin/env python3
"""Compare ray/quad results and quadtree traversal with original machine code.

Usage: differential_track_geometry.py entities.json [rebuilt.exe]
RayQuad executes real fixed-point helpers. FindTriangle's final nearest-triangle
provider is controlled: this test checks traversal and its full arguments, not
the provider's triangle intersection. Complete guarded heaps are compared.
"""
import json
from pathlib import Path
import random
import struct
import sys

from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP


class Geometry(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))

    def run_ray(self, points, direction):
        self.u.mem_write(HEAP, bytes([0xA5]) * 0x10000)
        self.u.mem_write(STACK, bytes(0x10000))
        for index, point in enumerate(points):
            self.put(self.addr(0x591438) + index * 12, "<3i", *point)
        self.put(HEAP + 0x1000, "<3i", *direction)
        self.invoke(0x489060, [HEAP + 0x1000, HEAP + 0x2000, HEAP + 0x2100])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 16
        return self.u.reg_read(UC_X86_REG_EAX), self.read(HEAP, 0x10000)

    def nearest(self):
        point, output, y, count, triangle_list = self.args(5)
        # The provider takes a short; upper argument-slot bits are unspecified.
        count = (count & 0xFFFF) - (0x10000 if count & 0x8000 else 0)
        self.trace.append((self.read(point, 12), output, y, count, triangle_list))
        self.put(output, "<h", 123)
        self.u.reg_write(UC_X86_REG_EAX, 0x12345678)

    def run_tree(self, seed, depth, outside):
        self.u.mem_write(HEAP, bytes([0x5A]) * 0x10000)
        self.u.mem_write(STACK, bytes(0x10000))
        self.trace = []
        self.callbacks = {self.addr(0x4916A0): (20, self.nearest)}
        self.put(self.addr(0x591AF8), "<I", HEAP + 0x1200)
        self.put(HEAP + 0x1200, "<2i", -0x2000000, 0x1000000)
        self.put(self.addr(0x591B20), "<I", HEAP + 0x1220)
        self.put(self.addr(0x591B1C), "<I", HEAP + 0x1224)
        self.put(HEAP + 0x1220, "<h", 4)
        self.put(HEAP + 0x1224, "<h", 3)
        randomizer = random.Random(seed)
        dx = randomizer.randrange(4 << 24)
        dz = randomizer.randrange(3 << 24)
        if outside == 1:
            dx = -1
        elif outside == 2:
            dx = 4 << 24
        elif outside == 3:
            dz = -1
        elif outside == 4:
            dz = 3 << 24
        self.put(HEAP + 0x1000, "<3i", dx - 0x2000000, seed, dz + 0x1000000)
        self.put(self.addr(0x591B18), "<I", HEAP + 0x8000)
        for level in range(6):
            table = HEAP + 0x2000 + level * 0x800
            self.put(self.addr(0x591B00) + level * 4, "<I", table)
            for node in range(32):
                self.put(
                    table + node * 8,
                    "<hhi",
                    -1 if level < depth else node + 1,
                    0,
                    (node % 2) * 16 if level < depth else node * 3 + level * 96,
                )
        self.invoke(0x491550, [HEAP + 0x1000, HEAP + 0x1100, seed & 0xFFFFFFFF])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 16
        if outside or depth >= 6:
            assert self.trace == [] and self.u.reg_read(UC_X86_REG_EAX) == 0
        else:
            cell = (dz >> 24) * 4 + (dx >> 24)
            mask, shift = 0xFFFFFF, 24
            for _ in range(depth):
                dx &= mask
                dz &= mask
                mask >>= 2
                shift -= 2
                cell = (dx >> shift) + (cell % 2) * 16 + (dz >> shift) * 4
            assert len(self.trace) == 1
            assert self.trace[0][3:] == (
                cell + 1,
                HEAP + 0x8000 + (cell * 3 + depth * 96) * 2,
            )
        return self.u.reg_read(UC_X86_REG_EAX), self.trace, self.read(HEAP, 0x10000)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Geometry(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Geometry(
        Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities
    )
    randomizer = random.Random(873)
    hits = misses = count = 0
    for case in range(1800):
        if case % 10 == 0:
            points = [(0, 0, 0)] * 4
        else:
            points = [
                (
                    randomizer.randrange(-8, 9) << 16,
                    0,
                    randomizer.randrange(-8, 9) << 16,
                )
                for _ in range(4)
            ]
        direction = (
            randomizer.randrange(-0x10000, 0x10001),
            0,
            randomizer.randrange(-0x10000, 0x10001),
        )
        left, right = original.run_ray(points, direction), rebuilt.run_ray(
            points, direction
        )
        if left != right:
            print("FAIL ray/quad", case, points, direction, hex(left[0]), hex(right[0]))
            return 1
        hits += left[0] != 0x7D000000
        misses += left[0] == 0x7D000000
        count += 1
    trees = 0
    for seed in range(12):
        for depth in range(7):
            for outside in range(5):
                case = seed, depth, outside
                left, right = original.run_tree(*case), rebuilt.run_tree(*case)
                if left != right:
                    print("FAIL quadtree", case, left[:2], right[:2])
                    return 1
                trees += 1
    assert hits and misses
    print(
        f"{count} ray/quad cases ({hits} hits, {misses} misses), {trees} quadtree cases: identical returns, outputs, provider arguments and guards"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
