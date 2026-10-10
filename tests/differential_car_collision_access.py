#!/usr/bin/env python3
"""Full car-contact bodies: guarded records, native-field views and provider ABI.

Geometry, clamps and fixed-point helpers execute unchanged. Only overlap/sphere
classification, damage, recursive frame updates, cheats and debris are leaves.
Static contacts also have an independent integer model for identity transforms.
Every case compares the entire poisoned heap, selected globals and leaf traces.
"""
import itertools
import json
import math
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_car_info_records import Records, put
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import mul, signed

A, B = HEAP + 0x1000, HEAP + 0x2000
MATRIX, BOX_A, BOX_B = HEAP + 0x4000, HEAP + 0x5000, HEAP + 0x5100
ENTRY, OWNER = HEAP + 0x6000, HEAP + 0x7000
CENTRE_A, CENTRE_B = HEAP + 0x8000, HEAP + 0x8100
VERTICES_A, VERTICES_B = HEAP + 0x9000, HEAP + 0x9100
GLOBALS = ((0x5915E8, 12), (0x5915DC, 4), (0x591468, 4))


class Contacts(Records):
    def provider(self, address, nargs):
        args = tuple(self.args(nargs))
        value = 0
        if address == 0x489B20:
            assert args[0] == BOX_A and args[3] == 0x8000
            self.put(args[1], '<i', self.factor)
            self.put(args[2], '<i', self.factor // 3)
            self.trace.append(('sphere', args[0], args[3]))
            value = self.side
        elif address == 0x488640:
            assert args[:2] == (BOX_A, BOX_B) and args[3] == 0x8000
            self.trace.append(('overlap', self.read(args[2], 12)))
            value = self.overlap
        elif address == 0x48C870:
            other = (args[1] & 255)
            if other >= 128:
                other -= 256
            self.trace.append(('frame', args[0] & 255, other,
                               self.read(args[2], 12), args[3]))
        elif address == 0x466EF0:
            self.trace.append(('deform', args[0], self.read(args[1], 12),
                               self.read(args[2], 12), signed(args[3]),
                               args[4] & 255, args[5]))
        elif address == 0x499750:
            self.trace.append(('debris', args[0], self.read(args[1], 12),
                               args[2], self.read(args[3], 36), *args[4:]))
        elif address == 0x4063F0:
            assert args == (4,)
            self.trace.append(('cheat', 4))
            value = self.cheat
        else:
            raise AssertionError(hex(address))
        self.u.reg_write(UC_X86_REG_EAX, value)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def fixture(self, seed, counts=(1, 1), nulls=0):
        h = self.begin(seed)
        self.intercept((0x489B20, 4), (0x488640, 4), (0x48C870, 4),
                       (0x466EF0, 6), (0x499750, 6), (0x4063F0, 1))
        self.side, self.overlap, self.factor, self.cheat = 1, 1, 0x8000, 0
        self.put(self.addr(0x6E0EF4), '<4096H',
                 *(math.isqrt((8 + 16 * i) * 65536) for i in range(4096)))
        put(h, MATRIX, '<16i', 65536, 0, 0, 0, 0, 65536, 0, 0,
            0, 0, 65536, 0, 0, 0, 0, 65536)
        for p, index in ((A, 254), (B, 129)):
            put(h, p + 0xB1A, '<B', index)
            put(h, p + 0x750, '<I', MATRIX)
            put(h, p + 0x75C, '<i', 65536)
            for offset in (0x408, 0x420, 0x5C4, 0x5D0, 0x5DC):
                put(h, p + offset, '<3i', 0, 0, 0)
            put(h, p + 0x204, '<3i', 65536, 65536, 2 * 65536)
            put(h, p + 0x360, '<9i', 65536, 0, 0, 0, 65536, 0, 0, 0, 65536)
            put(h, p + 0xB35, '<B', 1)
            put(h, p + 0xB3E, '<2b', 0, 0)
            for offset in (0xB64, 0xB70, 0xC00):
                put(h, p + offset, '<i', 0)
            put(h, p + 0x2D0, '<3i', 0 if p == A else 2 * 65536, 0, 0)
            put(h, p + 0x2E8, '<3i', seed * 31, -seed * 17, seed * 43)
            for i in range(8):
                put(h, p + 0x270 + 12 * i, '<3i',
                    (i + (0 if p == A else 2)) * 65536, i * 1024, (i - 3) * 65536)
        for box, centre, vertices in ((BOX_A, CENTRE_A, VERTICES_A),
                                       (BOX_B, CENTRE_B, VERTICES_B)):
            put(h, box, '<2i', 65536, 2 * 65536)
            put(h, box + 0x10, '<6i', 65536, 0, 0, 0, 0, 65536)
            put(h, centre, '<3i', 65536 if box == BOX_A else -65536, 0, 0)
            put(h, box + 0x90, '<2I', 0 if nulls & 1 else vertices,
                0 if nulls & 2 else centre)
            for i in range(8):
                put(h, box + 0x30 + 12 * i, '<3i', i * 65536, i * 123, -i * 32768)
                put(h, vertices + 12 * i, '<3i', i * 77, i * 111, i * 333)
        put(h, ENTRY, '<2I', OWNER, BOX_A)
        self.put(self.addr(0x5915E0), '<I', BOX_A)
        self.put(self.addr(0x591394), '<I', BOX_B)
        for address, count in zip((0x5915F4, 0x5914D4), counts):
            self.put(self.addr(address), '<b', count)
        for address in (0x5914C4, 0x590ECC):
            self.put(self.addr(address), '<4b', 0, 1, 2, 3)
        for address in (0x590EC8, 0x5914A4):
            self.put(self.addr(address), '<4B', seed % 2, 1, 0, 1)
        for address, values in ((0x5915E8, (0, 0, 65536)),
                                (0x5914A8, (12345, -23456, 34567)),
                                (0x5914B8, (-67890, 11111, 22222)),
                                (0x591498, (-65536, 0, -65536))):
            self.put(self.addr(address), '<3i', *values)
        for address, value in ((0x5915DC, 0x8000), (0x591468, 0x1578D),
                               (0x591490, 0x4000), (0x519C90, 65536), (0x5914D8, 0)):
            self.put(self.addr(address), '<i', value)
        for i in range(8):
            self.put(self.member(0x591628, 0x5915F8) + i * 12,
                     '<3i', i * 32768, 0, -i * 65536)
        for address, size in GLOBALS:
            self.allowed.update((self.addr(address) + i, 4) for i in range(0, size, 4))
        return h

    def outcome(self, address, args):
        memory = self.invoke_guarded(address, args)
        return (memory, tuple(self.read(self.addr(a), n) for a, n in GLOBALS),
                tuple(self.trace), self.u.reg_read(UC_X86_REG_EAX))

    def box(self, side, factor, nulls, seed):
        h = self.fixture(seed, nulls=nulls)
        self.side, self.factor = side, factor
        result = self.outcome(0x489750, [A, BOX_A, 0x8000])
        assert result[3] == int(side != 0)
        if side == 0:
            assert result[0] == bytes(h) and len(result[2]) == 1
        else:
            assert result[1][1:] == (struct.pack('<i', 0x8000), struct.pack('<i', 0x1578D))
            assert result[2][-1][:3] == ('frame', 254, -1)
        return result

    def separate(self, overlap, counts, nulls, seed):
        h = self.fixture(seed, counts, nulls)
        self.overlap = overlap
        result = self.outcome(0x48A5F0, [A, B])
        assert result[3] == overlap
        if not overlap:
            assert result[0] == bytes(h) and len(result[2]) == 1
        frames = [call for call in result[2] if call[0] == 'frame']
        assert len(frames) == (2 if overlap and 0 in counts else 0)
        if frames:
            assert frames[0][1:3] == (254, -127) and frames[1][1:3] == (129, -2)
        return result

    def cars(self, normal, speed, upper, ground, counts, cheat, seed):
        h = self.fixture(seed, counts)
        self.cheat = cheat
        self.put(self.addr(0x5915E8), '<3i', *normal)
        for p in (A, B):
            put(h, p + 0xC00, '<i', upper)
            put(h, p + 0xB35, '<B', ground)
            put(h, p + 0x5DC, '<3i', 3 * 65536, -16384, -4 * 65536)
        put(h, A + 0x408, '<3i', speed, speed // 3, 2 * speed)
        put(h, B + 0x408, '<3i', -speed // 2, 0, speed // 4)
        put(h, B + 0x75C, '<i', 2 * 65536)
        result = self.outcome(0x48AE90, [A, B])
        assert len([call for call in result[2] if call[0] == 'deform']) == 2
        for p in (A, B):
            x, _, z = struct.unpack_from('<3i', result[0], p-HEAP+0x5DC)
            assert abs(x) <= 65536 and abs(z) <= 2 * 65536
        return result[:3]  # void caller: EAX is not part of its ABI contract.

    def static(self, speed, damp, wall, counts, known, current, cheat, scale, seed):
        h = self.fixture(seed, counts)
        self.cheat = cheat
        put(h, OWNER + 0x10, '<I', damp)
        put(h, A + 0x408, '<3i', 0, 0, speed)
        put(h, A + 0x5DC, '<3i', 2 * 65536, -54321, -3 * 65536)
        put(h, A + 0xB3E, '<2b', int(known), current)
        put(h, A + 0xAD6, '<h', 9)
        self.put(self.addr(0x5914D8), '<i', wall)
        self.put(self.addr(0x519C90), '<i', scale)
        expected = bytearray(h)
        flag_a, flag_b = abs(speed) > 0x8000, abs(speed) > 0x1578D
        flag_c = bool(damp & 0x2001000 and speed)
        if flag_c:
            flag_b = False
        impulse = mul(mul(-speed, 0x28F) if flag_c else -speed, scale)
        put(expected, A + 0x5DC, '<3i', 65536, 0, -2 * 65536)
        put(expected, A + 0x5C4, '<3i', 0, 0, 0 if cheat else impulse)
        if flag_a and not flag_c:
            for offset, value in ((0x96C, 65536), (0xC00, 1), (0xC04, 1)):
                put(expected, A + offset, '<i', value)
        weight = 0x40000 if flag_a else (0x18000 if wall else 0x30000)
        spin_x = mul(impulse, 4 * 65536 if flag_b else 65536) if flag_a else 0
        spin_x, spin_y = mul(spin_x, weight), mul(impulse, weight)
        limits = (mul(0x80000, scale), mul(0x40000, scale))
        put(expected, A + 0x5D0, '<3i', max(-limits[0], min(limits[0], spin_x)),
            max(-limits[1], min(limits[1], spin_y)), 0)
        put(expected, A + 0x408, '<3i', 0, 0x4000 if flag_b else 0,
            signed(speed + 2 * impulse) if cheat else speed)
        if current < 5:
            put(expected, A + 0xAE0 + current * 2, '<h', 9)
            put(expected, A + 0xB3F, '<b', current + 1)
        result = self.outcome(0x48BE20, [A, ENTRY, 0, 9])
        assert result[0] == bytes(expected), ('independent static model', speed, damp, wall, counts, known, current, cheat, scale,
                    [(hex(i-0x1000), a, b) for i, (a, b) in enumerate(zip(result[0], expected)) if a != b][:16])
        assert result[3] == flag_c
        deformation = [call for call in result[2] if call[0] == 'deform']
        kind = (0 if counts[0] else 2) if wall else 1
        assert len(deformation) == int(not known and (not wall or any(counts)))
        if deformation:
            assert deformation[0][4:6] == (0x4000 if not wall else 0, kind)
        return result


def compare(original, rebuilt, method, cases):
    count = 0
    for case in cases:
        left, right = getattr(original, method)(*case), getattr(rebuilt, method)(*case)
        if left != right:
            raise AssertionError((method, case, 'memory/globals/trace/return mismatch'))
        count += 1
    return count


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Contacts(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Contacts(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = compare(original, rebuilt, 'box',
                    ((side, factor, nulls, seed)
                     for side, factor, nulls, seed in itertools.product(range(4),
                         (-0x8000, 0, 0x8000), range(4), range(3))
                     if side != 3 or not nulls & 2))
    count += compare(original, rebuilt, 'separate',
                      ((overlap, counts, nulls, seed)
                       for overlap, counts, nulls, seed in itertools.product((0, 1),
                         ((0, 1), (1, 0), (1, 1), (4, 4)), range(4), range(3))
                       if not overlap or not nulls & 2))
    count += compare(original, rebuilt, 'static', itertools.product(
                      (-0x20000, -0x8000, 0, 0x8000, 0x8001, 0x1578D, 0x1578E),
                      (0, 0x1000, 0x2000000, 0x20), (0, 1),
                      ((0, 0), (1, 0), (0, 1)), (0, 1), (0, 4, 5), (0, 1),
                      (0x8000, 0x10000), (7,)))
    count += compare(original, rebuilt, 'cars', itertools.product(
                      ((65536, 0, 0), (0, 65536, 0), (0, 0, 65536)),
                      (-0x20000, 0, 0x10000), (0, 1), (0, 1),
                      ((1, 0), (0, 1), (1, 1)), (0, 1), (11,)))
    print(count, 'car contacts: complete heap/global/trace comparisons, independent static model, signed indices, pointer guards and stdcall')
    return 0


if __name__ == '__main__':
    sys.exit(main())
