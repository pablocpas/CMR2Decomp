#!/usr/bin/env python3
"""Compare complete firework updates in the original and rebuilt x86 images.

Usage: differential_fireworks.py entities.json [rebuilt.exe]
Random numbers and sound/debris leaves are controlled. Compare all rocket and
scratch memory, flash state, call arguments, RNG consumption and preserved
registers across rising, bursting, fading and inactive rockets.
"""
import json
import math
from pathlib import Path
import random
import struct
import sys

import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ESI,
                              UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP,
                              UC_X86_REG_EIP, UC_X86_REG_FPCW)
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP

SAVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


class Fireworks(Drawing):
    def __init__(self, path, entities=None):
        pe = pefile.PE(str(path))
        self.memory = pe.get_memory_mapped_image()
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.entities = entities
        self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        self.u.mem_map(self.base, (pe.OPTIONAL_HEADER.SizeOfImage + 4095) & ~4095)
        self.u.mem_map(HEAP, 0x10000)
        self.u.mem_map(STACK, 0x10000)
        self.u.mem_map(STOP, 4096)
        self.callbacks = {}
        for address, name, cleanup in [(0x4c6676, 'rand', 0), (0x4b78a0, 'playing', 4),
                                       (0x4b78d0, 'free', 4), (0x4b7790, 'play', 24),
                                       (0x47fcb0, 'debris', 12)]:
            self.callbacks[self.addr(address)] = (cleanup, lambda n=name, c=cleanup: self.leaf(n, c))
        self.u.hook_add(UC_HOOK_CODE, self.hook)
        # The game's startup fills this BSS table; PE loading alone leaves it zero.
        self.sqrt_table = struct.pack('<4096H', *(math.isqrt((8 + 16 * i) * 65536) for i in range(4096)))

    def leaf(self, name, cleanup):
        args = self.args(cleanup // 4)
        if name == 'rand':
            self.seed = (self.seed * 214013 + 2531011) & 0xffffffff
            result = (self.seed >> 16) & 0x7fff
            if self.case % 13 == 0:
                result = (0, 16383, 16384, 32767)[len(self.trace) % 4]
            self.trace.append((name, result))
        elif name == 'debris':
            position, velocity, kind = args
            self.rocket_index(position)
            assert STACK <= velocity < STACK + 0x10000
            self.trace.append((name, struct.unpack('<3i', self.read(position, 12)),
                               struct.unpack('<3i', self.read(velocity, 12)), kind))
            result = 0
        else:
            self.trace.append((name, args))
            result = self.case % 2 if name == 'playing' else 0
        self.u.reg_write(UC_X86_REG_EAX, result)

    @staticmethod
    def rocket_index(position):
        index, remainder = divmod(position - HEAP - 0x100, 0x938)
        assert 0 <= index < 4 and remainder == 0
        return index

    def run(self, case, rounding, initial):
        self.case = case
        self.seed = case + 42
        self.trace = []
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(self.addr(0x6e0ef4), self.sqrt_table)
        self.u.mem_write(HEAP, initial)
        self.u.mem_write(STACK, bytes(0x10000))
        for address, value in [(0x590af8, HEAP + 0x100), (0x590afc, case % 5),
                               (0x590b0c, HEAP + 0x5000), (0x5909c4, HEAP + 0x7000),
                               (0x5909bc, 11), (0x5909b8, 1234)]:
            self.put(self.addr(address), '<I', value)
        self.put(self.addr(0x5112f0), '<f', 1.0 / 32767)
        self.put(self.addr(0x5112e0), '<d', 65536.0)
        self.put(self.addr(0x5112e8), '<d', -65536.0)
        sp = STACK + 0xff00
        self.put(sp, '<I', STOP)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.u.reg_write(UC_X86_REG_FPCW, 0x27f | rounding)
        for reg in SAVED:
            self.u.reg_write(reg, 0x12345678 + reg)
        self.u.emu_start(self.addr(0x47eab0), STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'fireworks did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 4, 'unbalanced stack'
        assert all(self.u.reg_read(reg) == 0x12345678 + reg for reg in SAVED), 'preserved register changed'
        return self.read(HEAP, 0x10000), self.read(self.addr(0x5909b8), 4), self.trace, self.seed


def fixture(case):
    rng = random.Random(case)
    data = bytearray(rng.randbytes(0x10000))

    def write(offset, value):
        struct.pack_into('<I', data, offset, value & 0xffffffff)

    for rocket in range(case % 5):
        at = 0x100 + rocket * 0x938
        for offset in range(0, 0x674, 4):
            write(at + offset, rng.randrange(-0x80000, 0x80000))
        write(at + 0x694, (0, 1, 2, 3, 0xffffffff)[(case + rocket) % 5])
        write(at + 0x698, case % 4)
        write(at + 0x660, 0x3000)
        write(at + 0x664, (0, 1, 65536, 65537, 131072)[case % 5])
        write(at + 0x668, (0, 1, 65536, 65537)[case % 4])
        write(at + 0x66c, 0x666)
        write(at + 0x670, 0x400)
        if case % 7 == 0:
            for offset in (0xc, 0x10, 0x14, 0x66c):
                write(at + offset, 0)
        data[at + 0x690] = (0, 1, 3, 4, 16, 19)[(case + rocket) % 6]
        data[at + 0x691] = (case * 3 + rocket) % 20
        data[at + 0x692] = case % 6
        data[at + 0x693] = (0xff, 0, 3, 0x80)[case % 4]
        write(at + 0x92c, (case // 5) % 2)
        write(at + 0x934, case % 3)
    for row in range(3):
        write(0x5000 + row * 4, HEAP + 0x5100 + row * 0x24)
        for component in range(9):
            write(0x5100 + row * 0x24 + component * 4, rng.randrange(-65536, 65536))
    return bytes(data)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Fireworks(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Fireworks(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    for case in range(300):
        initial = fixture(case)
        for rounding in (0, 0x400, 0x800, 0xc00):
            a, b = original.run(case, rounding, initial), rebuilt.run(case, rounding, initial)
            if a != b:
                print('FAIL fireworks case', case, 'rounding', hex(rounding))
                differences = [(hex(i), x, b[0][i]) for i, x in enumerate(a[0]) if x != b[0][i]]
                print('memory differences:', differences[:10])
                print('original flash/calls/RNG:', a[1:])
                print('rebuilt flash/calls/RNG:', b[1:])
                return 1
    print('1200 fireworks cases: identical rocket memory, flash state, calls and RNG in all four x87 rounding modes')
    return 0


if __name__ == '__main__':
    sys.exit(main())
