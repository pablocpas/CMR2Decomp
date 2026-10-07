#!/usr/bin/env python3
"""Compare converted wheel buffers and their subsequent session cleanup.

Both full x86 bodies execute. Allocation/release and mode queries are controlled;
fixed-point normalisation executes normally. Compare every converted byte and
allocation/release, including cleanup's count read from the shared mesh record.
"""
import itertools
import json
import math
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP, UC_X86_REG_FPCW
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting

RECORDS, BLOCKS, FLAGS = 0x82d220, 0x831198, 0x82d1dc
SQRT = struct.pack('<4096H', *(math.isqrt((8 + 16 * i) * 65536) for i in range(4096)))


class Vertices(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x4aad70, 1), (0x4aade0, 1), (0x405d10, 0), (0x405d70, 0)):
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        args = self.args(nargs)
        value = 0
        if address == 0x4aad70:
            size, = args
            value = self.next
            self.next += ((size + 15) & ~15) + 32
            assert self.next < HEAP + 0xff00
            self.allocations.append((value, size))
            self.trace.append(('allocate', size, value))
            self.u.mem_write(value, bytes([0xa5]) * size)
        elif address == 0x4aade0:
            pointer, = args
            assert pointer in {p for p, _ in self.allocations}, 'release outside allocations'
            assert pointer not in self.released, 'double release'
            self.released.add(pointer)
            self.trace.append(('release', pointer))
        elif address == 0x405d10:
            value = 1  # Wheel restoration is unrelated to converted vertex ownership.
        elif address == 0x405d70:
            value = 2
        self.u.reg_write(UC_X86_REG_EAX, value)
        self.u.reg_write(UC_X86_REG_ECX, 0xa5a5a5a5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5a5a5a5a)

    def setup(self, slot, counts, seed, rounding):
        self.reset()
        self.allocations, self.released, self.trace = [], set(), []
        self.next = HEAP + 0x9000
        self.u.mem_write(self.addr(0x6e0ef4), SQRT)
        self.u.reg_write(UC_X86_REG_FPCW, 0x27f | rounding)
        rng = random.Random(seed)
        record = self.addr(RECORDS) + slot * 0x2ac
        self.put(record + 0x26a, '<B', len(counts))
        for mesh, count in enumerate(counts):
            meshptr, node, source = HEAP + mesh * 0x100, HEAP + 0x2000 + mesh * 0x100, HEAP + 0x4000 + mesh * 0x400
            self.put(record + mesh * 4, '<I', meshptr)
            self.put(record + 0x3c + mesh * 4, '<I', node)
            self.put(record + 0x24c + mesh * 2, '<H', count)
            self.put(meshptr + 0xc, '<I', source)
            self.put(node + 0x30, '<B', 31 + mesh)
            for vertex in range(count):
                values = [rng.randrange(-0x30000, 0x30000) / 65536 for _ in range(6)]
                self.put(source + vertex * 48, '<6f', *values)
                normals = ((128, 128, 128), (0, 0, 0), (255, 255, 255),
                           (128, 128, 255), (1, 127, 129), (0, 255, 128))
                self.put(source + vertex * 48 + 24, '<4B', *normals[(seed + vertex + mesh) % len(normals)], 0x5a)
        return record

    def invoke_checked(self, address, args):
        self.invoke(address, args)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff00 + (len(args) + 1) * 4

    def snapshot(self):
        return (self.read(HEAP, 0x10000), list(self.trace),
                self.read(self.addr(BLOCKS), 64), self.read(self.addr(FLAGS), 64))

    def lifecycle(self, slot, counts, seed, rounding):
        record = self.setup(slot, counts, seed, rounding)
        self.invoke_checked(0x506bb0, [slot, 0, record])
        converted = self.snapshot()
        self.invoke_checked(0x505f10, [])
        assert self.u.reg_read(UC_X86_REG_EAX) == 1
        released = self.snapshot()
        return converted, released, len(self.allocations), len(self.released)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    a = Vertices(ROOT / 'cmr2bin/CMR2.exe')
    b = Vertices(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = vertices = releases = 0
    configurations = ((1,), (), (0,), (0, 1, 7), (7, 1, 0, 4), (1,) * 15)
    for slot, counts, rounding in itertools.product((0, 1, 7, 8, 15), configurations, (0, 0x400, 0x800, 0xc00)):
        case = slot, counts, cases + 73, rounding
        aa, bb = a.lifecycle(*case), b.lifecycle(*case)
        if aa != bb:
            print('FAIL', case)
            print('converted equal:', aa[0] == bb[0])
            print('original releases:', aa[3], 'rebuilt releases:', bb[3])
            print('original trace:', aa[1][1], '\nrebuilt trace:', bb[1][1])
            return 1
        assert aa[2] == aa[3], 'not every allocation was released'
        vertices += sum(counts)
        releases += aa[3]
        cases += 1
    print(cases, 'conversion/cleanup lifecycles:', vertices, 'vertices and', releases,
          'releases identical, with all allocations released')
    return 0


if __name__ == '__main__':
    sys.exit(main())
