#!/usr/bin/env python3
"""Compare complete lighting-file relocation across the 64 KiB zone boundary.

The fixtures contain real zone/item/header records, with no sectors or meshes.
Only the two zero-size allocation requests are supplied by controlled leaves.
"""
import json
from pathlib import Path
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK

DATA = HEAP + 0x10000


class LightZones(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.u.mem_map(DATA, 0x200000)
        self.callbacks = {self.addr(0x4aad70): (4, self.allocate)}

    def allocate(self):
        assert self.args(1) == (0,)
        self.trace.append(self.args(1))
        self.u.reg_write(UC_X86_REG_EAX, HEAP + 0x9000 + len(self.trace) * 0x100)

    def run(self, count):
        self.reset()
        self.u.mem_write(DATA, bytes(0x200000))
        self.put(self.addr(0x72d568), '<I', 0)
        self.put(DATA + 48, '<H', count)
        zones = DATA + 50
        items = zones + count * 20
        # One item in each of the last two zones: its initial lighting scale is
        # a distinct sentinel so skipping either item cannot pass comparison.
        for i in (count - 2, count - 1):
            self.put(zones + i * 20 + 2, '<H', 1)
        for i in range(2):
            self.put(items + i * 48 + 32, '<I', 0x12345678 + i)
        before = self.read(items, 96)
        self.invoke(0x4b4aa0, [DATA])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff00 + 8
        expected = bytearray(before)
        expected[32:36] = expected[80:84] = b'\x00\x00\x01\x00'
        after = self.read(items, 96)
        # Record relocation has to preserve the complete input, except pointer
        # fields in zone/header records and each item's unity lighting scale.
        assert self.read(DATA, 48) == bytes(48)
        for i in range(count):
            assert self.read(zones + i * 20, 2) == b'\xff\xff'
            assert self.read(zones + i * 20 + 4, 8) == bytes(8)
        return self.read(DATA, 50 + count * 40 + 96), after == expected, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = LightZones(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = LightZones(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = (2, 16, 256, 3276, 3277, 3278, 4096)
    for count in cases:
        a, b = original.run(count), rebuilt.run(count)
        assert a[1], ('original did not initialize both items', count)
        if a != b:
            print('FAIL light zone offsets:', count, 'zones; item initialization:', a[1], b[1])
            return 1
    print(f'{len(cases)} lighting files: identical relocation and item initialization across 64 KiB')
    return 0


if __name__ == '__main__':
    sys.exit(main())
