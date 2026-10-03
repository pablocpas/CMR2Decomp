#!/usr/bin/env python3
"""Compare complete car route updates, including their actual route helpers.

Usage: differential_car_route_update.py entities.json [rebuilt.exe]
No callees are mocked. Fixtures cover probes, cache replacement, forward/backward
movement, open/closed routes, long distances and the 25-cycle refresh boundary.
Full records, probe/cache globals, stack cleanup and guarded heap are compared.
"""
import itertools
import json
import math
from pathlib import Path
import random
import sys

from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK
from unicorn.x86_const import UC_X86_REG_ESP


class Routes(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        self.put(
            self.addr(0x6E0EF4),
            "<4096H",
            *(int(math.sqrt((8 + 16 * i) / 65536) * 65536) for i in range(4096)),
        )

    def run_route(self, slot, node, offset, closed, flags, cycles, missing=False):
        r = random.Random(slot + node * 7 + offset * 13 + flags + cycles)
        heap = r.randbytes(0x10000)
        self.u.mem_write(HEAP, heap)
        self.u.mem_write(STACK, bytes(0x10000))
        car, nodes = HEAP + 0x1000, HEAP + 0x3000
        spacing = 120 if node == 6 else 40
        self.put(self.addr(0x52F2AC), "<I", (2 << 14) | flags)
        self.put(self.addr(0x538A84), "<i", 0 if missing else 8)
        self.put(self.addr(0x538A88), "<i", 8)
        self.put(self.addr(0x538A94), "<i", closed)
        self.put(self.addr(0x538A9C), "<I", nodes)
        self.put(self.addr(0x538A8C), "<3h", -1, -1, -1)
        self.put(self.addr(0x538AA0), "<h", 0)
        self.u.mem_write(self.addr(0x538B68), bytes([0xA5]) * 36)
        for index in range(8):
            self.put(
                nodes + index * 0x2C,
                "<3i",
                (index * spacing) << 16,
                0,
                ((index % 3) * 20) << 16,
            )
            self.put(
                self.addr(0x538AA8) + index * 24,
                "<5i2h",
                node,
                7,
                9,
                0x100000,
                0x3000,
                node,
                77,
            )
        for index in range(2):
            self.put(self.addr(0x538A78) + index * 4, "<I", 0x7FFFFFFF)
            self.put(self.addr(0x538A80) + index * 2, "<H", 7)
            self.put(self.addr(0x538A98) + index * 2, "<h", 2)
            self.put(self.addr(0x538C8C) + index * 4, "<i", cycles)
        self.put(car + 0xB1A, "<b", slot)
        self.put(
            car + 0x2D0,
            "<3i",
            (node * spacing + offset) << 16,
            0,
            ((node % 3) * 20) << 16,
        )
        self.invoke(0x420A30, [car])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 8
        output = self.read(HEAP, 0x10000)
        assert (
            output
            == heap[:0x1000]
            + output[0x1000 : 0x1000 + 0xC24]
            + heap[0x1000 + 0xC24 : 0x3000]
            + output[0x3000:0x3160]
            + heap[0x3160:]
        )
        globals_ = [
            self.read(self.addr(address), size)
            for address, size in (
                (0x538AA8, 192),
                (0x538B68, 36),
                (0x538A8C, 6),
                (0x538AA0, 2),
                (0x538A78, 8),
                (0x538A80, 4),
                (0x538A98, 4),
                (0x538C8C, 8),
            )
        ]
        return output, globals_


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Routes(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Routes(
        Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities
    )
    count = 0
    cases = itertools.product(
        (0, 1, 7),
        (0, 1, 6, 7),
        (0, 10, -10, 200, -200),
        range(2),
        (0, 1 << 24, 1 << 25, 1 << 27),
        (0, 25),
    )
    for case in cases:
        left, right = original.run_route(*case), rebuilt.run_route(*case)
        if left != right:
            print("FAIL car route update", case)
            for address, a, b in zip(
                (
                    0x538AA8,
                    0x538B68,
                    0x538A8C,
                    0x538AA0,
                    0x538A78,
                    0x538A80,
                    0x538A98,
                    0x538C8C,
                ),
                left[1],
                right[1],
            ):
                if a != b:
                    print(
                        "global", hex(address), "original", a.hex(), "rebuilt", b.hex()
                    )
            return 1
        count += 1
    assert original.run_route(0, 0, 0, 0, 0, 0, True) == rebuilt.run_route(
        0, 0, 0, 0, 0, 0, True
    )
    print(
        f"{count+1} complete car route cases: identical records, probes, direction cache and guarded heap; no mocked callees"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
