#!/usr/bin/env python3
"""Guarded models of car-part slots, light flicker and float triangle sampling.

Execute complete original/rebuilt bodies, including the ordered setup body's
compaction and snapshot copies. Rendering, model providers, RNG and saved-driver
records are controlled leaves. Check independent models, the entire heap,
provider order and stdcall/callee-saved ABI. Random conversion uses nearest-even
FISTP; the 1/RAND_MAX constant is read from the original image.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                              UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP, UC_X86_REG_ESP)
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting, add, mul, signed

CAR, PARTS, NODE = HEAP + 0x1000, HEAP + 0x2000, HEAP + 0x3000
MESH, TRIANGLES, VERTICES = HEAP + 0x5000, HEAP + 0x6000, HEAP + 0x7000
DAMAGE, SAVED, ORDER, OUT = HEAP + 0x8000, HEAP + 0x9000, HEAP + 0xA000, HEAP + 0xB000
REGISTERS = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def write(heap, address, fmt, *values):
    struct.pack_into(fmt, heap, address - HEAP, *values)


class Parts(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        # rand is cdecl (zero arguments); these remaining leaves are stdcall.
        for address, nargs in ((0x4C6676, 0), (0x4AB710, 1), (0x42B5F0, 1),
                               (0x480A60, 0), (0x480A50, 0), (0x480B40, 1),
                               (0x480AF0, 3), (0x4074F0, 0), (0x41B370, 0),
                               (0x407610, 1), (0x46ACB0, 3), (0x469690, 1)):
            self.callbacks[self.addr(address)] = (4 * nargs,
                lambda a=address, n=nargs: self.provider(a, n))
        self.reciprocal = struct.unpack_from('<f', self.memory, self.addr(0x5112F0) - self.base)[0]

    def provider(self, address, nargs):
        args = tuple(self.args(nargs))
        value = 0
        if address == 0x4C6676:
            value = self.draws.pop(0)
            self.trace.append(('rand', value))
        elif address == 0x4AB710:
            value = self.vertices_count
            self.trace.append(('vertex_count', *args))
        elif address == 0x42B5F0:
            value = CAR
            self.trace.append(('car', *args))
        elif address == 0x4074F0:
            value = self.players
        elif address == 0x41B370:
            value = 3
        elif address == 0x407610:
            value = SAVED
            self.trace.append(('saved', *args))
        else:
            self.trace.append((address, *args))
        self.u.reg_write(UC_X86_REG_EAX, value & 0xFFFFFFFF)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def begin(self, seed):
        self.reset()
        self.trace, self.draws = [], []
        self.vertices_count, self.players = 0, 0
        self.heap = bytearray(random.Random(seed).randbytes(0x10000))
        self.u.mem_write(STACK, bytes([seed & 255]) * 0x10000)
        self.put(self.addr(0x588990), '<8I', *([PARTS] * 8))
        self.put(self.addr(0x588B94), '<I', PARTS)
        self.put(self.addr(0x588B98), '<I', DAMAGE)
        # Explicit x87 nearest-even mode, as used by the game.
        from unicorn.x86_const import UC_X86_REG_FPCW
        self.u.reg_write(UC_X86_REG_FPCW, 0x037F)
        return self.heap

    def check(self, function, args, expected, trace, result=None):
        self.u.mem_write(HEAP, bytes(self.heap))
        initial = {reg: 0x12345678 + i * 0x11111111 for i, reg in enumerate(REGISTERS)}
        for reg, value in initial.items():
            self.u.reg_write(reg, value)
        self.invoke(function, [v & 0xFFFFFFFF for v in args])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF04 + len(args) * 4, 'stdcall'
        assert all(self.u.reg_read(reg) == value for reg, value in initial.items()), 'saved registers'
        actual = self.read(HEAP, 0x10000)
        assert actual == expected, (hex(function), 'heap model / guards',
            next(((hex(i), a, b) for i, (a, b) in enumerate(zip(actual, expected)) if a != b), None))
        assert self.trace == trace, (hex(function), 'provider order', self.trace, trace)
        if result is not None:
            assert signed(self.u.reg_read(UC_X86_REG_EAX)) == result, (hex(function), 'return')
        return actual, self.trace

    def register(self, index, node_type, object_present, vertex_count, seed):
        h = self.begin(seed)
        write(h, NODE + 0x30, '<I', 0xFEDCBA00 | (index + 5))
        write(h, NODE + 0x178, '<i', node_type)
        write(h, NODE + 0xC, '<I', MESH if object_present else 0)
        write(h, PARTS + 0x45C, '<i', 2)
        self.vertices_count = vertex_count
        expected, trace = bytearray(h), []
        if object_present and node_type == 0:
            write(expected, PARTS + 0x3C + 4 * index, '<I', NODE)
            write(expected, PARTS + 4 * index, '<I', MESH)
            write(expected, PARTS + 0x420 + 4 * index, '<i', vertex_count)
            trace.append(('vertex_count', MESH))
            if vertex_count > 0:
                write(expected, PARTS + 0x45C, '<i', 3)
        return self.check(0x466E90, [NODE, PARTS], expected, trace)

    def find(self, count, match, requested, seed):
        h = self.begin(seed)
        write(h, PARTS + 0x45C, '<i', count)
        for i in range(max(count, 0)):
            write(h, PARTS + 0x3C + 4 * i, '<I', NODE + 0x18C * i)
            write(h, NODE + 0x18C * i + 0x30, '<I', 0xA1B2C300 | (requested if i == match else 254))
        result = match if 0 <= match < count and requested <= 255 else -1
        return self.check(0x4692B0, [requested, PARTS], h, [], result)

    def flicker(self, damage, timer, phase, draw, car, seed):
        h = self.begin(seed)
        write(h, CAR + 0xB1A, '<b', car)
        write(h, PARTS + 0x29C, '<i', damage)
        write(h, PARTS + 0x40C, '<i', timer)
        write(h, PARTS + 0x4CC, '<i', phase)
        expected, trace = bytearray(h), []
        if damage > 0x8000:
            k = mul(max(0, min(0x10000, mul(signed(damage - 0x8000), 0x20000))), 0xCCCC)
            if timer > 0:
                write(expected, PARTS + 0x40C, '<i', add(timer, -0x10000))
            else:
                self.draws = [draw]
                trace = [('rand', draw)]
                r = round(draw * self.reciprocal * 65536)
                next_phase = 0 if phase else 1
                duration = (add(mul(0x10000 - k, mul(r, 0xFA0000)), mul(0x10000 - k, 0x320000))
                            if phase else mul(k, mul(r, 0xF0000)))
                write(expected, PARTS + 0x40C, '<i', duration)
                write(expected, PARTS + 0x4CC, '<i', next_phase)
        return self.check(0x46B4E0, [CAR], expected, trace)

    def edge(self, part, present, triangle, edge, draw, seed):
        h = self.begin(seed)
        write(h, PARTS + 4 * part, '<I', MESH if present else 0)
        write(h, MESH + 0xC, '<I', VERTICES)
        write(h, MESH + 0x24, '<Ii', TRIANGLES, 3)
        coords = [(-1.25, 0.5, 2.25), (3.75, -4.5, 0.25), (0.125, 7.75, -5.0),
                  (-2.0, -0.25, 1.0), (0.5, 0.75, -0.125)]
        indices = [(0, 1, 2), (4, 0, 3), (2, 4, 1)]
        for i, v in enumerate(coords):
            write(h, VERTICES + i * 0x30, '<3f', *v)
        for i, ix in enumerate(indices):
            write(h, TRIANGLES + i * 0x4C + 0x40, '<3H', *ix)
        expected, trace = bytearray(h), []
        value = [0, 0, 0]
        if present:
            self.draws = [triangle, edge, draw]
            trace = [('rand', d) for d in self.draws]
            ia, ib = indices[triangle % 3][edge % 3], indices[triangle % 3][(edge + 1) % 3]
            a, b = [[round(x * 65536) for x in coords[i]] for i in (ia, ib)]
            t = round(draw * self.reciprocal * 65536)
            value = [add(x, mul(y - x, t)) for x, y in zip(a, b)]
        write(expected, OUT, '<3i', *value)
        return self.check(0x469C30, [OUT, PARTS, part], expected, trace, int(present))

    def setup(self, lock, keep, populated, saved, empty_impacts, seed):
        h = self.begin(seed)
        write(h, ORDER, '<h', 0)
        write(h, CAR + 0xB1A, '<b', 0)
        # One mesh at sparse slot 2. Its full registrar body runs normally.
        h[PARTS - HEAP:PARTS - HEAP + 0xB4] = bytes(0xB4)
        write(h, PARTS + 0x420, '<15i', *([0] * 15))
        write(h, CAR + 0x720, '<I', NODE if populated else 0)
        write(h, NODE, '<3I', 0, 0, 0)
        write(h, NODE + 0xC, '<I', MESH)
        write(h, NODE + 0x30, '<I', 7)
        write(h, NODE + 0x178, '<i', 0)
        write(h, SAVED + 0x104, '<B', 0 if empty_impacts else 2)
        self.players, self.vertices_count = int(saved), 3
        expected, trace = bytearray(h), [(0x480A60,)]
        if not lock:
            trace.append((0x480A50,))
        trace.extend([('car', 0), (0x480B40, CAR)])
        write(expected, PARTS + 0x45C, '<i', int(populated))
        if populated:
            # Apply registration followed by array compaction; the final slot
            # duplicates the prior tail just as the original does.
            write(expected, PARTS + 0x410, '<4i', 0, 0, 0, 0)
            write(expected, PARTS + 8, '<I', MESH)
            write(expected, PARTS + 0x3C + 8, '<I', NODE)
            write(expected, PARTS + 0x420 + 8, '<i', 3)
            trace.append(('vertex_count', MESH))
            j = 0
            while j < 15:
                owner = struct.unpack_from('<I', expected, PARTS - HEAP + 0x3C + 4 * j)[0]
                if owner:
                    j += 1
                    continue
                moved = False
                for k in range(j, 14):
                    owners = struct.unpack_from('<2I', expected, PARTS - HEAP + 0x3C + 4 * k)
                    moved |= any(owners)
                    for offset, stride in ((0, 4), (0x3C, 4), (0x78, 4), (0x420, 4), (0xB4, 12), (0x168, 12)):
                        dest = PARTS - HEAP + offset + k * stride
                        expected[dest:dest + stride] = expected[dest + stride:dest + 2 * stride]
                j += int(not moved)
            trace.append((0x480AF0, CAR, NODE, 0))
            write(expected, PARTS + 0x460, '<6B', 4, 5, 1, 2, 8, 9)
            write(expected, PARTS + 0x21C, '<9i', *([0] * 9))
            write(expected, PARTS + 0x469, '<B', 0)
            write(expected, PARTS + 0x46C, '<i', 0)
            write(expected, PARTS + 0x240, '<34i', *([0] * 34))
            write(expected, PARTS + 0x350, '<34i', *([0] * 34))
            write(expected, PARTS + 0x470, '<16i', *([0] * 16))
            scales = [0x4CCC, 0x4CCC, 0x9999, 0x9999, 0x4CCC, 0x4CCC, 0x4CCC, 0x4CCC,
                      0x9999, 0x9999, 0x4CCC, 0x4CCC, 0x9999, 0x9999, 0x9999, 0x4CCC,
                      0x9999, 0x9999] + [0x10000] * 5 + [0x6666, 0x9999, 0x4CCC] + [0x10000] * 8
            write(expected, PARTS + 0x2C8, '<34i', *scales)
            if not keep:
                write(expected, DAMAGE + 0x28C, '<i', 0)
            if saved:
                trace.append(('saved', 3))
                source = h[SAVED - HEAP:SAVED - HEAP + 0x148]
                for offset, src, size in [(0, 0, 0x106), (0x20C, 0x108, 0x40)] + (
                        [(0x106, 0, 0x106), (0x24C, 0x108, 0x40)] if not keep else []):
                    start = DAMAGE - HEAP + offset
                    expected[start:start + size] = source[src:src + size]
                if empty_impacts:
                    for i in range(20):
                        write(expected, DAMAGE + i * 13 + 12, '<B', 255)
                        if not keep:
                            write(expected, DAMAGE + 0x106 + i * 13 + 12, '<B', 255)
            trace.extend([(0x46ACB0, 0, NODE, PARTS), (0x469690, CAR)])
        if lock:
            trace.append((0x480A50,))
        return self.check(0x4669F0, [lock, keep, ORDER, 1], expected, trace)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    rebuilt = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe'
    images = (Parts(ROOT / 'cmr2bin/CMR2.exe'), Parts(rebuilt, entities))
    cases = []
    for index, kind, present, count in itertools.product((0, 2, 14), (0, 1, 3), (0, 1), (-1, 0, 7)):
        cases.append(('register', (index, kind, present, count)))
    for count in (-1, 0, 1, 7, 15):
        for match in (-1, 0, max(0, count - 1)):
            for requested in (7, 254, 263):
                # nonmatching slots use 254, so request 254 only for count=0
                if requested == 254 and count > 0:
                    continue
                cases.append(('find', (count, match, requested)))
    for damage, timer, phase, draw in itertools.product(
            (-0x80000000, 0x7FFF, 0x8000, 0x8001, 0xBFFF, 0x10000, 0x18000, 0x7FFFFFFF),
            (-1, 0, 1, 0x10000), (0, 1, -1), (0, 12345, 32767)):
        cases.append(('flicker', (damage, timer, phase, draw, len(cases) % 8)))
    for part, tri, edge, draw in itertools.product((0, 7, 14), (0, 1, 2), (0, 1, 2), (0, 16383, 32767)):
        cases.append(('edge', (part, 1, tri, edge, draw)))
    cases.extend(('edge', (part, 0, 0, 0, 0)) for part in (0, 7, 14))
    for args in itertools.product((0, 1), repeat=5):
        cases.append(('setup', args))
    for i, (method, args) in enumerate(cases):
        try:
            results = [getattr(image, method)(*args, 8137 + i) for image in images]
        except Exception as exc:
            raise AssertionError((method, args, 8137 + i)) from exc
        assert results[0] == results[1], (method, args, 'original / rebuilt')
    print(f'{len(cases)} car part cases: 0 differences; slot registration/search, sparse compaction, '
          'saved snapshots, signed flicker, all triangle edges, RNG order, heap guards and ABI checked')


if __name__ == '__main__':
    main()
