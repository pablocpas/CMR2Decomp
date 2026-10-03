#!/usr/bin/env python3
"""Compare full three-vector frames supplied by impact and part debris callers.

Usage: differential_debris_frames.py entities.json [rebuilt.exe]
Whole callers and fixed-point geometry execute. Selection, clamp, feedback,
damage updates and final debris spawning are controlled leaves. Reading all
36 frame bytes detects accidental reliance on adjacent independent locals.
Surface contacts use differential_surface_collision's strengthened provider.
"""
import itertools
import json
import math
from pathlib import Path
import struct
import sys
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK


class DebrisFrames(Lighting):
    def __init__(self, path, mapping, entities=None):
        super().__init__(path, entities)
        ids = {v[1]: int(k, 16) for k, v in mapping.items()}
        leaves = {
            0x469680: (4, lambda: self.result(HEAP + 0x5000)),
            0x499750: (24, self.debris),
            0x468c10: (4, lambda: self.result(0)),
            0x4074f0: (0, lambda: self.result(0)),
            0x48c6e0: (12, lambda: None),
            0x48c750: (4, lambda: None),
            ids['StageObject_IsEligibleType']: (12, lambda: self.result(not self.blocked)),
            ids['ForceFeedback_UpdateSlot']: (12, lambda: None),
        }
        self.callbacks = {self.addr(k): v for k, v in leaves.items()}
        self.sqrt_table = struct.pack('<4096H', *(math.isqrt((8 + 16 * i) * 65536) for i in range(4096)))

    def result(self, value):
        self.u.reg_write(UC_X86_REG_EAX, int(value))

    def debris(self):
        size, pos, car, axes, count, chance = self.args(6)
        self.trace.append((size, self.read(pos, 12), car,
                           self.read(axes, 36), count, chance))

    def run(self, kind, seed, direction, strength, blocked):
        self.reset()
        self.blocked = blocked
        self.u.mem_write(self.addr(0x6e0ef4), self.sqrt_table)
        car, other = HEAP + 0x1000, HEAP + 0x2000
        normal = (0, 65536, 0) if direction == 0 else (0, 0, 65536)
        self.put(HEAP + 0x4000, '<12i', 65536, 0, 0, 0, 0, 65536, 0, 0, 0, 0, 65536, 0)
        for p in (car, other):
            self.put(p + 0x750, '<I', HEAP + 0x4000)
            self.put(p + 0x75c, '<i', 65536)
            self.put(p + 0x2d0, '<3i', seed * 65536, 0, -seed * 65536)
            self.put(p + 0xb70, '<i', blocked)
        if kind == 'impact':
            self.put(car + 0x988, '<4i', *(0x2000,) * 4)
            self.put(car + 0x9d8, '<4i', *(-0x3000,) * 4)
            self.put(car + 0x9c8, '<4i', *(0x1000,) * 4)
            self.put(car + 0xb74, '<i', not blocked)
            self.put(car + 0x778, '<i', strength)
            self.put(car + 0x48c, '<3i', *normal)
            self.put(car + 0x408, '<3i', strength + seed * 257, 0, strength)
            self.put(car + 0x270, '<3i', (seed + 3) * 65536, 65536, 2 * 65536)
            self.put(car + 0x294, '<3i', seed * 65536, 65536, 2 * 65536)
            entry, args = 0x46a500, [car]
        elif kind == 'cars':
            self.put(car + 0x408, '<3i', strength * 2 + seed * 257, 0, strength)
            self.put(self.addr(0x5915e8), '<3i', *normal)
            entry, args = 0x48ae90, [car, other]
        else:
            self.put(self.addr(0x590d74), '<I', car)
            self.put(self.addr(0x590c20), '<I', HEAP + 0x6000)
            self.put(self.addr(0x590d78), '<I', HEAP + 0x8000)
            self.put(HEAP + 0x6120, '<3i', (seed + 1) * 65536, 0, 65536)
            self.put(HEAP + 0x6000 + 0x17c + 0x20, '<3i', 0, 0, 65536)
            self.put(car + 0x48c, '<3i', *normal)
            self.put(HEAP + 0x5000, '<3i', strength + seed * 257, 0, strength)
            entry, args = 0x483100, [HEAP + 0x5000, 0]
        self.invoke(entry, args)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff04 + len(args) * 4
        return self.trace, self.read(HEAP, 0x10000)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    a = DebrisFrames(ROOT / 'cmr2bin/CMR2.exe', entities)
    b = DebrisFrames(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities, entities)
    count = spawned = 0
    for case in itertools.product(('impact', 'cars', 'part'), range(8), range(2),
                                  (0x3333, 0x8000, 0x10000), range(2)):
        left, right = a.run(*case), b.run(*case)
        if left != right:
            print('FAIL debris frame', case, 'original', left[0], 'rebuilt', right[0])
            return 1
        count += 1
        spawned += len(left[0])
    assert spawned > 0, 'fixtures must reach debris spawning'
    print(count, 'debris callers: identical complete frames, positions, counts, guarded memory and ABI;', spawned, 'spawns checked')
    return 0


if __name__ == '__main__':
    sys.exit(main())
