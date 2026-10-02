#!/usr/bin/env python3
"""Compare complete CPU telemetry with the original, using curved route fixtures.

Usage: differential_ai_telemetry.py entities.json [rebuilt.exe]
Route providers and atan2 are shared leaves. Coordinate selection, all masks,
angle wrapping, reference interpolation and fixed-point arithmetic run for real.
"""
import itertools
import json
import math
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP


class Telemetry(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {
            self.addr(0x421430): (0, self.count),
            self.addr(0x421530): (8, self.node),
            self.addr(0x498d80): (8, self.angle),
        }

    def count(self):
        self.u.reg_write(UC_X86_REG_EAX, len(self.nodes))

    def node(self):
        index, output = self.args(2)
        assert index < len(self.nodes)
        self.put(output, '<3i', *self.nodes[index])

    def angle(self):
        y, x = struct.unpack('<2i', self.read(self.u.reg_read(UC_X86_REG_ESP) + 4, 8))
        self.trace.append((y, x))
        self.u.reg_write(UC_X86_REG_EAX, int(math.atan2(y, x) * 57.295827908797776 * 65536) & 0xffffffff)

    def run(self, seed, mask, variant):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, bytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        self.put(self.addr(0x6e0ef4), '<4096H',
                 *(int(math.sqrt((8 + 16*i) / 65536) * 65536) for i in range(4096)))
        self.put(self.addr(0x6e2ef4), '<4096i',
                 *(int(math.sin(i * math.tau / 4096) * 65536) for i in range(4096)))
        self.nodes = [(int(math.cos(i * math.tau / 12) * 12 * 65536), 0,
                       int(math.sin(i * math.tau / 12) * 9 * 65536)) for i in range(12)]
        car, output, reference = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x4000
        node_index = seed % 12
        x, _, z = self.nodes[node_index]
        self.put(car + 0x2d0, '<3i', x + rnd.randint(-65536, 65536), 0, z + rnd.randint(-65536, 65536))
        self.put(car + 0x360, '<3i', rnd.randint(-65536, 65536), 0, rnd.randint(-65536, 65536))
        self.put(car + 0x408, '<3i', rnd.randint(-65536, 65536), 0, rnd.randint(-65536, 65536))
        self.put(car + 0x778, '<i', rnd.randint(0, 0x20000))
        self.put(car + 0x870, '<i', rnd.randint(-65536, 65536))
        self.put(car + 0xb10, '<h', rnd.randint(-200, 200))
        self.put(output + 0x54, '<i', node_index)
        for i in range(1, 4):
            self.put(self.addr(0x592744) + 4*i, '<I', reference)
        for i, (x, _, z) in enumerate(self.nodes):
            self.put(reference + i*4, '<i', rnd.randint(-0xb40000, 0xb40000))
            self.put(reference + 0x5c8 + i*4, '<i', rnd.randint(0x100000, 0x640000))
            self.put(reference + 0xb90 + i*4, '<i', x + 65536)
            self.put(reference + 0x1158 + i*4, '<i', z - 65536)
        sp = STACK + 0xff00
        self.put(sp, '<5I', STOP, car, mask, output, variant)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(self.addr(0x498620), STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP
        return self.read(HEAP, 0x10000), self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Telemetry(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Telemetry(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = 0
    for seed, mask, variant in itertools.product(range(24), [0, 3, 0xfffffffc, 0xffffffff], [1, 2, 3]):
        a, b = original.run(seed, mask, variant), rebuilt.run(seed, mask, variant)
        if a != b:
            print('FAIL telemetry', seed, hex(mask), variant)
            print('output offsets:', [hex(i-0x3000) for i,(x,y) in enumerate(zip(a[0],b[0])) if x != y][:20])
            print('angle inputs original:', a[1], 'rebuilt:', b[1])
            return 1
        cases += 1
    print(f'{cases} CPU telemetry cases: identical route angles, complete state and angle inputs')
    return 0


if __name__ == '__main__':
    sys.exit(main())
