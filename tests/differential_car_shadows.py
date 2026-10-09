#!/usr/bin/env python3
"""Car shadow triangles, their submission order and contact midpoint writes.

Usage: differential_car_shadows.py entities.json [rebuilt.exe]
Runs the full shadow entry and real colour, render-state and skid-visibility
helpers. Replay lookup, camera offset and final triangle queue are controlled.
An independent fixed-point model checks every vertex, RGBA and UV field,
untouched contact fields and guarded heap. Includes poison in unused stack slots.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting
from differential_auto_steering import signed
from differential_body_patch import add, sub, scale, mul

SIZE = 0x2A4
SHIFT = (0x8000, -0x4000, 0x10000)


def vector(data, offset):
    return list(struct.unpack_from('<3i', data, offset))


def midpoint(a, b):
    return add(scale(sub(a, b), 0x8000), b)


def vertex(point, colour):
    return struct.pack('<3i', *point) + colour + bytes(8)


class Shadows(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {self.addr(0x469680): (4, self.replay),
                          self.addr(0x495F50): (8, self.offset),
                          self.addr(0x4BBA00): (24, self.triangle)}

    def clobber(self, result=0x12345678):
        for register, value in ((UC_X86_REG_EAX, result), (UC_X86_REG_ECX, 0xA5A5A5A5),
                                (UC_X86_REG_EDX, 0x5A5A5A5A)):
            self.u.reg_write(register, value)

    def replay(self):
        assert self.args(1) == (self.index,)
        self.trace.append(('replay', self.index))
        self.clobber(HEAP + 0x6000)

    def offset(self):
        view, record = self.args(2)
        assert (view, record) == (self.view, self.addr(0x592490))
        self.trace.append(('offset', view))
        count, = struct.unpack('<h', self.read(record + 0x292, 2))
        starts = [0xC0 + 12 * i for i in range(count)]
        if struct.unpack('<i', self.read(record + 0x298, 4))[0]:
            starts += list(range(0, 0xC0, 12))
        for start in starts:
            self.put(record + start, '<3i', *add(vector(self.read(record, SIZE), start), SHIFT))
        self.clobber()

    def triangle(self):
        context, a, b, c, texture, layer = self.args(6)
        assert (context, texture, layer) == (0, 0, 10)
        self.trace.append(('triangle', self.read(a, 24), self.read(b, 24), self.read(c, 24)))
        self.clobber()

    def hook(self, u, address, size, data):
        if address == self.addr(0x4B4A80):
            self.trace.append(('colour',))
        elif address == self.addr(0x411060):
            self.trace.append(('render-state',))
        elif address == self.addr(0x494D40):
            car, record, point = self.args(3)
            assert car == HEAP + 0x1000 and record == self.addr(0x592490)
            self.trace.append(('visibility', point))
        super().hook(u, address, size, data)

    def run_shadows(self, seed, index, count, edge, mode, replay, wheels, poison):
        case = seed, index, count, edge, mode, replay, wheels, poison
        rnd = random.Random(seed)
        self.reset()
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, bytes([poison]) * 0x10000)
        car, contacts, nodes, graphics = (HEAP + n for n in (0x1000, 0x3000, 0x5000, 0x7000))
        record = contacts + index * SIZE
        self.index, self.view = index, seed % 2
        ghost, disabled = ((0, 0), (0, 1), (1, 0))[mode]
        self.put(car + 0xB1A, '<b', index)
        strength = (0, 0x8000, 0x10000, 0x18000)[seed % 4]
        self.put(car + 0xA70, '<i', strength)
        height = (-1, 1, 0x7FFF, 0x8001)[seed % 4]
        for i in range(8):
            y = struct.unpack('<i', self.read(car + 0x270 + i * 12 + 4, 4))[0]
            self.put(car + 0x8DC + i * 4, '<i', signed(y - height - i * 31))
        for i in range(4):
            self.put(car + 0x738 + i * 4, '<I', nodes + i * 0x20)
            self.put(nodes + i * 0x20 + 8, '<i', 9 if wheels == 2 and i == 1 else 10)
        self.put(self.addr(0x536BE0), '<i', 9)
        self.put(self.addr(0x520B74), '<I', graphics)
        self.put(graphics + 0x3BC, '<I', 0x20 if disabled else 0)
        self.put(self.addr(0x592734), '<I', contacts)
        self.put(HEAP + 0x6000 + 0x4BC, '<i', replay)
        handling = HEAP + 0x8000
        self.put(handling, '<3B', count, 0, count // 2)
        self.put(self.addr(0x592278) + index * 4, '<I', handling)
        self.put(self.addr(0x5922BC) + index * 4, '<I', handling + 1)
        light_colour = rnd.randbytes(4)
        light_level = (0, 0x7F0000, 0xFF0000, 0x7FFFFFFF)[seed % 4]
        self.u.mem_write(self.addr(0x6E0098), light_colour)
        self.put(self.addr(0x6E0B38), '<i', light_level)
        fade = (0, 0x8000, 0x10000, -0x10000)[seed % 4]
        grips = (1, 0 if wheels == 2 else 0x8000, 0x10000, -0x10000)
        self.put(record + 0x250, '<5i', fade, *grips)
        self.put(record + 0x290, '<2h5i', edge, count + 5, ghost, int(wheels != 0),
                 seed % 2, (seed // 2) % 2, 0)
        before_heap = self.read(HEAP, 0x10000)
        expected_heap = bytearray(before_heap)
        contact = bytearray(self.read(record, SIZE))
        drawn = bytearray(contact)
        for start in [0xC0 + 12 * i for i in range(count + 5)] + (
                list(range(0, 0xC0, 12)) if wheels else []):
            struct.pack_into('<3i', drawn, start, *add(vector(drawn, start), SHIFT))
        # Guards may contain other real globals; take their value after fixture setup.
        guard_start = self.addr(0x592490) - 16
        before_guard = self.read(guard_start, SIZE + 32)
        expected_guard = bytearray(before_guard)
        rgb = light_colour[:3]
        clear = rgb + b'\0'
        trace = [('replay', index), ('colour',), ('offset', self.view)]

        def emit(p0, c0, p1, c1, p2, c2):
            trace.append(('triangle', vertex(p0, c0), vertex(p1, c1), vertex(p2, c2)))

        if wheels:
            for i in range(4):
                trace.append(('render-state',))
                if (wheels == 2 and i == 1) or grips[i] == 0:
                    continue
                corners = [vector(contact, i * 48 + j * 12) for j in range(4)]
                for offset, pair in ((0x198, (0, 2)), (0x1C8, (1, 3))):
                    struct.pack_into('<3i', expected_heap, record - HEAP + offset + i * 12,
                                     *midpoint(corners[pair[0]], corners[pair[1]]))
                corners = [vector(drawn, i * 48 + j * 12) for j in range(4)]
                front, rear = midpoint(corners[0], corners[2]), midpoint(corners[1], corners[3])
                colour = rgb + bytes([(mul(0xE60000, grips[i]) >> 16) & 255])
                emit(corners[0], clear, corners[1], clear, front, colour)
                emit(front, colour, corners[1], clear, rear, colour)
                emit(front, colour, rear, colour, corners[2], clear)
                emit(rear, colour, corners[3], clear, corners[2], clear)
        points = [vector(drawn, 0xC0 + j * 12) for j in range(18)]
        centre = midpoint(points[0], points[2])
        if not ghost and not disabled:
            outline = [points[edge], points[(edge + 1) % 4]]
            for i in range(count + 1):
                visible = i - seed % 2
                if (seed // 2) % 2 == 0:
                    visible = count - visible - 1
                if replay:
                    trace.append(('visibility', i))
                if not replay or not 0 <= visible < count // 2:
                    outline.append(points[4 + i])
            outline.extend((points[(edge + 3) % 4], points[(edge + 2) % 4]))
            total = count - (count // 2 if replay else 0) + 4
            assert len(outline) == total + 1
            factor = signed(0xFD70 - mul(signed(0x10000 - fade), 0x23D7))
            ring = [add(centre, scale(sub(p, centre), factor)) for p in outline]
            colour = rgb + bytes([(mul(light_level, strength) >> 16) & 255])
            for i in range(total):
                j = (i + 1) % total
                emit(ring[i], colour, centre, colour, ring[j], colour)
                emit(outline[i], clear, ring[i], colour, outline[j], clear)
                emit(outline[j], clear, ring[i], colour, ring[j], colour)
        ring = [None] * 8
        for i in range(4):
            edge_vector = sub(points[(i + 1) % 4], points[i])
            ring[i * 2 + 1] = add(points[i], scale(edge_vector, 0xCCC))
            ring[(i * 2 + 2) % 8] = add(points[i], scale(edge_vector, 0xF333))
        core = [add(centre, scale(sub(p, centre), 0x9999)) for p in ring]
        if ghost:
            alpha = 255
        else:
            t = 0x10000 if height <= 0 else 0x10000 - min(mul(height, 0x20000), 0x10000)
            alpha = ((t * 0xFF0000) >> 32) & 255
        colour = rgb + bytes([alpha])
        for i in range(8):
            j = (i + 1) % 8
            emit(core[j], colour, core[i], colour, centre, colour)
            emit(ring[i], clear, core[i], colour, ring[j], clear)
            emit(core[i], colour, core[j], colour, ring[j], clear)
        expected_guard[16:16 + SIZE] = drawn
        self.invoke(0x494DB0, [car, self.view])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 12
        assert self.read(HEAP, 0x10000) == expected_heap, (case, 'heap/contact midpoint writes')
        assert self.read(guard_start, SIZE + 32) == expected_guard, (case, 'contact view/guards')
        assert self.trace == trace, (case, 'triangle order/vertices/providers', next(
            ((i, a, b) for i, (a, b) in enumerate(itertools.zip_longest(self.trace, trace)) if a != b), None))
        return self.read(HEAP, 0x10000), self.read(self.addr(0x592490), SIZE), self.trace


def cases():
    yield from itertools.product(range(4), (0, 7), (0, 3, 13), range(4), range(3),
                                 (0, 1), range(3), (0, 0xA5))


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Shadows(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Shadows(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    for count, case in enumerate(cases(), 1):
        assert original.run_shadows(*case) == rebuilt.run_shadows(*case), case
    print(f'{count} car-shadow cases: identical ordered triangles and fixed-point vertices/RGBA/UV; '
          'independent geometry, guarded contact writes and poisoned stack')
    return 0


if __name__ == '__main__':
    sys.exit(main())
