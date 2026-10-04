#!/usr/bin/env python3
"""Compare unlock flags with original x86 and an independent byte model.

Usage: differential_unlock_flags.py entities.json [rebuilt.exe]
The entire flag updater executes with no controlled leaves. Adjacent state,
all eight rows and the even/odd final column are checked over random records.
"""
import json
from pathlib import Path
import random
import sys
from unicorn.x86_const import UC_X86_REG_ESP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, STACK


class UnlockFlags(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}

    def run(self, seed):
        self.reset()
        rng = random.Random(seed)
        base = self.addr(0x52afa0)
        self.u.mem_write(base, rng.randbytes(0x3990))
        counts, unlocks = rng.getrandbits(32), rng.getrandbits(32)
        self.put(base + 0x9c, '<I', counts)
        self.put(base + 0xa0, '<I', unlocks)
        expected = bytearray(self.read(base, 0x3990))
        for row in range(8):
            offset = 0x38f8 + row * 11
            if row < 4:
                expected[offset] &= 253
            if row < ((counts >> 8) & 15):
                for column in range(4):
                    expected[offset + column] &= 253
                if row < ((counts >> 12) & 15):
                    for column in range(4, 8):
                        expected[offset + column] &= 253
                    if row < ((counts >> 16) & 15) and counts & 1:
                        for column in range(8, 10):
                            expected[offset + column] &= 253
            if row % 2:
                bit = 1 << (row // 2)
                if unlocks & (bit | (bit << 5) | (bit << 10)):
                    expected[offset + 10] &= 253
            else:
                expected[offset + 10] = 4
        self.invoke(0x406580, [])
        actual = self.read(base, 0x3990)
        assert actual == expected, ('unlock flags model', seed)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff04
        return actual


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = UnlockFlags(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = UnlockFlags(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    for seed in range(512):
        assert original.run(seed) == rebuilt.run(seed)
    print('512 unlock-flag fixtures: original and rebuilt match independent byte model, adjacent state and ABI')
    return 0


if __name__ == '__main__':
    sys.exit(main())
