#!/usr/bin/env python3
"""Direct ground-contact branches with real fixed-point and matrix helpers.

Usage: differential_ground_contact.py entities.json [rebuilt.exe]
Terrain heights, new ground normal, airborne flags, free-corner lift and wheel
travel are controlled leaves. Real corner geometry, axis-angle matrix creation,
rotation and normalization execute. Independent integer geometry checks the
whole guarded heap, torque, basis, damping, sink depth, globals and ABI.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_menu_list import ROOT, HEAP, STACK
from differential_fixed_matrix_ops import FixedOps
from differential_body_patch import SQRT, mul, add, sub, scale, dot, cross
from differential_auto_steering import signed
from differential_basis_rotate import SINES
from differential_car_corners import SIGNS

CAR, WORLD = HEAP + 0x1000, HEAP + 0x3000
ROT_GLOBALS = (0x67F250, 0x67F248, 0x68336C, 0x67F254, 0x67F258,
               0x67F25C, 0x67F238, 0x67F244, 0x67F24C)
NORMALS = ((0, 65536, 0), (4096, 65536, -8192), (0, 45000, 46000),
           (0, -65536, 0), (1, 65536, 0), (0, 0, 0))


def normalized(v):
    squared = sum(mul(x, x) for x in v) & 0xFFFFFFFF
    if not squared:
        return [0, 0, 0], 0
    exponent = (squared.bit_length() - 1 - 15 + 1) // 2 * 2
    shift = exponent + 4
    index = squared >> shift if shift >= 0 else squared << -shift
    root = SQRT[index]
    length = root << (exponent // 2) if exponent >= 0 else root >> (-exponent // 2)
    assert length > 2, 'fixture reciprocal must fit signed 32 bits'
    return scale(v, (1 << 32) // length), length


def axis_angle(axis, angle):
    sine, cosine = SINES[-angle & 4095], SINES[(-angle + 1024) & 4095]
    omc = 65536 - cosine
    xx, yy, zz = (mul(x, x) for x in axis)
    xy = mul(mul(axis[0], axis[1]), omc)
    xz = mul(mul(axis[0], axis[2]), omc)
    yz = mul(mul(axis[2], axis[1]), omc)
    xs, ys, zs = (mul(x, sine) for x in axis)
    rows = [[signed(mul(cosine, 65536 - xx) + xx), signed(xy - zs), signed(ys + xz)],
            [signed(zs + xy), signed(mul(cosine, 65536 - yy) + yy), signed(yz - xs)],
            [signed(xz - ys), signed(xs + yz), signed(mul(cosine, 65536 - zz) + zz)]]
    return rows, (sine, cosine, omc, xx, yy, zz, xy, xz, yz)


def rotate(v, rows):
    return [signed(sum(mul(rows[k][j], v[k]) for k in range(3))) for j in range(3)]


class GroundContact(FixedOps):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {self.addr(a): (4 * n, lambda a=a, n=n: self.provider(a, n))
                          for a, n in ((0x4930E0, 2), (0x42DE20, 0), (0x42F820, 0),
                                       (0x42F8C0, 0), (0x42E450, 0))}

    def provider(self, address, count):
        args = self.args(count)
        if address == 0x4930E0:
            assert args[0] == CAR and args[1] in (4, 8)
            self.trace.append(('terrain', args[1]))
            self.put(CAR + 0x8DC, '<8i', *self.heights)
        elif address == 0x42DE20:
            self.trace.append(('normal',))
            self.put(CAR + 0x48C, '<3i', *self.normal)
        elif address == 0x42F820:
            self.trace.append(('airborne',))
            if self.air == 2 and self.flag_calls == 0:
                self.put(CAR + 0xB35, '<B', 0)
            elif self.flag_calls or self.air == 0:
                self.put(CAR + 0xB35, '<B', self.seed % 2)
            self.flag_calls += 1
        else:
            self.trace.append(('lift' if address == 0x42F8C0 else 'travel',))
        for register, value in ((UC_X86_REG_EAX, 0x12345678),
                                (UC_X86_REG_ECX, 0xA5A5A5A5), (UC_X86_REG_EDX, 0x5A5A5A5A)):
            self.u.reg_write(register, value)

    def hook(self, u, address, size, data):
        if address == self.addr(0x43EEF0):
            assert self.args(1) == (CAR,)
            self.trace.append(('corners',))
        elif address == self.addr(0x4ADB10):
            _, axis, angle = self.args(3)
            # The caller pushes a word angle with an unspecified high word;
            # the real matrix helper consumes only the low twelve bits.
            assert axis == CAR + 0x360 and signed(angle, 16) == self.tip
            self.trace.append(('tip', self.read(axis, 12), signed(angle, 16)))
        super().hook(u, address, size, data)

    def run_contact(self, seed, normal, flow, air, tip, poison):
        case = seed, normal, flow, air, tip, poison
        self.prepare(seed)
        self.trace = []
        self.u.mem_write(STACK, bytes([poison]) * 0x10000)
        self.put(self.addr(0x6E0EF4), '<4096H', *SQRT)
        self.put(self.addr(0x6E2EF4), '<4096i', *SINES)
        self.seed, self.air, self.normal, self.tip = seed, air, NORMALS[normal], tip
        self.flag_calls = 0
        self.heights = [-0x18000 + seed * 3333 + i * 2345 for i in range(8)]
        self.put(self.addr(0x53CC18), '<I', CAR)
        timestep = (0, 0x8000, 0x10000, 0x7FFFFFFF)[seed % 4]
        self.put(self.addr(0x519C8C), '<i', timestep)
        self.put(CAR + 0x750, '<I', WORLD)
        self.put(CAR + 0x204, '<3i', 0x10000, 0x8000, 0x18000)
        self.put(CAR + 0xB64, '<i', 1)  # suspend extra upper-corner offset pass
        self.put(CAR + 0x48C, '<3i', 0, 65536, 0)
        up = ((0, 65536, 0), (0, 0xB333, 0), (0, 0xB334, 0), (0, 0xFD6F, 0),
              (0, 0xFD70, 0), (0, 65536, 0), (0, 65536, 0), (0, 0, 65536))[seed]
        right = up if seed == 5 else (65536, 8192 if seed % 3 else 0, -4096)
        self.put(CAR + 0x360, '<9i', *right, *up, 0, 0, 65536)
        self.put(CAR + 0x384, '<9i', 0, 0, -65536, 0, 65536, 0, 65536, 0, 0)
        self.put(CAR + 0x778, '<i', (-0x80000000, -0x10000, 0, 1, 0x8000,
                                   0x1570A, 0x1570B, 0x7FFFFFFF)[seed])
        velocity = scale(self.normal, (0, -0x80000, 0x80000)[flow])
        self.put(CAR + 0x408, '<3i', *velocity)
        self.put(CAR + 0xB35, '<B', 0 if air == 0 else 2 if air == 3 else 1)
        self.put(CAR + 0xB60, '<i', (seed >> 1) & 1)
        self.put(CAR + 0xB2A, '<B', seed & 1)
        self.put(CAR + 0xC00, '<i', (seed >> 2) & 1)
        self.put(CAR + 0xB18, '<h', tip)
        self.put(CAR + 0xB2C, '<8B', *(int((seed + i) % 3 == 0) for i in range(8)))
        self.put(WORLD, '<16i', 65536, 0, 0, 0, 0, 65536, 0, 0,
                 0, 0, 65536, 0, 333, 777, -111, 65536)
        expected = bytearray(self.read(HEAP, 0x10000))
        globals_before = [self.read(self.addr(a), 4) for a in ROT_GLOBALS]
        trace = [('terrain', 4)]
        c = CAR - HEAP

        def get(offset):
            return list(struct.unpack_from('<3i', expected, c + offset))

        def setv(offset, v):
            struct.pack_into('<3i', expected, c + offset, *v)

        def seti(offset, value):
            struct.pack_into('<i', expected, c + offset, value)

        def matrix():
            return [list(struct.unpack_from('<3i', expected, WORLD - HEAP + i * 16))
                    for i in range(3)]

        def publish():
            for i in range(3):
                struct.pack_into('<3i', expected, WORLD - HEAP + i * 16, *get(0x360 + i * 12))
            struct.pack_into('<3i', expected, WORLD - HEAP + 48, *get(0x2D0))

        expected[c + 0x498:c + 0x4A4] = expected[c + 0x48C:c + 0x498]
        expected[c + 0x504:c + 0x564] = expected[c + 0x4A4:c + 0x504]
        expected[c + 0xBA0:c + 0xBAC] = bytes(12)
        struct.pack_into('<8i', expected, c + 0x8DC, *self.heights)
        if air in (1, 2):
            trace.append(('airborne',))
            if air == 2:
                expected[c + 0xB35] = 0
        got_normal = 0
        if air in (0, 2):
            old_normal = get(0x48C)
            setv(0x48C, self.normal)
            trace.append(('normal',))
            ground_speed = dot(self.normal, get(0x408))
            speed = struct.unpack_from('<i', expected, c + 0x778)[0]
            fraction = min(mul(min(speed, 0x1570A), 0x8000), 65536)
            if ground_speed <= mul(signed(65536 - fraction), 0x1999):
                alignment = dot(get(0x36C), self.normal)
                if alignment < 0xB334:
                    seti(0xC00, 1)
                    seti(0x96C, 65536)
                    seti(0xC04, 1)
                elif not ((seed >> 1) & 1) or alignment < 0xFD70:
                    if seed & 1:
                        setv(0x36C, self.normal)
                    else:
                        setv(0x36C, normalized(add(scale(sub(self.normal, get(0x36C)), 0x8000),
                                                   get(0x36C)))[0])
                    got_normal = 1
                    up, right = get(0x36C), get(0x360)
                    setv(0x360, normalized(sub(right, scale(up, dot(up, right))))[0])
                    setv(0x378, normalized(cross(get(0x360), up))[0])
                else:
                    expected[c + 0x360:c + 0x384] = expected[c + 0x384:c + 0x3A8]
                for i in range(3):
                    seti(0x91C + i * 4, dot(get(0x360 + i * 12), self.normal))
                publish()
                rows, position, half = matrix(), get(0x2D0), get(0x204)
                for i, signs in enumerate(SIGNS):
                    setv(0x270 + i * 12, [signed(position[j] + sum(signs[k] * mul(rows[k][j], half[k])
                                               for k in range(3))) for j in range(3)])
                trace.append(('corners',))
                contact_count = 8 if struct.unpack_from('<i', expected, c + 0xC00)[0] else 4
                trace.append(('terrain', contact_count))
            else:
                delta, length = normalized(sub(self.normal, old_normal))
                if length > 0:
                    local = [dot(row, delta) for row in matrix()]
                    torque = [mul(signed(-local[2]), 0x14CCC), 0, mul(local[0], 0xE666)]
                    setv(0x5D0, add(get(0x5D0), torque))
                setv(0x48C, old_normal)
            trace.extend((('lift',), ('airborne',)))
            expected[c + 0xB35] = seed % 2
            if seed % 2 == 0:
                if ground_speed < 0:
                    setv(0x408, sub(get(0x408), scale(get(0x48C), ground_speed)))
                damping = mul(0x1EB8, timestep)
                omega = get(0x420)
                setv(0x420, [mul(omega[0], damping), omega[1], mul(omega[2], damping)])
        trace.append(('travel',))
        expected[c + 0xB2A] = got_normal
        publish()
        scratch = globals_before
        if tip:
            axis = get(0x360)
            trace.append(('tip', struct.pack('<3i', *axis), tip))
            rows, values = axis_angle(axis, tip)
            scratch = [struct.pack('<i', v) for v in values]
            for i in (1, 2):
                struct.pack_into('<3i', expected, WORLD - HEAP + i * 16,
                                 *normalized(rotate(get(0x360 + i * 12), rows))[0])
        depth = 0x1999
        for i in range(4):
            delta = signed(get(0x270 + i * 12)[1] - self.heights[i])
            if not expected[c + 0xB2C + i] and delta < depth:
                depth = max(delta, 0)
        seti(0x958, -depth)
        self.allowed_globals = {(self.addr(0x53CC18), 4)} | {(self.addr(a), 4) for a in ROT_GLOBALS}
        self.checked_invoke(0x42CE60, [])
        assert self.read(HEAP, 0x10000) == expected, (case, 'ground-contact model/heap guards',
               [hex(i - c) for i, (a, b) in enumerate(zip(self.read(HEAP, 0x10000), expected)) if a != b][:20])
        assert self.trace == trace, (case, 'provider/helper order', self.trace, trace)
        assert [self.read(self.addr(a), 4) for a in ROT_GLOBALS] == scratch, (case, 'rotation scratch')
        assert self.read(self.addr(0x53CC18), 4) == struct.pack('<I', CAR)
        assert self.read(self.addr(0x6E0EF4), 8192) == struct.pack('<4096H', *SQRT)
        assert self.read(self.addr(0x6E2EF4), 16384) == struct.pack('<4096i', *SINES)
        return self.read(HEAP, 0x10000), self.trace, scratch


def cases():
    yield from itertools.product(range(8), range(6), range(3), range(4),
                                 (0, 1, 0x400, -1), (0, 0xA5))


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = GroundContact(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = GroundContact(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    for count, case in enumerate(cases(), 1):
        assert original.run_contact(*case) == rebuilt.run_contact(*case), case
    mutant = GroundContact(ROOT / 'cmr2bin/CMR2.exe')
    memory = bytearray(mutant.memory)
    at = 0x42D8CB - mutant.base
    assert memory[at:at + 5] == bytes.fromhex('bacc4c0100')
    memory[at + 2] = 0xCC  # incorrect x-axis torque multiplier
    mutant.memory = bytes(memory)
    try:
        mutant.run_contact(2, 2, 2, 0, 0, 0xA5)
    except AssertionError as error:
        assert 'ground-contact model' in str(error), error
    else:
        raise AssertionError('wrong torque multiplier survived')
    print(f'{count} ground-contact cases: independent torque/basis/damping/corners/depth model; '
          'guarded heap, actual matrix helpers, provider order and ABI; torque mutation rejected')
    return 0


if __name__ == '__main__':
    sys.exit(main())
