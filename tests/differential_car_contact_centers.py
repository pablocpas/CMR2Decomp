#!/usr/bin/env python3
"""Original/rebuilt tyre trail centres from guarded wheel contact records.

Runs the actual contact getter and fixed-point helpers. An independent integer
midpoint model covers all eight cars/four wheels, signed wrap, odd differences,
null slot-zero records, upper-bound returns, read-only inputs and callee ABI.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import (UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EBX,
                              UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from differential_stage_lighting import signed

CONTACTS = HEAP + 0x1000
RECORD_SIZE = 0x2a4
CENTRES_SIZE = 8 * 4 * 12
GUARD = 64
SAVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


class ContactCenters(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}

    def hook(self, u, address, size, data):
        if address == self.addr(0x498570):
            self.trace.append(self.args(1)[0])
        super().hook(u, address, size, data)

    def run_centres(self, seed, car, null=False):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        self.put(self.addr(0x592734), '<I', 0 if null else CONTACTS)
        output = self.addr(0x549c20)
        self.u.mem_write(output - GUARD, rnd.randbytes(CENTRES_SIZE + 2 * GUARD))
        limits = [-2**31, 2**31-1, -65537, -1, 0, 1, 65537, -0x100000]
        for slot, wheel, component in itertools.product(range(8), range(4), range(3)):
            k = seed + slot * 3 + wheel + component
            for edge, start in enumerate([0x198, 0x1c8]):
                value = limits[(k + edge * 3) % len(limits)] if seed % 2 else signed(rnd.getrandbits(32))
                self.put(CONTACTS + slot * RECORD_SIZE + start + wheel * 12 + component * 4, '<i', value)
        heap_before = self.read(HEAP, 0x10000)
        pointer_before = self.read(self.addr(0x592734), 4)
        expected = bytearray(self.read(output - GUARD, CENTRES_SIZE + 2 * GUARD))
        active = car < 8 and not null
        if active:
            for wheel in range(4):
                front = struct.unpack_from('<3i', heap_before, CONTACTS-HEAP + car * RECORD_SIZE + 0x198 + wheel * 12)
                rear = struct.unpack_from('<3i', heap_before, CONTACTS-HEAP + car * RECORD_SIZE + 0x1c8 + wheel * 12)
                centre = [signed(b + (signed(a - b) >> 1)) for a, b in zip(front, rear)]
                centre[1] = signed(centre[1] + 0xccc)
                struct.pack_into('<3i', expected, GUARD + (car * 4 + wheel) * 12, *centre)
        sp = STACK + 0xff00
        self.put(sp, '<2I', STOP, car)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        for i, register in enumerate(SAVED):
            self.u.reg_write(register, 0x13570000 + i)
        self.trace = []
        self.u.emu_start(self.addr(0x4657d0), STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 8
        assert all(self.u.reg_read(r) == 0x13570000+i for i, r in enumerate(SAVED))
        assert self.read(HEAP, 0x10000) == heap_before, (seed, car, null, 'input/heap guard')
        assert self.read(self.addr(0x592734), 4) == pointer_before
        actual = self.read(output - GUARD, CENTRES_SIZE + 2 * GUARD)
        assert actual == expected, (seed, car, null, 'midpoint/output guard')
        expected_trace = [car] * (9 if active else 1) if car < 8 else []
        assert self.trace == expected_trace, (seed, car, null, 'real getter trace', self.trace)
        return actual, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = ContactCenters(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = ContactCenters(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = list(itertools.product(range(32), range(10), [False]))
    cases.extend((seed, 0, True) for seed in range(32))
    for case in cases:
        assert original.run_centres(*case) == rebuilt.run_centres(*case), case
    print(f'{len(cases)} tyre centre cases: independent signed midpoint model, guarded contacts/outputs, actual getter/helpers and callee ABI')
    return 0


if __name__ == '__main__':
    sys.exit(main())
