#!/usr/bin/env python3
"""Checkpoint progression and ordered event calls against an independent model.

Usage: differential_checkpoint_advance.py entities.json [rebuilt.exe]
The complete progression entry executes, with event/update callees controlled.
Covers both directions, loop wrap, +/-50 thresholds, bounded traversal, signed
lap counters and unsigned high-water marks. Callee implementations are excluded.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_menu_list import ROOT, STACK
from differential_stage_lighting import Lighting


class Advance(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x458F30, 3), (0x4590A0, 2), (0x458FD0, 2),
                               (0x459180, 1), (0x459250, 3)):
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs: self.event(a, n))

    def event(self, address, nargs):
        self.trace.append((address, self.args(nargs)))
        for register, value in ((UC_X86_REG_EAX, 0x12345678),
                                (UC_X86_REG_ECX, 0xA5A5A5A5),
                                (UC_X86_REG_EDX, 0x5A5A5A5A)):
            self.u.reg_write(register, value)

    def run_advance(self, seed, count, looped, alternate, car, target_kind):
        self.reset()
        rnd = random.Random(seed)
        base = self.addr(0x542E78)
        self.u.mem_write(base - 16, rnd.randbytes(8 * 28 + 32))
        current = (0, count // 2, count - 1)[seed % 3]
        target = (current, 0, count - 1, count + 1)[target_kind]
        progress = (-32768, -1, 0, 1, 32767)[seed % 5]
        maximum = (0, 1, 32767, 32768, 65535)[seed % 5]
        self.put(base + car * 28, "<h", current)
        self.put(base + car * 28 + 0x10, "<Hh", maximum, progress)
        self.put(self.addr(0x542C70), "<i", count)
        self.put(self.addr(0x542CAE), "<B", looped)
        self.put(self.addr(0x542CAD), "<B", alternate)
        expected = bytearray(self.read(base - 16, 8 * 28 + 32))
        trace = []
        if target != current:
            delta = target - current
            step = (1 if delta > 0 else -1) if not looped else (
                1 if delta < -50 or 0 < delta < 50 else -1)
            for _ in range(count):
                if current == target:
                    break
                progress = ((progress + step + 32768) % 65536) - 32768
                if progress > maximum:
                    maximum = progress
                prev = current
                current += step
                if current < 0:
                    current = count - 1
                if current >= count:
                    current = 0
                if looped:
                    trace.append((0x458F30, (car, prev, current)))
                trace.extend(((0x4590A0 if alternate else 0x458FD0, (car, current)),
                              (0x459180, (car,)),
                              (0x459250, (car, current, step & 0xFFFFFFFF))))
            struct.pack_into("<h", expected, 16 + car * 28, target)
            struct.pack_into("<Hh", expected, 16 + car * 28 + 0x10, maximum, progress)
            expected[16 + car * 28 + 0x16] = 1
        self.invoke(0x458E00, [car, target])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 12
        actual = self.read(base - 16, len(expected))
        assert actual == expected, (seed, count, looped, alternate, car, target_kind, "record/guard mismatch")
        assert self.trace == trace, (seed, count, looped, alternate, car, target_kind, "event mismatch")
        return actual, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Advance(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Advance(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities)
    count = 0
    for case in itertools.product(range(8), (1, 2, 9, 50, 51, 100, 101, 127),
                                  (0, 1), (0, 1), (0, 3, 7), range(4)):
        if original.run_advance(*case) != rebuilt.run_advance(*case):
            print("FAIL checkpoint progression", case)
            return 1
        count += 1
    print(f"{count} checkpoint fixtures: identical guarded records and event order; independent progression model, controlled callees")
    return 0


if __name__ == "__main__":
    sys.exit(main())
