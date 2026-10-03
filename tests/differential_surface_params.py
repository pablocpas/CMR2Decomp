#!/usr/bin/env python3
"""Compare all eight grip records, four drag records and noise smoothing.

Usage: differential_surface_params.py entities.json [rebuilt.exe]
Runs the complete surface update with the real fixed-point helpers and tables;
no provider is mocked. Fixtures use all 48 surfaces and valid compression,
blend and drag values. The complete guarded heap is compared.
"""
import itertools
import json
from pathlib import Path
import random
import sys

from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK
from unicorn.x86_const import UC_X86_REG_ESP


class Surfaces(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))

    def run_surface(self, surface, blend, compression, drag):
        randomizer = random.Random(surface * 17 + compression * 5 + drag)
        heap = randomizer.randbytes(0x10000)
        self.u.mem_write(HEAP, heap)
        self.u.mem_write(STACK, bytes(0x10000))
        car = HEAP + 0x1000
        for index in range(8):
            self.put(car + 0xAAE + index * 2, "<h", (surface + index * 7) % 48)
            self.put(car + 0x880 + index * 4, "<i", compression)
            self.put(car + 0x8A0 + index * 4, "<i", compression)
        self.put(car + 0xB29, "<B", drag)
        self.put(car + 0xA74, "<i", 0 if surface % 2 else 0x30000)
        self.invoke(0x4781D0, [car, blend])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 12
        output = self.read(HEAP, 0x10000)
        assert output[:0x1000] == heap[:0x1000]
        assert output[0x1000 + 0xC24 :] == heap[0x1000 + 0xC24 :]
        return output


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Surfaces(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Surfaces(
        Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities
    )
    count = 0
    for case in itertools.product(
        range(48),
        [0, 0x4000, 0xFFFF, 0x10000],
        [0, 0x8000, 0x10000, 0x18000],
        [0, 3, 6],
    ):
        left, right = original.run_surface(*case), rebuilt.run_surface(*case)
        if left != right:
            at = next(i for i, (a, b) in enumerate(zip(left, right)) if a != b)
            print(
                "FAIL surface parameters",
                case,
                "car offset",
                hex(at - 0x1000),
                "original",
                left[at : at + 8].hex(),
                "rebuilt",
                right[at : at + 8].hex(),
            )
            return 1
        count += 1
    print(
        f"{count} surface parameter cases: identical grip, drag, noise and complete guarded heap; real helpers"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
