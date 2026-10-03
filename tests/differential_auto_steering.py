#!/usr/bin/env python3
"""Exercise automatic steering with real ramp/torque/alignment helpers.

Usage: differential_auto_steering.py entities.json [rebuilt.exe]
Independent integer and rational x87 model, full guarded car memory, helper
trace and stack cleanup. Includes signed wrap and short-angle boundaries;
fixtures do not exercise the complete automatic gearbox or driving simulation.
"""
import itertools
import json
from fractions import Fraction
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_FPCW
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting


def signed(value, bits=32):
    return ((value + 2 ** (bits - 1)) % 2**bits) - 2 ** (bits - 1)


def mul(a, b):
    return signed((a * b) >> 16)


def rounded(value, mode):
    quotient, remainder = divmod(value.numerator, value.denominator)
    if mode == 0:
        twice = remainder * 2
        return quotient + (twice > value.denominator or
                           (twice == value.denominator and quotient % 2 != 0))
    if mode == 0x400:
        return quotient
    if mode == 0x800:
        return quotient + bool(remainder)
    return quotient + (value < 0 and bool(remainder))


def target_angle(value, coefficient, mode):
    product = Fraction(value) * Fraction.from_float(coefficient)
    if product:
        absolute = abs(product)
        exponent = absolute.numerator.bit_length() - absolute.denominator.bit_length()
        if absolute < Fraction(2)**exponent:
            exponent -= 1
        unit = Fraction(2)**(exponent - 52)
        product = rounded(product / unit, mode) * unit
    return signed(rounded(product, mode), 16)


def model(seed, current, flags, bits, rounding, coefficient):
    left, right = flags
    speed = (0, 1, 0x4000, 0x8000, 0x10000, 0x1FFFF)[seed % 6]
    grip = (0, 1, 0x4000, 0x8000, 0x10000, 0x18000)[seed % 6]
    scale_base = (0, 1, 64, -64, 128, -128)[seed % 6]
    alignment = abs(mul(12345 + seed * 1999, 50000) +
                    mul(-18300 + seed * 771, -30000) +
                    mul(8191 - seed * 277, 10000))
    amount = min(mul(alignment, (seed % 4) * 0x8000), 0xB333)
    scale = mul(0x10000 - amount, scale_base) * 0x1680 if bits & 1 else scale_base * 0x1680
    ramp = (-0x10001, -0x10000, -1, 0, 1, 0x10000)[seed % 6]
    if left:
        ramp = min(max(ramp, 0) + 0x10000, 0x10000)
    elif right:
        ramp = max(min(ramp, 0) - 0x10000, -0x10000)
    else:
        ramp = 0
    torque = mul((0, 1, 0x1000, 0x8000, 0x10000, 0x20000)[seed % 6],
                 (0, 0x8000, 0x10000)[seed % 3])
    if bits & 2:
        swing = signed(signed(-current) if current < 0 else current)
        swing = signed(swing - 0x10000)
        torque = mul(torque, mul(swing, swing))
    torque = mul(torque, abs(ramp)) if left or right else 0
    if right:
        torque = signed(-torque)
    resistance = (0, 1, 0x1000, 0x8000, 0x10000, 0x20000)[(seed + 2) % 6]
    if not current:
        resistance = 0
    elif mul(scale, current) > 0:
        resistance = -resistance
    factor = 0x10000 - mul(speed, 0x8000)
    torque = mul(mul(torque, factor), grip)
    resistance = mul(mul(resistance, factor), grip)
    braking = ((not left and not right) or (left and resistance > 0) or
               (right and resistance < 0))
    if braking:
        total = signed(torque + resistance)
        mag_current = signed(-current) if current < 0 else current
        mag_total = signed(-total) if total < 0 else total
        current = 0 if mag_current <= mag_total else signed(current + total)
    elif right:
        current = max(-0x10000, min(signed(current + torque), 0))
    elif left:
        current = min(0x10000, max(signed(current + torque), 0))
    angle = target_angle(mul(scale, current), coefficient, rounding)
    old_angle = (-32768, -1024, -35, -34, -33, 0, 33, 34, 35, 1024, 32767, 1)[seed]
    step = signed(angle - old_angle, 16)
    limit = signed(mul(0x22, 0x10000 - factor) + 0x22, 16)
    next_angle = angle if abs(step) < limit else signed(old_angle + (limit if step > 0 else -limit), 16)
    return ramp, current, angle, next_angle, scale, torque, resistance


class Steering(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        self.initial_image = self.read(self.base, self.size)
        self.observed = {self.addr(a): a for a in (0x4943D0, 0x4945D0, 0x494540)}
        self.coefficient, = struct.unpack('<d', self.read(self.addr(0x511308), 8))

    def hook(self, u, address, size, data):
        if address in self.observed:
            self.trace.append(self.observed[address])
        super().hook(u, address, size, data)

    def run_steering(self, seed, current, flags, bits, rounding):
        self.u.mem_write(self.base, self.initial_image)
        self.u.mem_write(STACK, bytes(0x10000))
        rnd = random.Random(seed)
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        car = HEAP + 0x1000
        self.put(self.addr(0x59226C), '<I', car)
        self.put(self.addr(0x519C8C), '<i', (0, 0x8000, 0x10000)[seed % 3])
        self.put(car + 0x1D0, '<2B', *flags)
        self.put(car + 0xB1C, '<B', bits)
        self.put(car + 0xB16, '<h', (0, 1, 64, -64, 128, -128)[seed % 6])
        self.put(car + 0xB10, '<h', (-32768, -1024, -35, -34, -33, 0, 33, 34, 35, 1024, 32767, 1)[seed])
        self.put(car + 0x818, '<i', (-0x10001, -0x10000, -1, 0, 1, 0x10000)[seed % 6])
        self.put(car + 0x81C, '<i', current)
        self.put(car + 0x820, '<i', (0, 1, 0x1000, 0x8000, 0x10000, 0x20000)[(seed + 2) % 6])
        self.put(car + 0x824, '<i', (0, 1, 0x1000, 0x8000, 0x10000, 0x20000)[seed % 6])
        self.put(car + 0x828, '<i', (seed % 4) * 0x8000)
        self.put(car + 0x804, '<i', (0, 1, 0x4000, 0x8000, 0x10000, 0x18000)[seed % 6])
        self.put(car + 0xB8, '<i', (0, 1, 0x4000, 0x8000, 0x10000, 0x1FFFF)[seed % 6] // 2)
        self.put(car + 0x94, '<i', (0, 1, 0x4000, 0x8000, 0x10000, 0x1FFFF)[seed % 6] -
                 (0, 1, 0x4000, 0x8000, 0x10000, 0x1FFFF)[seed % 6] // 2)
        self.put(car + 0x360, '<3i', 50000, -30000, 10000)
        self.put(car + 0x408, '<3i', 12345 + seed * 1999, -18300 + seed * 771, 8191 - seed * 277)
        expected = bytearray(self.read(HEAP, 0x10000))
        ramp, cur, angle, next_angle, scale, torque, resistance = model(seed, current, flags, bits, rounding, self.coefficient)
        for offset, fmt, value in ((0x818, '<i', ramp), (0x81C, '<i', cur),
                                   (0xB12, '<h', angle), (0xB10, '<h', next_angle)):
            struct.pack_into(fmt, expected, 0x1000 + offset, value)
        self.trace = []
        self.u.reg_write(UC_X86_REG_FPCW, 0x27F | rounding)
        self.invoke(0x494110, [])
        actual = self.read(HEAP, 0x10000)
        assert actual == expected, (seed, current, flags, bits, rounding,
            'guarded car mismatch', [hex(i - 0x1000) for i, (a, b) in enumerate(zip(actual, expected)) if a != b][:12])
        for address, value in zip((0x592160, 0x592164, 0x592168), (scale, torque, resistance)):
            assert self.read(self.addr(address), 4) == struct.pack('<i', value), (seed, current, flags, bits, rounding, hex(address), 'force mismatch')
        assert self.read(self.addr(0x59226C), 4) == struct.pack('<I', car)
        assert self.trace == [0x4943D0, 0x4945D0, 0x494540]
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 4
        return actual, (scale, torque, resistance), self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Steering(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Steering(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = 0
    for case in itertools.product(range(12),
            (-196608, -65537, -65536, -65535, -1, 0, 1, 65535, 65536, 65537, 196608, -2**31, 2**31-1),
            ((0, 0), (1, 0), (0, 1), (255, 127)), range(4), (0, 0x400, 0x800, 0xC00)):
        if original.run_steering(*case) != rebuilt.run_steering(*case):
            print('FAIL automatic steering', case)
            return 1
        cases += 1
    print(f'{cases} automatic-steering cases: independent integer/rational x87 model, complete guarded car, signed/short limits and actual helpers; no mocks')
    return 0


if __name__ == '__main__':
    sys.exit(main())
