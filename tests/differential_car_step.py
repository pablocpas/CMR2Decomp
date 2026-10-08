#!/usr/bin/env python3
"""Compare the nine car-step passes, including all four wheel accumulators.

Physics leaves and terrain queries are controlled providers. The complete pass
scheduler, per-car state copies, resets and helper order run in both images;
rotation into the body frame uses the actual matrix helper.
Usage: differential_car_step.py entities.json [rebuilt.exe]
"""
import itertools
import json
from pathlib import Path
import random
import sys

from unicorn.x86_const import UC_X86_REG_EAX
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import Lighting


class Step(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        leaves = (
            (0x433E80, 3), (0x460C80, 1), (0x469680, 1),
            (0x43B100, 0), (0x43B090, 1), (0x4B8910, 2),
            (0x43B020, 0), (0x493520, 1), (0x432B30, 0),
            (0x4348C0, 0), (0x434140, 0), (0x437DC0, 0),
            (0x4380C0, 0), (0x4781D0, 2), (0x434F50, 0),
            (0x433FD0, 0), (0x46B4E0, 1), (0x438460, 0),
            (0x4340F0, 0), (0x434070, 0), (0x435470, 0),
        )
        for address, count in leaves:
            self.callbacks[self.addr(address)] = (
                count * 4, lambda a=address, n=count: self.provider(a, n))

    def provider(self, address, count):
        args = self.args(count)
        self.trace.append((address, args))
        result = 0
        if address == 0x469680:
            result = HEAP + 0xA000 + args[0] * 0x500
        elif address == 0x4B8910:
            self.put(args[1], '<3h', 17, 23, 31)
            result = 5
        elif address == 0x437DC0:
            # Observe the accumulators immediately before the tyre pass.
            car = int.from_bytes(self.read(self.addr(0x53CC18), 4), 'little')
            self.trace.append(('wheel-accumulators', self.read(car + 0x870, 32)))
        self.u.reg_write(UC_X86_REG_EAX, result)

    def run_step(self, count, seed, reverse):
        self.reset()
        rnd = random.Random(seed)
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        matrix = HEAP + 0x8000
        self.put(matrix, '<12i', 65536, 0, 0, 0, 65536, 0,
                 0, 0, 65536, 0, 0, 0)
        for k in range(count):
            car = HEAP + 0x1000 + k * 0xC24
            self.put(car + 0xB1A, '<b', k)
            self.put(car + 0x750, '<I', matrix)
            self.put(car + 0xBAC, '<8i', *([0] * 8))
            self.put(car + 0x48C, '<3i', 0, 65536, 0)
            self.put(car + 0xBFC, '<i', seed % 2)
            self.put(HEAP + 0xA000 + k * 0x500 + 0x469, '<B', seed % 4)
            self.put(HEAP + 0xA000 + k * 0x500 + 0x46C, '<i', seed % 2)
        order = list(range(count))
        if reverse:
            order.reverse()
        self.put(HEAP + 0x9000, '<' + 'h' * count, *order)
        self.invoke(0x433890, [HEAP + 0x1000, HEAP + 0x9000, count])
        state = b''.join(self.read(self.addr(a), length) for a, length in
                         ((0x53CC18, 4), (0x53CC1C, 4), (0x53C9D4, 16)))
        return self.read(HEAP, 0x10000), state, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Step(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Step(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    total = 0
    for case in itertools.product((0, 1, 2, 8), range(100), (False, True)):
        a, b = original.run_step(*case), rebuilt.run_step(*case)
        if a != b:
            offsets = [hex(i - 0x1000) for i, (x, y) in enumerate(zip(a[0], b[0])) if x != y]
            print('FAIL car-step passes', case, 'car offsets:', offsets[:20])
            return 1
        total += 1
    print(f'{total} car-step cases: identical guarded cars/setup records, all four wheel resets, globals and provider order')
    return 0


if __name__ == '__main__':
    sys.exit(main())
