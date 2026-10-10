#!/usr/bin/env python3
"""Guarded common-geometry clamp and complete car/preview deformation bodies.

Independent signed clamp, mesh search and visibility models check every heap
byte and ABI. The full radial/planar car and preview consumers also execute,
including their real clamp and fixed-point helpers; only mesh-upload/rendering
leaves are controlled. Compare their full heaps, globals and upload ordering.
"""
import itertools
import json
import math
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                              UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_FPCW)
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting, signed

RECORD, CAR, BOX, NODES = HEAP + 0x1000, HEAP + 0x2000, HEAP + 0x3000, HEAP + 0x4000
MESHES, FIXED, FLOATS, OUT = HEAP + 0x6000, HEAP + 0x7000, HEAP + 0x9000, HEAP + 0xC000
SAVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
SQRT = struct.pack('<4096H', *(math.isqrt((8 + 16 * i) * 65536) for i in range(4096)))


def put(heap, address, fmt, *values):
    struct.pack_into(fmt, heap, address - HEAP, *values)


def clamp_model(position, base, limits):
    result = []
    for current, source, limit in zip(position, base, limits):
        delta = signed(current - source) >> 6
        if (delta > 0 and limit <= 0) or (delta < 0 and limit >= 0):
            delta = 0
        if (delta > limit and delta > 0) or (delta < limit and delta < 0):
            delta = limit
        result.append(signed(source + (delta << 6)))
    return result


class Geometry(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x4B1EA0, 1), (0x4B2020, 1), (0x4083F0, 1), (0x4B4100, 2)):
            self.callbacks[self.addr(address)] = (4 * nargs,
                lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        args = tuple(self.args(nargs))
        self.trace.append((address, args, self.read(FLOATS, 0x1000)))
        self.u.reg_write(UC_X86_REG_EAX, 0)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def begin(self, seed):
        self.reset()
        self.heap = bytearray(random.Random(seed).randbytes(0x10000))
        self.u.mem_write(STACK, bytes([seed & 255]) * 0x10000)
        self.u.reg_write(UC_X86_REG_FPCW, 0x037F)
        return self.heap

    def invoke_checked(self, address, args):
        self.u.mem_write(HEAP, bytes(self.heap))
        registers = {r: 0x13579BDF + i * 0x11111111 for i, r in enumerate(SAVED)}
        for register, value in registers.items():
            self.u.reg_write(register, value)
        self.invoke(address, args)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF04 + 4 * len(args), 'stdcall'
        assert all(self.u.reg_read(r) == v for r, v in registers.items()), 'callee ABI'
        return self.read(HEAP, 0x10000)

    def clamp(self, slot, vertex, base, delta, limits, seed):
        h = self.begin(seed)
        put(h, RECORD + 4 * slot, '<I', MESHES)
        put(h, RECORD + 0x78 + 4 * slot, '<I', FIXED)
        put(h, MESHES + 0xC, '<I', FLOATS)
        put(h, FIXED + 0x20 * vertex, '<3i', *base)
        put(h, FIXED + 0x20 * vertex + 0x1B, '<3b', *limits)
        position = [signed(a + b) for a, b in zip(base, delta)]
        put(h, OUT, '<3i', *position)
        expected = bytearray(h)
        result = clamp_model(position, base, limits)
        put(expected, OUT, '<3i', *result)
        put(expected, FLOATS + 0x30 * vertex, '<3f', *(v / 65536 for v in result))
        actual = self.invoke_checked(0x508740, [OUT, slot, vertex, RECORD])
        assert actual == expected, ('clamp model / complete heap guards', slot, vertex, base, delta, limits)
        return actual

    def find(self, count, match, requested, seed):
        h = self.begin(seed)
        put(h, RECORD + 0x26A, '<B', count)
        for i in range(count):
            put(h, RECORD + 0x3C + i * 4, '<I', NODES + i * 0x18C)
            put(h, NODES + i * 0x18C + 0x30, '<I', 0xA1B2C300 | (requested & 255 if i == match else 254))
        actual = self.invoke_checked(0x508F60, [requested, RECORD])
        expected = match if 0 <= match < count and requested <= 255 else -1
        assert signed(self.u.reg_read(UC_X86_REG_EAX)) == expected, 'preview search'
        assert actual == h, 'search heap guards'
        return actual

    def opacity(self, index, ready, count, visible, seed):
        h = self.begin(seed)
        record = self.addr(0x82D220) + index * 0x2AC
        self.u.mem_write(record, bytes([0xA5]) * 0x2AC)
        self.put(record + 0x2A8, '<i', ready)
        self.put(record + 0x26A, '<B', count)
        expected = bytearray(h)
        for i in range(count):
            self.put(record + 0x3C + i * 4, '<I', NODES + i * 0x18C)
            value = visible[i % len(visible)]
            self.put(record + 0x26C + i * 4, '<i', value)
            if ready:
                put(expected, NODES + i * 0x18C + 0x17C, '<B', 255 if value else 0)
        before = self.read(record, 0x2AC)
        actual = self.invoke_checked(0x509150, [index])
        assert actual == expected, 'visibility model / byte-width / heap guards'
        assert self.read(record, 0x2AC) == before, 'visibility does not mutate record'
        return actual

    def deform(self, target, counts, radius, falloff, phase, seed):
        h = self.begin(seed)
        self.trace = []
        self.u.mem_write(self.addr(0x6E0EF4), SQRT)
        preview = target in (0x508890, 0x507FE0)
        radial = target in (0x467700, 0x507FE0)
        if preview:
            put(h, RECORD + 0x26A, '<B', len(counts))
            put(h, RECORD + 0x24C, '<' + 'H' * len(counts), *counts)
            values = ((0x82D120, (0, 0, 0)), (0x82D12C, (0, 65536, 0)),
                      (0x82D138, (65536, 0, 0)))
            scalars = ((0x82D144, radius), (0x82D150, radius), (0x82D154, falloff), (0x82D158, 0x10000))
            # All hull points lie in the starting dent plane, leaving its offset unchanged.
            put(h, BOX + 0x64, '<36i', *([0] * 36))
        else:
            self.put(self.addr(0x588B94), '<I', RECORD)
            self.put(self.addr(0x588A70), '<I', CAR)
            self.put(self.addr(0x5889B0), '<36i', *([0] * 36))
            put(h, CAR + 0xB1A, '<b', 0)
            put(h, CAR + 0x720, '<I', NODES)
            put(h, RECORD + 0x45C, '<i', len(counts))
            put(h, RECORD + 0x420, '<' + 'i' * len(counts), *counts)
            values = ((0x588A40, (0, 0, 0)), (0x588A4C, (0, 65536, 0)),
                      (0x588A58, (65536, 0, 0)))
            scalars = ((0x588A64, radius), (0x588A74, radius), (0x588A78, falloff), (0x588A7C, 0x10000))
        for address, value in values:
            self.put(self.addr(address), '<3i', *value)
        for address, value in scalars:
            self.put(self.addr(address), '<i', value)
        coordinates = [(0, 0, 0), (0.25, -0.5, 0.125), (-0.25, -1.25, 0.25),
                       (0.5, -3.0, -0.5), (0, 1.25, 0.125)]
        for mesh, count in enumerate(counts):
            node, object_, fixed, floats = (NODES + mesh * 0x18C, MESHES + mesh * 0x120,
                                           FIXED + mesh * 0x200, FLOATS + mesh * 0x300)
            put(h, RECORD + mesh * 4, '<I', object_)
            put(h, RECORD + 0x3C + mesh * 4, '<I', node)
            put(h, RECORD + 0x78 + mesh * 4, '<I', fixed)
            put(h, node + 0xC, '<I', object_)
            put(h, object_ + 0xC, '<I', floats)
            for i in range(count):
                coords = coordinates[(i + phase) % len(coordinates)]
                put(h, floats + i * 0x30, '<6f', *coords, 0.25, 0.5, -0.25)
                put(h, fixed + i * 0x20, '<3i', *(round(v * 65536) for v in coords))
                put(h, fixed + i * 0x20 + 0x18, '<3b', 17, -111, 53)
                put(h, fixed + i * 0x20 + 0x1B, '<3b', 127, -127, -127)
        args = [RECORD, BOX] if target == 0x507FE0 else [RECORD] if preview else []
        actual = self.invoke_checked(target, args)
        # Only existing vertex float prefixes can change; every other byte is
        # guarded. Positions must also satisfy the independent source clamp.
        allowed = set()
        for mesh, count in enumerate(counts):
            for i in range(count):
                start = FLOATS - HEAP + mesh * 0x300 + i * 0x30
                allowed.update(range(start, start + 24))
                floats = struct.unpack_from('<3f', actual, start)
                after = [round(v * 65536) for v in floats]
                before = struct.unpack_from('<3i', h, FIXED - HEAP + mesh * 0x200 + i * 0x20)
                assert after == clamp_model(after, before, (127, -127, -127)), 'consumer clamp invariant'
        assert all(a == b or i in allowed for i, (a, b) in enumerate(zip(actual, h))), 'deform heap guards'
        globals_ = [self.read(self.addr(a), 12) for a, _ in values]
        return actual, self.trace, globals_


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    images = (Geometry(ROOT / 'cmr2bin/CMR2.exe'), Geometry(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities))
    cases = []
    for slot, vertex, limits, delta in itertools.product((0, 7, 14), (0, 4),
            ((0, 0, 0), (127, -127, 1), (-128, 127, -127), (-1, -1, -1)),
            ((0, 0, 0), (1, -1, 63), (64, -64, 65), (8128, -8128, -8192),
             (8192, -8192, 0x80000000), (-0x80000000, 0x7FFFFFFF, 0x10000))):
        cases.append(('clamp', (slot, vertex, (-0x7FFFFFFF, 12345, 0x7FFFFFFF), delta, limits)))
    rng = random.Random(70211)
    for _ in range(350):
        cases.append(('clamp', (rng.randrange(15), rng.randrange(5),
            tuple(rng.randrange(-0x80000000, 0x80000000) for _ in range(3)),
            tuple(rng.randrange(-0x80000000, 0x80000000) for _ in range(3)),
            tuple(rng.randrange(-128, 128) for _ in range(3)))))
    for count, match, requested in itertools.product((0, 1, 7, 15), (-1, 0, 6, 14), (7, 263)):
        cases.append(('find', (count, match, requested)))
    for index, ready, count, visible in itertools.product((0, 7, 15), (0, 1, -1), (0, 1, 7, 15), ((0,), (1,), (-1, 0, 1, 2))):
        cases.append(('opacity', (index, ready, count, visible)))
    for args in itertools.product((0x467700, 0x467E90, 0x508890, 0x507FE0),
                                  ((), (0,), (1,), (0, 5, 1)), (0x8000, 0x10000), (0x4000, 0x8000), range(5)):
        cases.append(('deform', args))
    for i, (method, args) in enumerate(cases):
        try:
            a, b = [getattr(image, method)(*args, 737 + i) for image in images]
            assert a == b, 'original / rebuilt'
        except Exception as exc:
            raise AssertionError((method, args, i)) from exc
    print(f'{len(cases)} deformation record cases: 0 differences; independent signed clamp/search/visibility models, '
          'four complete deformation consumers, their real helpers, upload order, complete heap guards and ABI')


if __name__ == '__main__':
    main()
