#!/usr/bin/env python3
"""Compare complete mesh partitioning, allocations and rebased index buffers.

Only the allocator is intercepted. Check contiguous texture groups against an
independent model as well as original code, with guarded memory and the ABI.
"""
import argparse
from pathlib import Path
import random
import struct

from differential_car_forces import object_body, SCRATCH
from differential_car_body_matrix import SAVED
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)

ADDRESS = 0x4b1ac0


class Parts(Drawing):
    def __init__(self, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.code = code
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)
        self.callbacks = {0x4aad70: (4, self.allocate)}

    def allocate(self):
        size, = self.args(1)
        pointer = HEAP + self.cursor
        assert 0 < size <= 0x100 and self.cursor + size < 0xf000
        self.allocations.append((pointer, size))
        self.cursor += ((size + 15) & ~15) + 32
        self.u.reg_write(UC_X86_REG_EAX, pointer)
        self.u.reg_write(UC_X86_REG_ECX, 0x12345678)
        self.u.reg_write(UC_X86_REG_EDX, 0x87654321)

    def run(self, seed, model=True):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        self.cursor, self.allocations = 0x8000, []
        mesh, triangles = HEAP + 0x1000, HEAP + 0x2000
        existing = seed % 17 == 0
        count = (-1, 0, 1, 2, 3, 8, 16, 24)[seed % 8]
        if seed == 1:
            count, existing = 1, False
        self.put(mesh + 0x24, '<Ii', triangles, count)
        self.put(mesh + 0x100, '<i', 2 if existing else 0)
        groups = []
        texture = -99
        for i in range(max(count, 0)):
            if i == 0 or rnd.randrange(3) == 0:
                texture = rnd.choice((-1, 0, 1, 2, 17, 49))
            field = rnd.randrange(0x100000000)
            indices = [rnd.randrange(2000) for _ in range(3)]
            indices[0] = rnd.randrange(999)
            if seed == 1:
                indices = [998, 999, 998]
            self.put(triangles + i*0x4c + 4, '<2I', texture & 0xffffffff, field)
            self.put(triangles + i*0x4c + 0x40, '<3H', *indices)
            if not groups or groups[-1][0] != texture:
                groups.append((texture, field, []))
            groups[-1][2].extend(indices)
        expected = bytearray(self.read(HEAP, 0x10000))
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<2I', STOP, mesh)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.u.emu_start(ADDRESS if self.code is None else SCRATCH, STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'partitioner did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 8, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        if model:
            if existing:
                assert not self.allocations
            else:
                assert len(self.allocations) == 2*len(groups)
                struct.pack_into('<i', expected, 0x1100, len(groups))
                for i, (texture, field, indices) in enumerate(groups):
                    part, size = self.allocations[i]
                    data, data_size = self.allocations[len(groups) + i]
                    assert (size, data_size) == (0x1c, 2*len(indices))
                    lo, hi = min(indices), max(indices)
                    struct.pack_into('<I', expected, 0x1038 + i*4, part)
                    struct.pack_into('<6I', expected, part - HEAP,
                                     texture & 0xffffffff, field, lo, hi, len(indices), data)
                    struct.pack_into('<' + 'H'*len(indices), expected, data - HEAP,
                                     *(value - lo for value in indices))
            assert self.read(HEAP, 0x10000) == expected, ('index model or memory guards', seed)
        return self.read(HEAP, 0x10000), self.allocations


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=600)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    code = object_body(args.object, ADDRESS, 'Mesh.cpp')
    original, rebuilt = Parts(), Parts(code)
    for seed in range(args.cases):
        if original.run(seed) != rebuilt.run(seed):
            print(f'FAIL mesh parts seed {seed}')
            return 1
    damaged = bytearray(code)
    at = damaged.find(struct.pack('<I', 999))
    assert at >= 0, 'minimum-index initial bound missing'
    struct.pack_into('<I', damaged, at, 998)
    negative = Parts(bytes(damaged))
    assert original.run(1) != negative.run(1, model=False), 'negative control passed'
    print(f'PASS mesh parts: {args.cases} cases, independent index model, original allocation trace, guarded memory and ABI identical; negative control detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
