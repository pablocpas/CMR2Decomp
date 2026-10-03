#!/usr/bin/env python3
"""Restore saved car state using actual matrix/corner helpers, without mocks.

Usage: differential_car_restore.py entities.json [rebuilt.exe]
Compare the whole guarded heap, saved input, matrix and current-car pointer.
Fixtures include both suspension-offset branches and aliased/distinct inputs.
"""
import itertools
import json
import math
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting


class Restore(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}

    def run_restore(self, seed, suspension, aliased):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(STACK, bytes(0x10000))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        car, saved, world = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x5000
        if aliased:
            saved = car
        self.put(car + 0x750, "<I", world)
        self.put(world, "<16i", 65536, 0, 0, 0, 0, 65536, 0, 0, 0, 0, 65536, 0,
                 *(rnd.randrange(-0x1000000, 0x1000000) for _ in range(3)), 65536)
        self.put(car + 0x204, "<3i", *(rnd.randrange(0, 5 * 65536) for _ in range(3)))
        self.put(car + 0xB64, "<i", suspension)
        self.put(car + 0x764, "<3i", *(rnd.randrange(-65536, 65537) for _ in range(3)))
        self.put(car + 0x2D0, "<3i", *(rnd.randrange(-0x1000000, 0x1000000) for _ in range(3)))
        self.put(car + 0x2E8, "<3i", *(rnd.randrange(-0x1000000, 0x1000000) for _ in range(3)))
        for offset in (0x408, 0x420, 0x360, 0x36C, 0x378, 0x48C):
            self.put(car + offset, "<3i", *(rnd.randrange(-65536, 65537) for _ in range(3)))
        angle = seed * math.pi / 120
        c, s = int(math.cos(angle) * 65536), int(math.sin(angle) * 65536)
        self.put(saved, "<16i", c, 0, -s, 0, 0, 65536, 0, 0, s, 0, c, 0, 0, 0, 0, 65536)
        self.put(saved + 0x70, "<6i", *(rnd.randrange(-0x200000, 0x200000) for _ in range(6)))
        self.put(saved + 0xB8, "<i", rnd.randrange(-65536, 65537))
        self.put(saved + 0xC4, "<H", rnd.randrange(65536))
        self.put(saved + 0xCC, "<B", rnd.randrange(256))
        self.put(saved + 0xD0, "<2i", rnd.randrange(-100, 101), seed % 3)
        before = self.read(HEAP, 0x10000)
        self.invoke(0x426D80, [car, saved])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 12
        after = self.read(HEAP, 0x10000)
        # Only the destination car and its world matrix may change.
        assert after[:0x1000] == before[:0x1000]
        assert after[0x1C24:0x5000] == before[0x1C24:0x5000]
        assert after[0x5040:] == before[0x5040:]
        assert self.read(self.addr(0x53CC18), 4) == struct.pack("<I", car)
        assert after[0x1384:0x13A8] == before[0x1360:0x1384]
        assert after[0x1414:0x1420] == before[0x1408:0x1414]
        source = saved - HEAP
        assert after[0x1408:0x1414] == before[source + 0x70:source + 0x7C]
        assert after[0x1420:0x142C] == before[source + 0x7C:source + 0x88]
        assert after[0x15C4:0x15DC] == bytes(24)
        assert after[0x1C20:0x1C24] == struct.pack("<i", 1)
        return after


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Restore(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Restore(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities)
    cases = itertools.product(range(240), (0, 1), (False, True))
    count = 0
    for case in cases:
        left, right = original.run_restore(*case), rebuilt.run_restore(*case)
        if left != right:
            offsets = [hex(i - 0x1000) for i, (a, b) in enumerate(zip(left, right)) if a != b]
            print("FAIL restore", case, "offsets", offsets[:20])
            return 1
        count += 1
    print(f"{count} saved-state restores: identical guarded car, matrix and input; actual helpers, both suspension branches and aliased inputs")
    return 0


if __name__ == "__main__":
    sys.exit(main())
