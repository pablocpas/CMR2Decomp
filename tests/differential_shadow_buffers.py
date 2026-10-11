#!/usr/bin/env python3
"""Complete shadow registration/emission with guarded work buffers and x86 ABI.

Only allocation, cylinder selection and release are controlled leaves. Both
float matrix helpers execute. The emitted-face model is independent of the C
pointer walk; the native companion compares the same buffers with original x86.
"""
import itertools
import json
import struct
import sys
from pathlib import Path

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK

POSITIONS = ((-2., 0., 1.), (0., 2., 1.), (2., 0., 1.),
             (-2., 0., -1.), (0., -2., -1.), (2., 0., -1.))
FACES = (((0, 1, 2),), ((3, 4, 5),), ((0, 1, 2), (3, 4, 5)),
         ((0, 1, 2), (0, 2, 4)), ())
IDENTITY = (1., 0., 0., 0., 0., 1., 0., 0., 0., 0., 1., 0.)


def cases():
    return itertools.product(range(1, 5), range(len(FACES)), (0, 65536, 131072, -65536))


class ShadowBuffers(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {self.addr(0x4aad70): (4, self.allocate)}

    def allocate(self):
        size, = self.args(1)
        at = self.cursor + 16
        self.cursor = at + ((size + 15) & ~15) + 16
        assert self.cursor < 0xff00
        self.allocations.append((at, size))
        self.u.reg_write(UC_X86_REG_EAX, HEAP + at)

    def checked(self, target, args):
        self.invoke(target, args)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff04 + 4 * len(args)

    def run(self, part_count, face_case, factor, project=False):
        self.reset()
        self.u.mem_write(HEAP, bytes([0xa5]) * 0x10000)
        self.cursor = 0x6000
        self.allocations = []
        faces = FACES[face_case]
        self.put(self.addr(0x6deab8), '<B', 0)
        self.u.mem_write(self.addr(0x6e0124), bytes(120))
        for i in range(part_count):
            node, mesh, triangles = HEAP + 0x1000 + i * 0x200, HEAP + 0x2000 + i * 0x200, HEAP + 0x3000 + i * 0x200
            self.u.mem_write(node, bytes(0x18c))
            self.u.mem_write(mesh, bytes(0x120))
            self.put(node + 4, '<I', node + 0x200 if i + 1 < part_count else 0)
            self.put(node + 12, '<I', mesh)
            self.put(node + 0x30, '<I', 5)
            self.put(node + 0x17c, '<B', 255)
            self.put(node + 0x118, '<16f', *IDENTITY, float(i), float(-i), float(i * 2), 1.)
            self.put(mesh + 0x10, '<i', len(POSITIONS))
            self.put(mesh + 0x24, '<Ii', triangles, len(faces))
            for j, face in enumerate(faces):
                self.put(triangles + j * 0x4c + 0x40, '<3H', *face)
        immutable = self.read(HEAP + 0x1000, 0x3000)
        self.checked(0x4b45d0, [HEAP + 0x1000, 1])
        caster, = struct.unpack('<I', self.read(self.addr(0x6e0124), 4))
        parts, = struct.unpack('<I', self.read(caster + 4, 4))
        assert self.read(caster + 8, 1) == bytes([part_count])
        assert [size for _, size in self.allocations] == [20, part_count * 88] + [72, 24, 72, len(faces) * 12, 288, 72] * part_count
        self.put(self.addr(0x6e0b38), '<i', 0xff0000)
        self.put(self.addr(0x6e0098), '<I', 0xff123456)
        self.put(self.addr(0x6e01b0), '<3f', 0., 0., 1.)
        self.put(self.addr(0x6e0220), '<9f', 0., 0., 1., 0., 1., 0., 1., 0., 0.)
        outputs = []
        for i in range(part_count):
            part = parts + i * 88
            pointers = struct.unpack('<6I', self.read(part + 0x38, 24))
            work, work2, flags, work3, vertices, triangles = pointers
            self.u.mem_write(work, struct.pack('<18f', *(v for p in POSITIONS for v in p)))
            self.u.mem_write(work2, struct.pack('<18f', *(v for p in POSITIONS for v in p)))
            self.checked(0x4b4180, [part, factor & 0xffffffff])
            face_count, vertex_count = struct.unpack('<2h', self.read(part + 0x50, 4))
            visible = [face for face in faces if any(POSITIONS[v][2] >= 0 for v in face)]
            assert face_count == len(visible)
            assert vertex_count == len(set(v for face in visible for v in face))
            edge_values = struct.unpack('<' + 'I' * (3 * len(faces)), self.read(triangles, 12 * len(faces)))
            edges = [(v - vertices) // 48 if j < face_count * 3 else v for j, v in enumerate(edge_values)]
            remap = {}
            expected_edges = []
            for face in visible:
                for vertex in face:
                    if vertex not in remap:
                        remap[vertex] = len(remap)
                    expected_edges.append(remap[vertex])
            assert edges[:face_count * 3] == expected_edges, 'shared vertices must use the buffer origin'
            outputs.append((self.read(part, 48), face_count, vertex_count,
                            self.read(flags, 24), self.read(work3, 72),
                            self.read(vertices, 288), tuple(edges)))
        assert self.read(HEAP + 0x1000, 0x3000) == immutable
        for at, size in self.allocations:
            assert self.read(HEAP + at - 16, 16) == bytes([0xa5]) * 16
            assert self.read(HEAP + at + size, 16) == bytes([0xa5]) * 16
        if project:
            item, owner, mesh, vertices, triangle = (HEAP + n for n in (0x4000, 0x4100, 0x4300, 0x4600, 0x4a00))
            self.u.mem_write(owner, bytes(0xa0))
            self.u.mem_write(mesh, bytes(0x120))
            self.put(item + 0x18, '<2i', 0, 0)
            self.put(item + 0x2c, '<I', owner)
            self.put(owner + 0xc, '<I', mesh)
            self.put(owner + 0x18, '<i', 65536)
            self.put(owner + 0x2c, '<i', 65536)
            self.put(mesh + 0xc, '<Ii', vertices, 4)
            self.put(mesh + 0x24, '<I', triangle)
            self.put(triangle + 4, '<i', 17)
            for j, (x, y) in enumerate(((-1., -1.), (1., -1.), (-1., 1.), (1., 1.))):
                self.put(vertices + j * 48, '<3f', x, y, 0.)
                self.put(vertices + j * 48 + 32, '<2f', (x + 1.) / 2., (y + 1.) / 2.)
            self.put(self.addr(0x6e01f0), '<I', HEAP + 0x5000)
            self.put(self.addr(0x6e0b90), '<I', HEAP + 0xa000)
            self.put(self.addr(0x6e0b94), '<i', 0)
            self.put(self.addr(0x6e0b98), '<B', 0)
            self.put(self.addr(0x5210d4), '<2i', -1, -1)
            self.checked(0x4b5770, [item, caster, 255])
            written, = struct.unpack('<i', self.read(self.addr(0x6e0b94), 4))
            assert 0 <= written <= 3 * len(faces) * part_count
            assert self.read(HEAP + 0xa000 + written * 48, 16) == bytes([0xa5]) * 16
            outputs.append((written, self.read(HEAP + 0xa000, written * 48),
                            self.read(HEAP + 0x5000, 16),
                            [self.read(struct.unpack('<I', self.read(parts + i * 88 + 0x48, 4))[0], 288)
                             for i in range(part_count)]))
        return outputs


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = ShadowBuffers(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = ShadowBuffers(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0
    for case in cases():
        assert original.run(*case, project=True) == rebuilt.run(*case, project=True), case
        count += 1
    print(f'PASS: {count} shadow registration/emission/projection cases; full guarded buffers, shared vertices, complete batches, emitted-face model and x86 ABI')


if __name__ == '__main__':
    main()
