#!/usr/bin/env python3
"""Body contact rectangle, ghost rescaling and box fallback with real helpers.

Usage: differential_body_patch.py entities.json [rebuilt.exe]
Independent fixed-point geometry and quantized-length model; guarded contact,
car/hull/body records, globals and immutable scale/sqrt tables. Fixtures have
valid slots/type indices and distinct buffers; no full contact/race tick.
"""
import itertools
import json
import math
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting
from differential_auto_steering import signed

SQRT = [round(math.sqrt((8 + 16 * i) / 65536) * 65536) & 65535 for i in range(4096)]


def mul(a, b):
    return signed((a * b) >> 16)


def add(a, b):
    return [signed(x + y) for x, y in zip(a, b)]


def sub(a, b):
    return [signed(x - y) for x, y in zip(a, b)]


def scale(v, n):
    return [mul(x, n) for x in v]


def dot(a, b):
    return signed(sum(mul(x, y) for x, y in zip(a, b)))


def cross(a, b):
    return [signed(mul(a[(j + 1) % 3], b[(j + 2) % 3]) -
                   mul(a[(j + 2) % 3], b[(j + 1) % 3])) for j in range(3)]


def normalize(v):
    # Length uses a midpoint square-root lookup after selecting an even
    # power-of-two exponent. Products are rounded before summing in 16.16.
    squared = sum(mul(x, x) for x in v) & 0xFFFFFFFF
    if not squared:
        return [0, 0, 0]
    exponent = (squared.bit_length() - 1 - 15 + 1) // 2 * 2
    shift = exponent + 4
    index = squared >> shift if shift >= 0 else squared << -shift
    assert 0 <= index < 4096
    root = SQRT[index]
    length = root << (exponent // 2) if exponent >= 0 else root >> (-exponent // 2)
    assert length > 2, 'fixture must keep reciprocal within signed 32-bit range'
    return scale(v, (1 << 32) // length)


class BodyPatch(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        self.put(self.addr(0x6E0EF4), '<4096H', *SQRT)
        self.initial_image = self.read(self.base, self.size)
        self.scale_a = struct.unpack('<14i', self.read(self.addr(0x520150), 56))
        self.scale_b = struct.unpack('<14i', self.read(self.addr(0x520188), 56))
        self.edges = [tuple(self.read(self.addr(0x5201C0) + j * 2, 2)) for j in range(4)]

    def hook(self, u, address, size, data):
        if address == self.addr(0x456BE0):
            self.trace.append((0x456BE0, self.args(1)))
        super().hook(u, address, size, data)

    def run_patch(self, seed, index, ghost, override, unknown, stage, alignment):
        case = seed, index, ghost, override, unknown, stage, alignment
        rnd = random.Random(seed)
        self.u.mem_write(self.base, self.initial_image)
        self.u.mem_write(STACK, bytes(0x10000))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        car, contact, body, hull = (HEAP + i for i in (0x1000, 0x3000, 0x5000, 0x6000))
        locations = ((0x592298, 4), (0x5922E0, 12), (0x5922EC, 4), (0x5922F0, 12),
                     (0x592320, 12), (0x59232C, 12), (0x592468, 12), (0x592474, 12),
                     (0x592480, 4), (0x592484, 4))
        for a, size in locations:
            self.u.mem_write(self.addr(a) - 16, rnd.randbytes(size + 32))
        self.put(self.addr(0x592298), '<I', hull)
        self.put(self.addr(0x5922EC), '<I', body)
        origin = [rnd.randrange(-0xA0000, 0xA0001) for _ in range(3)]
        ground = [rnd.randrange(-0xA0000, 0xA0001) for _ in range(3)]
        normal, right, up = [0, 65536, 0], [65536, 0, 0], [0, 65536, 0]
        if alignment == 1:
            normal, up = [65536, 0, 0], [0, 65536, 0]
        elif alignment in (2, 3, 4, 5):
            t = (0xFD6F, 0xFD70, 0xFD71, -0xFD71)[alignment - 2]
            right = [65536, t, seed * 0x1000]
        elif alignment == 6:
            right = [1, 0, 1]
        elif alignment == 7:
            right = [-65536, 0, 32768]
        elif alignment == 8:
            normal = [0, 0, 0]
        elif alignment == 9:
            normal, right, up = [0, -65536, 0], [65536, 0, 0], [0, -65536, 0]
        self.put(self.addr(0x5922F0), '<3i', *origin)
        self.put(self.addr(0x5922E0), '<3i', *ground)
        self.put(self.addr(0x592320), '<3i', *right)
        self.put(self.addr(0x59232C), '<3i', *up)
        self.put(body + 0xE0, '<3i', *normal)
        heights = [rnd.randrange(-0x60000, 0x60001) for _ in range(4)]
        self.put(body + 0xEC, '<4i', *heights)
        points = [add(origin, [rnd.randrange(-0x40000, 0x40001) for _ in range(3)]) for _ in range(8)]
        for j, v in enumerate(points):
            self.put(hull + j * 12, '<3i', *v)
        car_axes = [[rnd.randrange(-65536, 65537) for _ in range(3)] for _ in range(3)]
        for j, v in enumerate(car_axes):
            self.put(car + 0x360 + j * 12, '<3i', *v)
        extents = [(0, 1, 65536, 0x40000)[(seed + j) % 4] for j in range(3)]
        self.put(car + 0x204, '<3i', *extents)
        self.put(car + 0x2D0, '<3i', *origin)
        corners = [[rnd.choice((-2**31, 2**31 - 1, -65536, 0, 65536)) for _ in range(3)] for _ in range(4)]
        for j, v in enumerate(corners):
            self.put(car + 0x270 + j * 12, '<3i', *v)
        self.put(car + 0xB1A, '<b', index)
        self.put(car + 0xB1B, '<B', stage if seed % 2 else (stage + 1) % 14)
        self.put(car + 0xB64, '<i', unknown)
        self.put(car + 0xC00, '<i', override)
        self.put(contact + 0x294, '<i', ghost)
        self.put(self.addr(0x542630) + index * 0x24, '<B', stage)
        heap_before = self.read(HEAP, 0x10000)
        expected = bytearray(heap_before)
        spans = {a: self.read(self.addr(a) - 16, size + 32) for a, size in locations}
        immutable = [(a, self.read(self.addr(a), size)) for a, size in
                     ((0x520150, 56), (0x520188, 56), (0x5201C0, 8), (0x6E0EF4, 8192), (0x542630, 8 * 0x24))]
        writes = {0x592480: struct.pack('<i', 0), 0x592484: struct.pack('<i', 0)}
        if ghost and not override:
            result = [points[j].copy() for j in (1, 0, 2, 3)]
            if not unknown:
                for j, source in enumerate((1, 0, 2, 3)):
                    result[j][1] = heights[source]
            if not seed % 2:
                for j, (a, b) in enumerate(self.edges):
                    half = scale(sub(result[a], result[b]), 32768)
                    centre = add(result[b], half)
                    radius = scale(half, self.scale_a[stage] if j < 2 else self.scale_b[stage])
                    result[a], result[b] = add(centre, radius), sub(centre, radius)
        elif unknown:
            direction, lateral = car_axes[0], car_axes[2]
            length, width = extents[0], extents[2]
            offset = add(sub(ground, origin), scale(car_axes[1], extents[1]))
            result = [add(corners[j], offset) for j in (0, 1, 3, 2)]
            writes.update({0x592468: struct.pack('<3i', *direction), 0x592474: struct.pack('<3i', *lateral),
                           0x592480: struct.pack('<i', length), 0x592484: struct.pack('<i', width)})
        else:
            primary = up if abs(dot(normal, right)) > 0xFD70 else right
            direction = normalize(sub(primary, scale(normal, dot(normal, primary))))
            lateral = cross(direction, normal)
            length = max(0, *(dot(sub(v, origin), direction) for v in points))
            width = max(0, *(dot(sub(v, origin), lateral) for v in points))
            f, s = scale(direction, length), scale(lateral, width)
            result = [add(ground, v) for v in (sub(f, s), add(f, s), add(scale(f, -65536), s), sub(scale(f, -65536), s))]
            writes.update({0x592468: struct.pack('<3i', *direction), 0x592474: struct.pack('<3i', *lateral),
                           0x592480: struct.pack('<i', length), 0x592484: struct.pack('<i', width)})
        for j, v in enumerate(result):
            struct.pack_into('<3i', expected, 0x3000 + 0xC0 + j * 12, *v)
        self.trace = []
        self.invoke(0x4962C0, [car, contact])
        actual = self.read(HEAP, 0x10000)
        assert actual == expected, ('contact/heap mismatch', case,
                                   [(hex(i), a, b) for i, (a, b) in enumerate(zip(actual, expected)) if a != b][:12])
        assert self.trace == [(0x456BE0, (index,))]
        for address, before in spans.items():
            start = self.addr(address) - 16
            expected_span = bytearray(before)
            for a, data in writes.items():
                pointer = self.addr(a)
                lo, hi = max(start, pointer), min(start + len(before), pointer + len(data))
                if lo < hi:
                    expected_span[lo - start:hi - start] = data[lo - pointer:hi - pointer]
            assert self.read(start, len(before)) == expected_span, ('global/guard mismatch', case, hex(address))
        assert all(self.read(self.addr(a), len(data)) == data for a, data in immutable)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 12
        return actual, tuple(self.read(self.addr(a), len(v)) for a, v in sorted(writes.items())), self.trace


def cases():
    yield from itertools.product(range(4), (0, 1, 7), (0, 1), (0, 1), (0, 1), range(14), range(10))


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = BodyPatch(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = BodyPatch(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    for count, case in enumerate(cases(), 1):
        assert original.run_patch(*case) == rebuilt.run_patch(*case), case
    print(f'{count} body-patch cases: independent ghost/plane/box geometry, all 14 types, normalization threshold/zero, guarded contact/globals and real helpers; no mocks')
    return 0


if __name__ == '__main__':
    sys.exit(main())
