#!/usr/bin/env python3
"""Direct car bounding corners and suspension offsets with real helpers.

Usage: differential_car_corners.py entities.json [rebuilt.exe]
Independent signed integer geometry model, full guarded heap, both branches,
global current-car identity, callee-saved registers and argument cleanup.
Car/world buffers are distinct; a whole driving simulation is outside scope.
"""
import itertools
import json
from pathlib import Path
import struct
import sys

from differential_fixed_matrix_ops import FixedOps, values, signed, mul
from differential_menu_list import ROOT, HEAP


SIGNS = ((1, -1, 1), (1, -1, -1), (-1, -1, 1), (-1, -1, -1),
         (1, 1, 1), (1, 1, -1), (-1, 1, 1), (-1, 1, -1))


def offsets(expected, car):
    right = struct.unpack_from('<3i', expected, car + 0x360)
    forward = struct.unpack_from('<3i', expected, car + 0x378)
    scales = struct.unpack_from('<3i', expected, car + 0x764)
    for corner, side, front in ((4, -1, -1), (5, -1, 1), (6, 1, -1), (7, 1, 1)):
        current = struct.unpack_from('<3i', expected, car + 0x270 + corner * 12)
        a = [mul(right[j], scales[0 if side < 0 else 1]) for j in range(3)]
        b = [mul(forward[j], scales[2]) for j in range(3)]
        struct.pack_into('<3i', expected, car + 0x270 + corner * 12,
                         *(signed(current[j] + side * a[j] + front * b[j]) for j in range(3)))


class Corners(FixedOps):
    def run(self, seed, suspension, standalone):
        self.prepare(seed)
        car, world = HEAP + 0x1000, HEAP + 0x3000
        self.put(car + 0x750, '<I', world)
        self.put(car + 0x204, '<3i', *values(seed, 3, 1))
        self.put(car + 0x2d0, '<3i', *values(seed, 3, 3))
        self.put(car + 0x360, '<9i', *values(seed, 9, 5))
        self.put(car + 0x764, '<3i', *values(seed, 3, 7))
        self.put(car + 0xb64, '<i', suspension)
        self.put(world, '<16i', *values(seed, 16, 9))
        pointer = self.addr(0x53cc18)
        self.put(pointer, '<I', car if standalone else HEAP + 0x5000)
        before = bytearray(self.read(HEAP, 0x10000))
        expected = before.copy()
        if not standalone:
            half = struct.unpack_from('<3i', before, 0x1204)
            position = struct.unpack_from('<3i', before, 0x12d0)
            matrix = struct.unpack_from('<16i', before, 0x3000)
            for corner, signs in enumerate(SIGNS):
                components = [signed(position[j] + sum(signs[k] * mul(matrix[k * 4 + j], half[k])
                                  for k in range(3))) for j in range(3)]
                struct.pack_into('<3i', expected, 0x1270 + corner * 12, *components)
        if standalone or suspension == 0:
            offsets(expected, 0x1000)
        self.allowed_globals = set() if standalone else {(pointer, 4)}
        self.checked_invoke(0x43f280 if standalone else 0x43eef0, [] if standalone else [car])
        actual = self.read(HEAP, 0x10000)
        assert actual == expected, ('car corners model/guards', seed, suspension, standalone)
        assert self.read(pointer, 4) == struct.pack('<I', car), 'current car identity'
        return actual


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    path = ROOT / 'cmr2bin/CMR2.exe'
    original = Corners(path)
    rebuilt = Corners(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0
    for case in itertools.product(range(192), (0, 1, -1), (False, True)):
        assert original.run(*case) == rebuilt.run(*case), ('differential corners', case)
        count += 1
    mutant = Corners(path)
    memory = bytearray(mutant.memory)
    offset = 0x43ef11 - mutant.base
    assert memory[offset:offset + 6] == bytes.fromhex('8b810c020000')
    memory[offset + 2] = 8  # z half-extent read from y instead
    mutant.memory = bytes(memory)
    try:
        mutant.run(2, 0, False)
    except AssertionError as error:
        assert 'model/guards' in str(error), error
    else:
        raise AssertionError('wrong half-extent offset survived')
    print(count, 'corner/offset cases: real helpers, independent geometry, complete heap, branches and ABI identical; wrong-offset mutation rejected')
    return 0


if __name__ == '__main__':
    sys.exit(main())
