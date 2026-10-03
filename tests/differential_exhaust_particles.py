#!/usr/bin/env python3
"""Original/rebuilt exhaust particles across complete consecutive frames.

Usage: differential_exhaust_particles.py entities.json [rebuilt.exe]
Spawn, motion, exhaust attachment, interpolation and draw traversal run for
real. Only rand and final billboard submission are controlled. Moving cars
and fractional render times expose feedback from drawing into simulation.
"""
import itertools
import json
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP, UC_X86_REG_FPCW
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK


class ExhaustParticles(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {
            self.addr(0x4c6676): (0, self.random),
            self.addr(0x4b11c0): (8, self.billboard),
        }

    def random(self):
        self.seed = (214013 * self.seed + 2531011) & 0xffffffff
        self.u.reg_write(UC_X86_REG_EAX, (self.seed >> 16) & 32767)

    def billboard(self):
        definition, texture = self.args(2)
        # Only bits 0 and 1 of flags are defined by Particle_DrawAll; the
        # other bits and field_0x24 are unused stack contents.
        data = bytearray(self.read(definition, 0x24))
        data[0x23] &= 3
        self.trace.append(('billboard', bytes(data), texture))

    def call(self, address, args):
        self.invoke(address, args)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff04 + 4 * len(args), 'stdcall cleanup'

    def run(self, car, twin_exhaust, rotation, fraction, attached, rounding, physics):
        self.reset()
        self.seed = 42
        self.u.reg_write(UC_X86_REG_FPCW, 0x27f | rounding)
        types, particles = HEAP + 0x1000, HEAP + 0x2000
        cars, matrices = HEAP + 0x3000, HEAP + 0xa000
        self.put(self.addr(0x53c9a4), '<I', cars)
        self.put(self.addr(0x53aba8), '<8I', *(cars + i * 0xc24 for i in range(8)))
        self.put(self.addr(0x6a2cd0), '<I', 4)
        self.put(self.addr(0x6a2cd4), '<I', 2)
        self.put(self.addr(0x6a2cd8), '<I', types)
        self.put(self.addr(0x6a2cdc), '<I', particles)
        self.put(self.addr(0x6a2ce4), '<I', 0)
        self.put(self.addr(0x543cf8), '<I', 0)
        self.u.mem_write(particles - 16, bytes([0xa5]) * (2 * 0x68 + 32))
        for i in range(2):
            self.put(particles + i * 0x68 + 0x57, '<B', 0)
        matrix = matrices + car * 0x40
        self.put(cars + car * 0xc24 + 0x754, '<I', matrix)
        rotations = [
            (65536, 0, 0, 0, 65536, 0, 0, 0, 65536),
            (0, 0, -65536, 0, 65536, 0, 65536, 0, 0),
            (-65536, 0, 0, 0, 65536, 0, 0, 0, -65536),
            (0, 65536, 0, -65536, 0, 0, 0, 0, 65536),
        ]
        axes = rotations[rotation]
        self.put(matrix, '<12i', axes[0], axes[1], axes[2], 0,
                 axes[3], axes[4], axes[5], 0, axes[6], axes[7], axes[8], 0)
        self.put(matrix + 0x30, '<3i', 20 * 65536, 3 * 65536, -40 * 65536)
        self.put(HEAP + 0xe000, '<3i', -65536, 0x4000, -2 * 65536)
        self.put(HEAP + 0xe010, '<3i', 65536, 0x4000, -2 * 65536)
        self.u.mem_write(self.addr(0x543580), bytes(64))
        self.put(self.addr(0x543580) + car * 8, '<2I', HEAP + 0xe000,
                 HEAP + 0xe010 if twin_exhaust else 0)
        typ = types + 3 * 0x70
        self.put(typ, '<i', 0x40000)
        self.put(typ + 0x20, '<3i', 0xccc, 0x2147, 0xccc)
        self.put(typ + 0x2e, '<6B', 30, 0, 3, 255, 128, 64)
        self.put(typ + 0x35, '<B', 0x49)
        if physics:
            self.put(typ + 0x10, '<4i', 0x800, 0x1000, 0x8000, 0x2000)
            self.put(self.addr(0x6a2ce8), '<3i', 0x1000, -0x400, 0x800)
            if physics >= 2:
                self.put(typ + 0x35, '<B', 0x49 | (0x80 if physics == 2 else 2))
        self.put(typ + 0x38, '<I', HEAP + 0xf000)
        self.put(typ + 0x3c, '<4i', -65536, -65536, 65536, 65536)
        self.put(typ + 0x64, '<2I', self.addr(0x45d2d0) if attached else 0,
                 self.addr(0x45de80))
        self.put(HEAP + 0xe100, '<3i', 0, 0, 0)
        self.put(HEAP + 0xe110, '<3i', 0x800, -0x100, 0x400)
        self.put(HEAP + 0xe120, '<i', car)
        for _ in range(1 + twin_exhaust):
            self.call(0x4b07a0, [3, HEAP + 0xe100, HEAP + 0xe110,
                               ((-0x640000) & 0xffffffff) if physics < 2 else 0,
                               0, 0, 0, HEAP + 0xe120, 1])
        snapshots = [self.read(particles, 2 * 0x68)]
        for frame in range(4):
            self.put(matrix + 0x30, '<3i', (20 + frame) * 65536,
                     (3 + frame) * 65536, (-40 + 2 * frame) * 65536)
            self.call(0x4b0110, [0])
            self.call(0x4b06a0, [fraction])
            self.call(0x4b0480, [0, 0])
            snapshots.append(self.read(particles, 2 * 0x68))
        assert self.read(particles - 16, 16) == bytes([0xa5]) * 16
        assert self.read(particles + 2 * 0x68, 16) == bytes([0xa5]) * 16
        return snapshots, self.trace, self.seed


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = ExhaustParticles(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = ExhaustParticles(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0
    for case in itertools.product(range(8), range(2), range(4), [0, 0x8000, 0x10000],
                                  range(2), [0, 0x400, 0x800, 0xc00], range(4)):
        left, right = original.run(*case), rebuilt.run(*case)
        if left != right:
            print('FAIL exhaust lifecycle', case)
            for frame, (a, b) in enumerate(zip(left[0], right[0])):
                if a != b:
                    differences = [(hex(i), a[i], b[i]) for i in range(len(a)) if a[i] != b[i]]
                    print('frame', frame, 'particle differences', differences[:12])
                    break
            print('draw positions original/rebuilt:',
                  [struct.unpack('<3i', x[1][:12]) for x in left[1]],
                  [struct.unpack('<3i', x[1][:12]) for x in right[1]])
            return 1
        count += 1
    print(count, 'particle lifecycles: identical exhaust attachment, moving-car drawing, motion, interpolation, RNG and guards')
    return 0


if __name__ == '__main__':
    sys.exit(main())
