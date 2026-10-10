#!/usr/bin/env python3
"""Menu texture loads/consumers and numeric frontend table getters.

Complete original/rebuilt bodies execute with poisoned record guards. Rendering,
text/formatting, selection and archive/loading providers are controlled. Texture
source rectangles, null resources, signed dimensions, provider order, global
store widths and stdcall are observed. The table getters have independent models.
"""
import itertools
import json
from pathlib import Path
import re
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_car_info_records import Records, put
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import signed

GRAPHICS, MENU, STATE, COMMON, STAGE = (HEAP + n for n in (0x1000, 0x2000, 0x2500, 0x2800, 0x2900))
TEXTURES = (0x831360, 0x831364, 0x831368, 0x831668, 0x83166C,
            0x831670, 0x8313AC, 0x831648, 0x8313B0, 0x831674)
TABLE, STRING, RECT = 0x516B40, 0x663B60, 0x831660


def short(v):
    return (v + 32768) % 65536 - 32768


class Options(Records):
    LEAVES = {
        0x405C10: 0, 0x405D80: 0, 0x405D90: 0, 0x406910: 0, 0x406930: 0,
        0x407150: 2, 0x4086B0: 1, 0x40ED90: 0, 0x40EDB0: 0,
        0x40B5B0: 2, 0x40B880: 6, 0x4A0380: 2, 0x4A3290: 9,
        0x4A3C60: 1, 0x4A5E40: 4, 0x4A9B80: 0, 0x4A9E60: 6,
        0x4B7560: 1, 0x4B7590: 1, 0x4FF4B0: 1, 0x4FF4C0: 1, 0x4FF4D0: 1,
        0x5004C0: 0, 0x5011B0: 0, 0x501230: 0, 0x501280: 2,
        0x501AB0: 0, 0x501D50: 4, 0x501DE0: 2, 0x501F00: 2,
        0x501F80: 9, 0x5020A0: 9, 0x5021C0: 1, 0x502500: 0,
        0x502630: 2, 0x502B10: 2, 0x502DF0: 3, 0x502FC0: 2,
        0x503930: 1, 0x503940: 2, 0x50A880: 2, 0x50CB30: 2,
        0x50EE50: 4, 0x50F620: 0, 0x50F640: 0,
    }

    def cstring(self, ptr):
        data = bytearray()
        while len(data) < 1024:
            value = self.read(ptr + len(data), 1)
            if value == b'\0':
                return bytes(data)
            data += value
        raise AssertionError(('unterminated string', hex(ptr)))

    def provider(self, address, nargs):
        args = tuple(self.args(nargs))
        value = 0
        if address == 0x405620:
            dst, fmt = self.args(2)
            pattern = self.cstring(fmt)
            specs = list(re.finditer(rb'%(?:[-+0-9.]*)([sd])', pattern))
            varargs = self.args(2 + len(specs))[2:]
            values = tuple(self.cstring(v) if m[1] == b's' else signed(v)
                           for m, v in zip(specs, varargs))
            self.trace.append(('format', pattern, values))
            out = bytearray(); last = 0
            for m, v in zip(specs, values):
                out += pattern[last:m.start()] + (v if isinstance(v, bytes) else str(v).encode())
                last = m.end()
            out += pattern[last:]
            self.u.mem_write(dst, bytes(out) + b'\0')
            value = len(out)
        elif address == 0x4A3290:
            src, dst, texture, layer, angle, centre, uv2, colour, flags = args
            self.trace.append(('sprite', self.read(src, 8), self.read(dst, 8), texture,
                layer, short(angle), self.read(centre, 12) if centre else None,
                self.read(uv2, 8) if uv2 else None, self.read(colour, 4) if colour else None, flags))
        elif address == 0x4A5E40:
            assert args[0] == GRAPHICS + 0x150
            self.trace.append(('fill', self.read(args[1], 8), self.read(args[2], 4), args[3]))
        elif address in (0x501F80, 0x5020A0):
            self.trace.append(('text', address, *args[:3], self.cstring(args[3]),
                              short(args[4]), short(args[5]), self.read(args[6], 4),
                              self.read(args[7], 4), args[8]))
        elif address == 0x40B880:
            self.trace.append(('font', args[0], self.cstring(args[1]),
                              signed(args[2]), signed(args[3]), self.read(args[4], 4), args[5]))
        elif address == 0x40B5B0:
            value = 37
            self.trace.append(('width', args[0], self.cstring(args[1])))
        elif address == 0x4A3C60:
            assert args[0] < 512
            value = HEAP + 0xA000 + args[0] * 16
            self.trace.append(('lookup', args[0]))
        elif address == 0x4A9E60:
            assert args[0] in (COMMON, STAGE) and args[2:] == (0, 0, 0, 0)
            self.trace.append(('load', args[0], self.cstring(args[1])))
            value = 0 if self.loads % 3 == self.missing else HEAP + 0x3000 + self.loads * 0x140
            self.loads += 1
        elif address in (0x40ED90, 0x40EDB0):
            value = HEAP + 0x9800 if address == 0x40ED90 else HEAP + 0x9900
        elif address in (0x50F620, 0x50F640):
            value = COMMON if address == 0x50F620 else STAGE
        elif address in (0x501D50, 0x501F00, 0x501DE0, 0x50CB30, 0x50EE50):
            count = 12 if address == 0x50EE50 else 4
            self.trace.append(('bar', address, args[0] if address not in (0x50CB30, 0x50EE50) else 0,
                               self.read(args[0] if address in (0x50CB30, 0x50EE50) else args[1], 8),
                               self.read(args[3], count) if address == 0x50EE50 else None))
        elif address == 0x4A0380:
            assert args[0] == MENU
            value = args[1]
            self.trace.append(('menu', args[1]))
        elif address == 0x502500:
            value = MENU
        elif address == 0x501AB0:
            value = STATE
        elif address == 0x405C10:
            value = self.width
        elif address in (0x4B7560, 0x4B7590):
            value = self.support
        elif address == 0x5004C0:
            value = self.pulse
        elif address == 0x5021C0:
            value = self.transition
        elif address in (0x4FF4B0, 0x4FF4C0, 0x4FF4D0):
            value = args[0] % 12
        elif address in (0x501280, 0x502630, 0x502B10, 0x502FC0, 0x503940):
            value = self.choice
        elif address == 0x502DF0:
            value = self.percentage
        elif address == 0x503930:
            value = 1
        elif address in (0x406910, 0x406930):
            value = self.country if address == 0x406910 else self.stage
        elif address == 0x4086B0:
            value = 3
        elif address == 0x407150:
            value = 5
        elif address == 0x4A9B80:
            value = self.frame
        elif address == 0x405D80:
            value = self.mode
        elif address in (0x405D90, 0x5011B0):
            value = 0
        elif address == 0x501230:
            value = 4
        elif address == 0x50A880:
            self.trace.append(('select', *args))
        else:
            raise AssertionError(hex(address))
        self.u.reg_write(UC_X86_REG_EAX, value & 0xFFFFFFFF)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def fixture(self, seed, width=640, arrows=1, percentage=34, choice=0, pulse=0):
        h = self.begin(seed)
        self.width, self.support, self.pulse = width, seed % 2, pulse
        self.transition, self.percentage, self.choice = seed % 3, percentage, choice
        self.country, self.stage, self.frame, self.mode = seed % 8, seed % 11, seed % 20, seed % 3
        self.loads, self.missing = 0, -1
        self.intercept(*self.LEAVES.items())
        self.callbacks[self.addr(0x405620)] = (0, lambda: self.provider(0x405620, 0))
        self.put(self.addr(0x520B74), '<I', GRAPHICS)
        put(h, GRAPHICS, '<2I', width, width * 3 // 4)
        put(h, MENU + 6, '<2b', 4, 1)
        for i in range(4):
            put(h, MENU + 0x1F + 20 * i, '<B', i)
        put(h, STATE, '<i', 0x22334455)
        h[0x9800:0x980A] = b'FrontDir\0\0'
        h[0x9900:0x990A] = b'SetupDir\0\0'
        for i in range(512):
            value = ('T%03d' % i).encode() + b'\0'
            h[0xA000 + i * 16:0xA000 + i * 16 + len(value)] = value
        for i, address in enumerate(TEXTURES):
            texture = HEAP + 0x3000 + i * 0x140
            optional = address in (0x83166C, 0x831670, 0x831668, 0x831674)
            self.put(self.addr(address), '<I', texture if arrows or not optional else 0)
            put(h, texture + 0x11C, '<4h', i * 3 - 7, 5 - i,
                -(17 + i * 2) if seed % 5 == 0 else 17 + i * 2, 31 - i)
            self.allowed.add((self.addr(address), 4))
        for i in range(12):
            texture = HEAP + 0x5000 + i * 0x140
            put(h, texture + 0x11C, '<4h', i, -i, 33+i, 21+i)
            self.put(self.addr(0x83137C) + i * 4, '<I', texture if seed % 3 else 0)
            self.allowed.add((self.addr(0x83137C) + i * 4, 4))
        for address, size in ((RECT, 8), (0x82AF70, 8), (0x82ACE8, 8)):
            self.allowed.update((self.addr(address) + i, 2) for i in range(0, size, 2))
        return h

    def outcome(self, address, args):
        h = self.invoke_guarded(address, args)
        return (h, self.read(self.addr(STRING), 260), self.read(self.addr(RECT), 8),
                self.read(self.addr(0x82AF70), 8), self.read(self.addr(0x82ACE8), 8), tuple(self.trace))

    def rows(self, count, cursor, kind, width, arrows, percentage, choice, pulse, seed):
        self.fixture(seed, width, arrows, percentage, choice, pulse)
        return self.outcome(0x50B1C0, [count, cursor, kind, 240, 170, seed % 2])

    def consumers(self, address, argument, width, arrows, pulse, seed):
        h = self.fixture(seed, width, arrows, 34, seed % 2, pulse)
        if address == 0x50E780:
            put(h, MENU + 7, '<b', argument)
            args = [MENU]
        elif address == 0x500550:
            args = [argument]
        elif address == 0x50BFD0:
            args = [argument]
            # Keep the nested rows real; its opaque leaves are already hooked.
        else:
            args = [argument]
        return self.outcome(address, args)

    def loading(self, country, missing, seed):
        self.fixture(seed)
        self.country, self.missing = country, missing
        result = self.outcome(0x50A080, [])
        assert self.loads == 22
        loads = [t for t in self.trace if t[0] == 'load']
        assert len(loads) == 22 and [t[1] for t in loads] == [STAGE if i == 3 else COMMON for i in range(22)]
        values = tuple(struct.unpack('<I', self.read(self.addr(address), 4))[0] for address in TEXTURES)
        values += struct.unpack('<12I', self.read(self.addr(0x83137C), 48))
        assert values == tuple(0 if i % 3 == missing else HEAP + 0x3000 + i * 0x140 for i in range(22))
        return result, values

    def getter(self, address, index, seed):
        h = self.fixture(seed)
        values = tuple(signed(seed * 0x10000000 + i * 0x1010101) for i in range(28))
        offset = 0 if address in (0x40EE90, 0x40EEA0) else 0xC8
        self.put(self.addr(TABLE) + offset, '<28i', *values)
        self.invoke_guarded(address, [index])
        bias = 22 if address == 0x40EEA0 else (14 if address == 0x40EE80 else 0)
        expected = values[index + bias] & 0xFFFFFFFF
        assert self.read(HEAP, 0x10000) == bytes(h)
        assert self.u.reg_read(UC_X86_REG_EAX) == expected and not self.trace
        return expected


def compare(a, b, method, cases):
    count = 0
    for case in cases:
        left, right = getattr(a, method)(*case), getattr(b, method)(*case)
        assert left == right, (method, case, 'heap/globals/trace mismatch', left[-1], right[-1])
        count += 1
    return count


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    a = Options(ROOT / 'cmr2bin/CMR2.exe')
    b = Options(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = compare(a, b, 'getter', ((address, index, seed)
             for address, limit in ((0x40EE70, 28), (0x40EE80, 14), (0x40EE90, 28), (0x40EEA0, 6))
             for index, seed in itertools.product(range(limit), (0, 1, 7, 15))))
    count += compare(a, b, 'loading', itertools.product(range(8), (-1, 0, 1, 2), (1, 5)))
    count += compare(a, b, 'rows', itertools.product((0, 1, 4), (-1, 0, 3), range(5),
             (640, 1024), (0, 1), (-1, 0, 1, 33, 34, 66, 67), (0, 1), (0, 1), (7, 10)))
    count += compare(a, b, 'consumers', itertools.product((0x50E780, 0x50A920, 0x50BFD0, 0x500550),
             (0, 1), (640, 1024), (0, 1), (0, 1), (7, 9, 10)))
    print(count, 'option records: numeric table models, texture loads, complete consumers, guarded heap/global writes, provider traces and ABI')
    return 0


if __name__ == '__main__':
    sys.exit(main())
