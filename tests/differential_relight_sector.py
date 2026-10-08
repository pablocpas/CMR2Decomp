#!/usr/bin/env python3
"""Compare complete sector relighting, mesh outputs, shadow zones and call order.

Only colour lookup, mesh upload and external attenuation leaves are intercepted.
The original and candidate execute all traversal, guards and fixed-point maths.
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

ADDRESS = 0x4b3c00


class Relight(Drawing):
    def __init__(self, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.code = code
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)
        self.callbacks = {
            0x4b3b40: (8, lambda: self.colour('light', 0x13579bdf)),
            0x4b3ba0: (8, lambda: self.colour('shadow', 0x2468ace0)),
            0x4b2020: (4, self.refresh),
            0x4b2090: (8, self.uniform),
            0x4a3240: (4, lambda: self.result('node', self.args(1), 0)),
            0x4b6ca0: (8, lambda: self.result('attenuate', self.args(2), self.seed % 3 != 0)),
        }

    def result(self, name, args, value):
        self.trace.append((name, *args))
        self.u.reg_write(UC_X86_REG_EAX, int(value) & 0xffffffff)
        self.u.reg_write(UC_X86_REG_ECX, 0x12345678)
        self.u.reg_write(UC_X86_REG_EDX, 0x87654321)

    def colour(self, name, mask):
        dest, intensity = self.args(2)
        value = intensity ^ mask
        self.put(dest, '<I', value)
        # One branch uses a local DWORD; normalize that address to its value.
        self.result(name, (None if STACK <= dest < STACK + 0x10000 else dest, intensity, value), value)

    def refresh(self):
        mesh, = self.args(1)
        vertices, count = struct.unpack('<Ii', self.read(mesh + 0xc, 8))
        self.result('refresh', (mesh, self.read(vertices, max(count, 0) * 0x30)), 0)

    def uniform(self):
        mesh, colour = self.args(2)
        vertices, count = struct.unpack('<Ii', self.read(mesh + 0xc, 8))
        for i in range(max(count, 0)):
            self.put(vertices + i*0x30 + 0x18, '<I', colour)
        self.result('uniform', (mesh, colour), 0)

    def run(self, seed):
        self.seed = seed
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        sector = seed % 4
        mode = (seed // 4) % 10
        ambient, light = rnd.randbytes(4), rnd.randbytes(4)
        self.u.mem_write(0x6e01ec, ambient)
        self.u.mem_write(0x6dfdc4, light)
        self.put(0x6e019c, '<I', 0 if mode == 0 else HEAP + 0x100)
        self.put(0x6dfd90, '<I', HEAP + 0x200)
        self.u.mem_write(HEAP + 0x100 + sector*4, ambient)
        self.u.mem_write(HEAP + 0x200 + sector*4, light)
        if mode in (2, 3, 4, 5, 6, 7):
            address = HEAP + (0x100 if seed % 2 else 0x200) + sector*4 + (seed // 40) % 3
            old = self.read(address, 1)[0]
            self.put(address, '<B', old ^ 128)
        # Alpha alone does not trigger a relight.
        if mode == 9:
            self.put(HEAP + 0x100 + sector*4 + 3, '<B', ambient[3] ^ 255)
        self.put(0x71f608 + sector*4, '<I', 0 if mode == 2 else HEAP + 0x1000)
        self.put(HEAP + 0x1010, '<4I', 0 if mode == 3 else HEAP + 0x2000,
                 0 if mode == 4 else HEAP + 0x5000, 0, HEAP + 0x6000)
        self.put(HEAP + 0x5098, '<Ii', HEAP + 0x5100, rnd.randrange(-65536, 200001))
        self.put(HEAP + 0x5198, '<Ii', 0, rnd.randrange(-65536, 200001))
        self.put(HEAP + 0x500c, '<I', HEAP + 0x2100)
        self.put(HEAP + 0x510c, '<I', HEAP + 0x2200)
        self.put(HEAP + 0x6170, '<I', HEAP + 0x6200)
        self.put(HEAP + 0x6370, '<I', 0)
        self.put(0x6dfd98, '<I', 0 if mode == 5 else HEAP + 0x300)
        self.put(HEAP + 0x300 + sector*4, '<I', 0 if mode == 6 else HEAP + 0x2300)
        self.put(0x5210c0, '<i', (seed // 40) % 2)
        self.put(0x6dfdbc, '<I', HEAP + 0x7000)
        self.put(0x6e0204, '<I', HEAP + 0x400)
        self.put(HEAP + 0x400 + sector*2, '<h', 1)
        zones = (seed // 80) % 4
        self.put(HEAP + 0x7016, '<H', zones)
        self.put(HEAP + 0x7020, '<I', HEAP + 0x7100)
        shadow_count = 0
        for i in range(zones):
            count = (seed // 320 + i) % 4
            intensity = (0, 1, 32768, 65535, 65536, 65537, -65536)[(seed // 160 + i) % 7]
            self.put(HEAP + 0x7100 + i*0x30 + 0x20, '<i', intensity)
            self.put(HEAP + 0x7100 + i*0x30 + 0x28, '<i', count)
            shadow_count += count
        for i in range(4):
            mesh = HEAP + 0x2000 + i*0x100
            count = shadow_count if i == 3 and (seed // 40) % 2 else (seed // 160 + i) % 6
            vertices, levels = HEAP + 0x3000 + i*0x600, HEAP + 0x8000 + i*0x100
            self.put(mesh + 0xc, '<Ii', vertices, count)
            self.put(mesh + 0x30, '<2I', 0x80 if i % 2 == 0 else 0, levels)
            for j in range(count):
                self.put(levels + j*4, '<i', rnd.randrange(-100000, 200001))
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<2I', STOP, sector)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(ADDRESS if self.code is None else SCRATCH, STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'relight did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 8, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        return self.read(HEAP, 0x10000), self.trace


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=1600)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    code = object_body(args.object, ADDRESS, 'SceneNode.cpp')
    original, rebuilt = Relight(), Relight(code)
    for seed in range(args.cases):
        if original.run(seed) != rebuilt.run(seed):
            print(f'FAIL sector relight seed {seed}: {original.trace} / {rebuilt.trace}')
            return 1
    damaged = bytearray(code)
    at = damaged.find(struct.pack('<I', 0x10000))
    assert at >= 0, 'full-intensity branch missing'
    struct.pack_into('<I', damaged, at, 0x10001)
    negative = Relight(bytes(damaged))
    assert any(original.run(seed) != negative.run(seed) for seed in range(600, 1000)), 'negative control passed'
    print(f'PASS sector relight: {args.cases} cases, mesh colours, zones, calls, guarded state and ABI identical; negative control detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
