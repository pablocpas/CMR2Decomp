#!/usr/bin/env python3
"""Race order passes with actual comparator/count/checkpoint helpers.

Usage: differential_race_order.py entities.json [rebuilt.exe]
Checks guarded order/timing records against both images and an independent
adjacent-pass model. Covers ties, signed progress, byte counter wrap and repeated
passes; does not claim to validate complete race classification.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_ESP
from differential_menu_list import ROOT, STACK
from differential_stage_lighting import Lighting
from matching_entities import entity_address


class Order(Lighting):
    def __init__(self, path, entities=None):
        if entities is not None:
            entities = dict(entities)
            # Stage.h exposes driverCount as a member of StageArchiveTables.
            entities["0x542c68"] = [entity_address(entities, 0x542C68, 0x542AE8),
                                   "g_stageArchiveTables.driverCount"]
        super().__init__(path, entities)
        self.callbacks = {}

    def run_order(self, seed, count, passes):
        self.reset()
        rnd = random.Random(seed)
        timing, checkpoints = self.addr(0x53D968), self.addr(0x542E78)
        # Guarded regions are separate symbols in the rebuilt image.
        self.u.mem_write(timing - 16, rnd.randbytes(8 * 136 + 32))
        self.u.mem_write(self.addr(0x53DDA8), rnd.randbytes(8))
        self.u.mem_write(checkpoints - 16, rnd.randbytes(8 * 28 + 32))
        self.put(self.addr(0x542C68), "<i", count)
        order = list(range(8))
        rnd.shuffle(order)
        self.put(self.addr(0x53DDA8), "<8B", *order)
        groups = [(0 if seed % 4 == 0 else i if seed % 4 == 1 else
                   7 - i if seed % 4 == 2 else
                   rnd.choice((-32768, -1, 0, 1, 32767))) for i in range(8)]
        for i in range(8):
            self.put(checkpoints + i * 28 + 0x12, "<h", groups[i])
            self.put(timing + i * 136, "<i", rnd.choice((-1000, 0, 1000)))
            self.put(timing + i * 136 + 0x81, "<B", rnd.choice((0, 1, 7, 127, 128, 255)))
        before = self.read(timing - 16, 8 * 136 + 32)
        untouched = self.read(checkpoints - 16, 8 * 28 + 32)
        expected = bytearray(before)
        counters = [before[16 + i * 136 + 0x81] for i in range(8)]
        for _ in range(passes):
            for i in range(1, count):
                a, b = order[i - 1], order[i]
                # The real comparator only returns +1 for greater progress.
                if groups[b] > groups[a]:
                    counters[a] = (counters[a] + 1) & 255
                    counters[b] = (counters[b] - 1) & 255
                    order[i - 1], order[i] = b, a
            self.invoke(0x448D50, [])
            assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 4
        for i, value in enumerate(counters):
            expected[16 + i * 136 + 0x81] = value
        # Original symbols are adjacent; rebuilt symbols need not be.
        order_offset = self.addr(0x53DDA8) - (timing - 16)
        if 0 <= order_offset <= len(expected) - 8:
            expected[order_offset:order_offset + 8] = bytes(order)
        actual = self.read(timing - 16, len(expected))
        assert actual == expected, (seed, count, passes, "timing/guard mismatch")
        assert self.read(self.addr(0x53DDA8), 8) == bytes(order)
        assert self.read(checkpoints - 16, len(untouched)) == untouched
        assert self.read(self.addr(0x542C68), 4) == struct.pack("<i", count)
        # Return only symbol contents, since neighbouring linked globals differ.
        return actual[16:16 + 8 * 136], bytes(order)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Order(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Order(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities)
    count = 0
    for case in itertools.product(range(96), range(9), (1, 2, 8)):
        if original.run_order(*case) != rebuilt.run_order(*case):
            print("FAIL race order", case)
            return 1
        count += 1
    print(f"{count} race-order fixtures: identical guarded timing records, order and independent model; actual helpers")
    return 0


if __name__ == "__main__":
    sys.exit(main())
