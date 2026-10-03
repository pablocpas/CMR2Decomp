#!/usr/bin/env python3
"""Direct fixed matrix product and basis integration, without controlled leaves.

Usage: differential_fixed_matrix_ops.py entities.json [rebuilt.exe]
Independent integer models with per-product 16-bit shifts and 32-bit wrap.
Tests full guarded heap, in-place/partial aliases, counter wrap and ABI.
Pointers refer to mapped buffers; no invalid/null pointer behavior is claimed.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn import UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EBX, UC_X86_REG_ESI,
                               UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP)
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting


LIMITS = (-0x80000000, -0x10001, -0x10000, -1, 0, 1,
          0x7fff, 0xffff, 0x10000, 0x10001, 0x7fffffff)
PRODUCT_LAYOUTS = ((0x1000, 0x2000, 0x3000), (0x2000, 0x2000, 0x3000),
                   (0x3000, 0x2000, 0x3000), (0x1000, 0x2000, 0x2000),
                   (0x2000, 0x2000, 0x2000), (0x2004, 0x2000, 0x3000),
                   (0x2ff4, 0x2000, 0x3000), (0x2020, 0x2000, 0x2004),
                   (0x1001, 0x2003, 0x3002))
INTEGRATE_LAYOUTS = ((0x1000, 0x2000), (0x1000, 0x1000), (0x1000, 0x100c),
                     (0x1000, 0x1018), (0x1000, 0x1004), (0x1000, 0x1010),
                     (0x1001, 0x2003))


def signed(value):
    value &= 0xffffffff
    return value if value < 0x80000000 else value - 0x100000000


def mul(a, b):
    return signed((a * b) >> 16)


def values(seed, count, salt=0):
    rnd = random.Random(seed * 19 + salt)
    if seed == 0:
        return [0] * count
    if seed < len(LIMITS) + 1:
        return [LIMITS[(seed + salt + i) % len(LIMITS)] for i in range(count)]
    if seed < 24:
        return [rnd.choice(LIMITS) for _ in range(count)]
    return [signed(rnd.getrandbits(32)) for _ in range(count)]


class FixedOps(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.running = False
        self.allowed_globals = set()
        self.u.hook_add(UC_HOOK_MEM_WRITE, self.global_write,
                        begin=self.base, end=self.base + self.size - 1)

    def global_write(self, u, access, address, size, value, data):
        if self.running:
            assert (address, size) in self.allowed_globals, ('unexpected global write', hex(address), size)

    def prepare(self, seed):
        self.running = False
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(STACK, bytes(0x10000))
        self.u.mem_write(HEAP, random.Random(seed).randbytes(0x10000))

    def checked_invoke(self, target, args):
        saved = {r: 0x13579bdf + i * 0x11111111 for i, r in enumerate(
            (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP))}
        for register, value in saved.items():
            self.u.reg_write(register, value)
        self.running = True
        try:
            self.invoke(target, args)
        finally:
            self.running = False
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff04 + len(args) * 4, 'ABI cleanup'
        assert all(self.u.reg_read(register) == value for register, value in saved.items()), 'callee-saved registers'

    def product(self, seed, layout, counter):
        self.prepare(seed)
        out, a, b = (HEAP + offset for offset in layout)
        self.put(a, '<16i', *values(seed, 16))
        self.put(b, '<16i', *values(seed, 16, 7))
        aa = struct.unpack('<16i', self.read(a, 64))
        bb = struct.unpack('<16i', self.read(b, 64))
        expected = bytearray(self.read(HEAP, 0x10000))
        result = [signed(sum(mul(aa[i * 4 + k], bb[k * 4 + j]) for k in range(4)))
                  for i in range(4) for j in range(4)]
        struct.pack_into('<16i', expected, out - HEAP, *result)
        count = self.addr(0x72d67c)
        self.put(count, '<I', counter)
        self.allowed_globals = {(count, 4)}
        self.checked_invoke(0x4b9f20, [out, a, b])
        actual = self.read(HEAP, 0x10000)
        assert actual == expected, ('matrix product model/guards', seed, layout, counter)
        assert self.read(count, 4) == struct.pack('<I', (counter + 1) & 0xffffffff), 'counter wrap'
        return actual

    def integrate(self, seed, layout):
        self.prepare(seed)
        rows, velocity = (HEAP + offset for offset in layout)
        self.put(rows, '<9i', *values(seed, 9))
        self.put(velocity, '<3i', *values(seed, 3, 5))
        expected = bytearray(self.read(HEAP, 0x10000))
        for i in range(3):
            # Aliased angular velocity must be re-read after each row update.
            w = struct.unpack_from('<3i', expected, velocity - HEAP)
            v = struct.unpack_from('<3i', expected, rows - HEAP + i * 12)
            cross = [signed(mul(w[(j + 1) % 3], v[(j + 2) % 3]) -
                            mul(w[(j + 2) % 3], v[(j + 1) % 3])) for j in range(3)]
            struct.pack_into('<3i', expected, rows - HEAP + i * 12,
                             *(signed(v[j] + cross[j]) for j in range(3)))
        self.allowed_globals = set()
        self.checked_invoke(0x441430, [rows, velocity])
        actual = self.read(HEAP, 0x10000)
        assert actual == expected, ('basis integration model/guards', seed, layout)
        return actual


def mutation_probe(original_path):
    product = FixedOps(original_path)
    memory = bytearray(product.memory)
    offset = 0x4b9f9a - product.base
    assert memory[offset:offset + 2] == bytes.fromhex('f7ea')
    memory[offset + 1] = 0xe2  # unsigned multiply must differ on signed inputs
    product.memory = bytes(memory)
    integration = FixedOps(original_path)
    memory = bytearray(integration.memory)
    offset = 0x44146e - integration.base
    assert memory[offset:offset + 4] == bytes.fromhex('0fa4c210')
    memory[offset + 3] = 15  # wrong fixed-point shift
    integration.memory = bytes(memory)
    detected = 0
    for instance, invoke in ((product, lambda: product.product(2, PRODUCT_LAYOUTS[0], 0)),
                             (integration, lambda: integration.integrate(2, INTEGRATE_LAYOUTS[0]))):
        try:
            invoke()
        except AssertionError as error:
            assert 'model/guards' in str(error), error
            detected += 1
    assert detected == 2, 'representative arithmetic mutations survived'


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    path = ROOT / 'cmr2bin/CMR2.exe'
    original = FixedOps(path)
    rebuilt = FixedOps(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0
    for case in itertools.product(range(96), PRODUCT_LAYOUTS, (0, 0x7fffffff, 0xffffffff)):
        assert original.product(*case) == rebuilt.product(*case), ('differential product', case)
        count += 1
    print(count, 'fixed matrix products: model, full heap, partial/in-place aliases, counter wrap and ABI identical')
    count = 0
    for case in itertools.product(range(96), INTEGRATE_LAYOUTS):
        assert original.integrate(*case) == rebuilt.integrate(*case), ('differential integration', case)
        count += 1
    print(count, 'basis integrations: signed wrap, sequential aliases, model, full heap and ABI identical')
    mutation_probe(path)
    print('2 arithmetic mutations rejected: unsigned multiply and wrong fixed-point shift')
    return 0


if __name__ == '__main__':
    sys.exit(main())
