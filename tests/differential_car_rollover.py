#!/usr/bin/env python3
"""Direct rollover solver with real fixed-point, angle and matrix code.

Usage: differential_car_rollover.py entities.json [rebuilt.exe]
No controlled calls. An independent integer model checks steep-slope torque,
grip averages, slide limits, tip interpolation and complete guarded car/world
memory. Lookup tables and their adjacent atan boundary word are seeded and
checked immutable. Fixtures
avoid zero slip denominators, INT_MIN tip angles (the original atan helper
cannot negate them) and reciprocal lengths that overflow IDIV.
"""
import itertools
import json
import math
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_FPCW
from differential_menu_list import ROOT, HEAP, STACK
from differential_fixed_matrix_ops import FixedOps
from differential_body_patch import SQRT, mul, add, sub, scale, dot, cross
from differential_ground_contact import normalized
from differential_auto_steering import signed

CAR, WORLD = HEAP + 0x1000, HEAP + 0x3000
ASIN = [round(round(math.asin(i / 4095) * 65536) * (2048 / math.pi / 65536))
        for i in range(4096)]
ATAN = [round(round(math.atan(i / 511) * 65536) * (2048 / math.pi / 65536))
        for i in range(512)] + [512]
NORMALS = ((0, 65536, 0), (32768, 56740, 0), (32768, 56755, 0),
           (0, 0, 65536), (0, -65536, 0), (0, 0, 0),
           (16384, 56744, 20480), (32768, -60000, -20480))
GRIPS = (((0, 0, 0, 0), (0, 0, 0, 0)),
         ((1, 2, 3, 4), (8, 10, 12, 14)),
         ((60, 61, 62, 63), (100, 120, 130, 140)),
         ((250, 251, 252, 253), (255, 255, 255, 255)))


def div(a, b):
    n = a << 16
    assert b != 0
    q = abs(n) // abs(b)
    q = -q if (n < 0) != (b < 0) else q
    assert -0x80000000 <= q <= 0x7fffffff
    return q


def atan(y, x):
    if y == 0:
        return 0
    if x == 0:
        return 1024
    negative = (y < 0) != (x < 0)
    y, x = abs(y), abs(x)
    steep = y > x
    fraction = (min(y, x) << 16) // max(y, x)
    result = ATAN[fraction >> 7]
    if steep:
        result = 1024 - result
    return -result if negative else result


class Rollover(FixedOps):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.matrix_trace = []

    def hook(self, u, address, size, data):
        if address == self.addr(0x4B9DA0):
            out, src, matrix = self.args(3)
            assert matrix == WORLD
            assert STACK <= out < STACK + 0xFFF4 and STACK <= src < STACK + 0xFFF4
            self.matrix_trace.append(struct.unpack('<3i', self.read(src, 12)))
        super().hook(u, address, size, data)

    def solve(self, seed, normal, grip, flow, flags, poison):
        self.prepare(seed)
        self.u.mem_write(STACK, bytes([poison]) * 0x10000)
        rounding = (0x037F, 0x0F7F)[seed % 2]
        self.u.reg_write(UC_X86_REG_FPCW, rounding)
        self.put(self.addr(0x6E0EF4), '<4096H', *SQRT)
        self.put(self.addr(0x6E6EF4), '<4096h', *ASIN)
        # The exactly-unit ratio reads one element past the 512-entry atan
        # table, as the original helper does; seed that mapped adjacent word.
        self.put(self.addr(0x6E8FF4), '<513H', *ATAN)
        tables = [self.read(self.addr(a), n) for a, n in
                  ((0x6E0EF4, 8192), (0x6E6EF4, 8192), (0x6E8FF4, 1026))]
        self.put(self.addr(0x53CC18), '<I', CAR)
        timestep = (0, 1, 32768, 65536, 131072, 0x7fffffff)[seed]
        physics_scale = (0, 32768, 65536, -65536, 0x7fffffff, 8192)[seed]
        self.put(self.addr(0x519C8C), '<i', timestep)
        self.put(self.addr(0x519C90), '<i', physics_scale)
        self.put(CAR + 0x750, '<I', WORLD)
        right, up, forward = ((65536, 8192, -4096), (0, 65536, 8192),
                              (4096, 8192, 65536))
        if seed == 0:
            forward = (0, 0, 65536)
        self.put(CAR + 0x360, '<9i', *right, *up, *forward)
        ground = NORMALS[normal]
        self.put(CAR + 0x48C, '<3i', *ground)
        velocity = ((0, 0, 0), (0, 0, 12345), (123456, -234567, 1234567),
                    (-123456, 234567, -1234567), (0, 0, 0x800000))[flow]
        if seed == 0:
            boundary = mul(mul(sum(GRIPS[grip][0]) << 16, 0x4000), div(65536, 0x431168))
            velocity = (0, 0, (-boundary, boundary, boundary + 1, -boundary - 1, 0x800000)[flow])
        force = (12345 + seed * 33, -0x180000, 23456 - seed * 47)
        self.put(CAR + 0x408, '<3i', *velocity)
        self.put(CAR + 0x6A8, '<3i', *force)
        torque = (0x7fffffff, -1234567, -0x80000000)
        self.put(CAR + 0x5D0, '<3i', *torque)
        mass = (0, 1, 65536, -65536, 0x7fffffff, 24576)[seed]
        self.put(CAR + 0x760, '<i', mass)
        tip_ratio = (-0x7fffffff, -0xCCCD, -0xCCCC, 0, 0xCCCC, 0xCCCD)[seed]
        self.put(CAR + 0x77C, '<i', tip_ratio)
        timer = (0, 1, 199, 200, 201, 65535)[seed]
        self.put(CAR + 0xA9C, '<H', timer)
        tumbling = int(seed == 5)
        self.put(CAR + 0xC00, '<i', tumbling)
        self.put(CAR + 0xB28, '<B', int(flags in (1, 3)))
        self.put(CAR + 0xB34, '<B', int(flags == 2))
        self.put(CAR + 0xB42, '<b', 1 if flags == 3 else -1 if seed & 1 else 0)
        for i in range(4):
            self.put(CAR + 0x1A0 + i * 12, '<2B', GRIPS[grip][0][i], GRIPS[grip][1][i])
        rows = ((65536, 0, 0), (0, 65536, 0), (0, 0, 65536)) if seed % 2 else (
            (0, 65536, 0), (0, 0, -65536), (65536, 0, 0))
        for i, row in enumerate(rows):
            self.put(WORLD + i * 16, '<3i', *row)
        expected = bytearray(self.read(HEAP, 0x10000))
        c = CAR - HEAP

        def seti(offset, value):
            struct.pack_into('<i', expected, c + offset, value)

        def start_tumbling():
            seti(0xC00, 1)
            seti(0x96C, 65536)
            for offset, axis in zip((0x91C, 0x920, 0x924), (right, up, forward)):
                seti(offset, dot(ground, axis))

        matrix_trace = []
        if flags:
            absolute = abs(ground[1])
            index = round(absolute * 4095 / 65536) if seed % 2 == 0 else absolute * 4095 // 65536
            angle = ASIN[min(index, 4095)] * (-1 if ground[1] < 0 else 1)
            if ground[1] < 65536 and signed(1024 - angle, 16) > 0x155 and flags != 3:
                ground_force = scale(ground, 0x280000)
                a = [dot(row, ground_force) for row in rows]
                factor = mul(signed(timer << 16), mul(div(65536, 0xC80000), 0x8000))
                time_force = scale(force, factor)
                b = [dot(row, time_force) for row in rows]
                torque = add(torque, scale(cross(b, a), mass))
                struct.pack_into('<3i', expected, c + 0x5D0, *torque)
                if not tumbling:
                    start_tumbling()
                tumbling = 1
                struct.pack_into('<H', expected, c + 0xA9C, timer + int(timer < 200))
                matrix_trace = [tuple(ground_force), tuple(time_force)]
            else:
                struct.pack_into('<H', expected, c + 0xA9C, 0)
            if not tumbling:
                limit = mul(mul(sum(GRIPS[grip][0]) << 16, 0x4000), div(65536, 0x431168))
                direction, _ = normalized(sub(forward, scale(ground, dot(forward, ground))))
                slide = dot(direction, velocity)
                target = 0
                if signed(abs(slide)) > limit:
                    high = mul(mul(sum(GRIPS[grip][1]) << 16, 0x4000), div(65536, 0x431168))
                    reciprocal = div(65536, signed(high - limit)) if limit else 0
                    excess = signed(slide - limit if slide > 0 else slide + limit)
                    target = mul(excess, reciprocal)
                    if signed(abs(target)) > 65536:
                        target = 65536 if target > 0 else -65536
                diff = signed(target - tip_ratio)
                rate = mul(0x1999, timestep)
                tip_ratio = target if signed(abs(diff)) < rate else signed(
                    tip_ratio + rate if diff > 0 else tip_ratio - rate)
                seti(0x77C, tip_ratio)
                angle = atan(tip_ratio, 0x20000)
                if signed(abs(tip_ratio)) > mul(0xCCCC, 65536):
                    angle = 0
                    start_tumbling()
                    seti(0x5D0, signed(torque[0] + mul(mul(slide, physics_scale), -0xA0000)))
                struct.pack_into('<h', expected, c + 0xB18, angle)
        self.allowed_globals = set()
        self.matrix_trace = []
        self.checked_invoke(0x4373C0, [])
        actual = self.read(HEAP, 0x10000)
        assert actual == expected, ('rollover model/guards', seed, normal, grip, flow, flags, poison,
                                    [hex(i-c) for i, (a, b) in enumerate(zip(actual, expected)) if a != b][:16])
        assert self.matrix_trace == matrix_trace, 'real matrix helper inputs/order/count'
        assert self.u.reg_read(UC_X86_REG_FPCW) == rounding, 'preserved x87 control word'
        assert tables == [self.read(self.addr(a), n) for a, n in
                          ((0x6E0EF4, 8192), (0x6E6EF4, 8192), (0x6E8FF4, 1026))], 'immutable angle/sqrt tables'
        return actual


def cases():
    yield from itertools.product(range(6), range(len(NORMALS)), range(4), range(5), range(4), (0, 0xA5))


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Rollover(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Rollover(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    for count, case in enumerate(cases(), 1):
        assert original.solve(*case) == rebuilt.solve(*case), case
    mutant = Rollover(ROOT / 'cmr2bin/CMR2.exe')
    memory = bytearray(mutant.memory)
    at = 0x4374EA - mutant.base
    assert memory[at:at + 3] == bytes.fromhex('8b45f4')
    # Quarter the steep-time factor instead of halving it.
    at = 0x4374ED - mutant.base
    assert memory[at:at + 5] == bytes.fromhex('ba00800000')
    memory[at + 2] = 0x40
    mutant.memory = bytes(memory)
    try:
        mutant.solve(2, 3, 0, 0, 1, 0xA5)
    except AssertionError as error:
        assert 'rollover model/guards' in str(error), error
    else:
        raise AssertionError('wrong slope factor survived')
    print(f'{count} rollover cases: independent torque/grip/tip model, full heap guards, '
          'real angle/matrix helpers, both rounding modes and ABI; slope-factor mutation rejected')
    return 0


if __name__ == '__main__':
    sys.exit(main())
