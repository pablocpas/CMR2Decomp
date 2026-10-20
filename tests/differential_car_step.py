#!/usr/bin/env python3
"""Compare car-step scheduling and the complete rigid-body integration pass.

Physics leaves and terrain queries are controlled providers. The complete pass
scheduler, per-car state copies, resets and helper order run in both images;
rotation into the body frame uses the actual matrix helper.
The integrator path executes its real vector, basis and corner helpers, with
whole-heap/car/setup guards, matrix alias cases and a zero-time-step model.
Usage: differential_car_step.py entities.json [rebuilt.exe]
"""
import itertools
import json
import math
from pathlib import Path
import random
import sys

from unicorn.x86_const import UC_X86_REG_EAX
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import Lighting
from differential_stage_lighting import mul, signed
from differential_car_info_records import Records, put


class Integrator(Records):
    """Run the complete rigid-body step and its real vector/corner helpers."""

    def integrate(self, yaw_only, basis_index, settled, skip_offsets, alias_matrix, step, seed):
        h = self.begin(seed)
        car, parts = HEAP + 0x1000, HEAP + 0x5000
        matrix = car if alias_matrix else HEAP + 0x7000
        h[car-HEAP:car-HEAP+0xc24] = bytes(0xc24)
        h[parts-HEAP:parts-HEAP+0x4d0] = bytes(0x4d0)
        bases = (((65536,0,0),(0,65536,0),(0,0,65536)),
                 ((0,0,-65536),(0,65536,0),(65536,0,0)),
                 ((65536,0,0),(0,46341,46341),(0,-46341,46341)))
        basis = bases[basis_index]
        position = (seed*307, -seed*113, seed*211)
        for i, row in enumerate(basis):
            put(h, car+0x360+12*i, '<3i', *row)
            put(h, matrix+16*i, '<3i', *row)
        put(h, matrix+0x30, '<3i', *position)
        put(h, car+0x750, '<I', matrix)
        put(h, car+0x2d0, '<3i', *position)
        put(h, car+0x204, '<3i', 65536,32768,98304)
        put(h, car+0x48c, '<3i', 0,65536,0)
        put(h, car+0x75c, '<2i', 65536,65536)
        put(h, car+0x3fc, '<3i', 32768,24576,16384)
        velocity = (seed*317, -seed*23, seed*541)
        angular = (0 if yaw_only else seed+1,seed*31,0 if yaw_only else -2*(seed+1))
        put(h, car+0x408, '<3i', *velocity)
        put(h, car+0x420, '<3i', *angular)
        put(h, car+0x778, '<i', seed*601)
        put(h, car+0xb34, '<b', settled)
        put(h, car+0xb64, '<i', skip_offsets)
        put(h, car+0x1d0, '<4b', seed%2,0,0,0)
        put(h, car+0x764, '<3i', seed*7,seed*11,seed*13)
        rng = random.Random(seed)
        for i in range(8):
            local = (65536 if i%4<2 else -65536,32768 if i>=4 else -32768,
                     98304 if i%2==0 else -98304)
            world = tuple(signed(position[k]+sum(mul(basis[j][k],local[j]) for j in range(3)))
                          for k in range(3))
            put(h, car+0x270+12*i, '<3i', *world)
            put(h, car+0x648+12*i, '<3i', *(rng.randrange(-8192,8193) if seed else 0 for _ in range(3)))
            if i<4:
                put(h, car+0x210+12*i, '<3i', local[0],0,local[2])
        put(h, car+0x6a8, '<3i', seed*17,-seed*19,seed*23)
        put(h, car+0x5c4, '<3i', seed*3,-seed*5,seed*7)
        put(h, car+0x5d0, '<3i', seed,-seed*2,seed*3)
        put(h, parts+0x408, '<i', seed*101)
        self.put(self.addr(0x53cc18), '<I', car)
        self.put(self.addr(0x53cc1c), '<I', parts)
        self.put(self.addr(0x519c8c), '<i', step)
        self.put(self.addr(0x519c88), '<i', 65536)
        self.put(self.addr(0x6e0ef4), '<4096H',
                 *(math.isqrt((8+16*i)*65536) for i in range(4096)))
        self.allowed.add((self.addr(0x53cc18),4))
        self.native_input = bytes(h)
        memory = self.invoke_guarded(0x440840, [])
        expected = bytearray(h)
        for start,end in ((0x270,0x2dc),(0x360,0x384),(0x408,0x42c),(0x778,0x77c),(0xb60,0xb64)):
            expected[car-HEAP+start:car-HEAP+end] = memory[car-HEAP+start:car-HEAP+end]
        for i in range(4):
            at = matrix-HEAP+16*i
            expected[at:at+12] = memory[at:at+12]
        assert memory == bytes(expected), 'integrator modified setup, forces, matrix padding or unrelated car state'
        assert self.read(self.addr(0x53cc18),4) == car.to_bytes(4,'little')
        assert memory[car-HEAP+0x414:car-HEAP+0x420] == bytes(h[car-HEAP+0x408:car-HEAP+0x414]), 'previous velocity snapshot'
        if step == 0:
            for start,end in ((0x2d0,0x2dc),(0x360,0x384),(0x408,0x414),(0x420,0x42c)):
                assert memory[car-HEAP+start:car-HEAP+end] == bytes(h[car-HEAP+start:car-HEAP+end]), 'zero-step motion'
        return memory


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
    left = Integrator(ROOT / 'cmr2bin/CMR2.exe')
    right = Integrator(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe', entities)
    integration_count = 0
    for case in itertools.product((0,1),range(3),(0,1),(0,1),(0,1),(0,16384,65536),(0,17)):
        assert left.integrate(*case) == right.integrate(*case), ('complete rigid-body integration',case)
        integration_count += 1
    print(f'{total} car-step cases and {integration_count} complete rigid-body integrations: guarded cars/setup/forces/matrices, zero-step model, real geometry/fixed-point helpers and ABI')
    return 0


if __name__ == '__main__':
    sys.exit(main())
