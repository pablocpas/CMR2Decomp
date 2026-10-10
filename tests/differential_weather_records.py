#!/usr/bin/env python3
"""Check weather record widths, transitions, shelter fades and interpolation.

Run full original/rebuilt bodies with independent signed fixed-point models,
poisoned records, complete heap guards and stdcall register/stack checks.
Camera/terrain integration is a controlled provider here; its complete body
is covered by differential_camera_deformation.py.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX,
                              UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP, UC_X86_REG_ESP)
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting, add, mul, signed

WEATHER, RAMPS = HEAP + 0x1000, HEAP + 0x3000
SAVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def store(heap, address, fmt, *values):
    struct.pack_into(fmt, heap, address - HEAP, *values)


class WeatherRecords(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x406910, 0), (0x406930, 0), (0x45F9D0, 3)):
            self.callbacks[self.addr(address)] = (
                nargs * 4, lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        if address == 0x45F9D0:
            self.trace.append(tuple(self.args(nargs)))
        value = self.country if address == 0x406910 else self.stage if address == 0x406930 else 0
        for register, number in ((UC_X86_REG_EAX, value),
                                 (UC_X86_REG_ECX, 0xA5A5A5A5),
                                 (UC_X86_REG_EDX, 0x5A5A5A5A)):
            self.u.reg_write(register, number)

    def prepare(self, seed):
        self.reset()
        self.country, self.stage = 0, 0
        heap = bytearray(random.Random(seed).randbytes(0x10000))
        self.u.mem_write(HEAP, bytes(heap))
        self.u.mem_write(STACK, bytes([seed & 255]) * 0x10000)
        self.put(self.addr(0x547AC8), '<I', WEATHER)
        self.put(self.addr(0x543EB8), '<I', RAMPS)
        return heap

    def check(self, function, args, heap, trace=()):
        saved = {reg: 0x13579BDF + i * 0x11111111 for i, reg in enumerate(SAVED)}
        for reg, value in saved.items():
            self.u.reg_write(reg, value)
        self.invoke(function, [value & 0xFFFFFFFF for value in args])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF04 + 4 * len(args), 'stdcall cleanup'
        assert all(self.u.reg_read(reg) == value for reg, value in saved.items()), 'callee-saved registers'
        actual = self.read(HEAP, 0x10000)
        assert actual == heap, (hex(function), 'record model / heap guards',
                               next((hex(i) for i, (a, b) in enumerate(zip(actual, heap)) if a != b), None))
        assert self.trace == list(trace), (hex(function), 'integration calls', self.trace, trace)
        return actual, list(self.trace)

    def scale(self, view, intensity, capacity, quality, seed):
        heap = self.prepare(seed)
        record = WEATHER + view * 0x178
        self.put(record + 0x74, '<h', capacity)
        store(heap, record + 0x74, '<h', capacity)
        self.put(self.addr(0x543D50), '<i', quality)
        store(heap, record + 0x54, '<i', intensity)
        store(heap, record + 0x5C, '<i', mul(mul(quality, intensity), signed(capacity << 16)))
        return self.check(0x460C30, [intensity, view], heap)

    def transition(self, current, target, count, capacity, intensity, quality, seed):
        heap = self.prepare(seed)
        for offset, fmt, value in ((0, '<i', current), (4, '<i', target),
                                   (0x54, '<i', intensity), (0x58, '<i', count),
                                   (0x74, '<h', capacity)):
            self.put(WEATHER + offset, fmt, value)
            store(heap, WEATHER + offset, fmt, value)
        self.put(self.addr(0x543D50), '<i', quality)
        trace = []
        if current != target:
            if current == 0 and target in (1, 2):
                store(heap, WEATHER + 0x58, '<i', 0)
                store(heap, WEATHER + 0x5C, '<i', mul(mul(quality, intensity), signed(capacity << 16)))
                store(heap, WEATHER, '<i', target)
                trace.append((0, WEATHER, 3))
            elif current in (1, 2) and target == 0:
                store(heap, WEATHER + 0x5C, '<i', 0)
                if count == 0:
                    store(heap, WEATHER, '<i', 0)
        return self.check(0x460B60, [WEATHER, 3], heap, trace)

    def shelter(self, country, stage, route, fade, step, seed):
        heap = self.prepare(seed)
        self.country, self.stage = country, stage
        for offset, fmt, value in ((0x6C, '<I', route), (0x64, '<i', fade)):
            self.put(WEATHER + offset, fmt, value)
            store(heap, WEATHER + offset, fmt, value)
        self.put(self.addr(0x51BD3C), '<i', step)
        sheltered = (country == 6 and stage == 0 and 0x66 < route < 0x6D or
                     country == 7 and stage == 2 and 0x1D5 < route < 0x1D8)
        store(heap, WEATHER + 0x174, '<i', int(sheltered))
        delta = mul(0x3333, step)
        fade = max(add(fade, -delta), 0) if sheltered else min(add(fade, delta), 0x10000)
        store(heap, WEATHER + 0x64, '<i', fade)
        return self.check(0x45E9A0, [WEATHER], heap)

    def interpolate(self, count, factor, variant, seed):
        heap = self.prepare(seed + variant * 65537)
        self.put(self.addr(0x547ACC), '<I', count)
        for i in range(count):
            address = RAMPS + i * 0x2C
            blend, yaw, old_blend, _, old_yaw = struct.unpack_from('<5i', heap, address - HEAP + 0x10)
            store(heap, address + 0x1C, '<i', add(mul(factor, add(blend, -old_blend)), old_blend))
            store(heap, address + 0x24, '<i', add(old_yaw, mul(factor, add(yaw, -old_yaw))))
        return self.check(0x461BB0, [factor], heap)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    a = WeatherRecords(ROOT / 'cmr2bin/CMR2.exe')
    b = WeatherRecords(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0

    def compare(method, cases):
        nonlocal count
        for case in cases:
            assert getattr(a, method)(*case, count) == getattr(b, method)(*case, count), (method, case)
            count += 1

    compare('scale', itertools.product((0, 1, 3), (-0x10000, 0, 1, 0x8000, 0x10000, 0x7FFFFFFF),
                                      (-32768, -1, 0, 100, 400, 32767), (0, 0x4000, 0x10000)))
    compare('transition', itertools.product((-1, 0, 1, 2, 3), (-1, 0, 1, 2, 3),
                                           (0, 1, 0x10000), (0, 100, 400),
                                           (0, 0x8000, 0x10000), (0, 0x4000, 0x10000)))
    compare('shelter', itertools.product((0, 6, 7), (0, 1, 2),
                                        (0, 0x66, 0x67, 0x6C, 0x6D, 0x1D5, 0x1D6, 0x1D7, 0x1D8, 0xFFFFFFFF),
                                        (0, 1, 0x8000, 0xFFFF, 0x10000), (0, 0x8000, 0x10000)))
    compare('interpolate', itertools.product((0, 1, 2, 4),
                                            (-0x80000000, -0x10000, -1, 0, 1, 0x8000, 0x10000, 0x7FFFFFFF),
                                            range(4)))
    print(f'{count} weather record cases: independent fixed-point models, signed widths, '
          'shelter boundaries, transitions, interpolation, complete heap guards and ABI')
    return 0


if __name__ == '__main__':
    sys.exit(main())
