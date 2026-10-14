#!/usr/bin/env python3
"""Execute .rcs writer/restorer against the original and an independent disk model.

Usage: differential_saved_game.py entities.json [rebuilt.exe]
File/profile providers are controlled leaves; the complete session bodies,
including the rebuilt validator, execute under Unicorn. Malformed sessions
exercise the rebuilt rejection path, which intentionally differs from CMR2.
"""
import itertools
import json
from pathlib import Path
import struct
import sys

import pefile
from unicorn.x86_const import UC_X86_REG_EAX
from differential_menu_list import ROOT, HEAP, STOP
from differential_stage_lighting import Lighting


class Session(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.image = self.addr(0x81a858)
        self.gameinfo = self.addr(0x52afa0)
        for address, nargs in [
            (0x408d80, 0), (0x405d70, 0), (0x405d80, 0),
            (0x405d90, 0), (0x405da0, 0), (0x4086f0, 1),
            (0x408d60, 1), (0x408860, 1), (0x407610, 1),
            (0x407630, 1), (0x40cfe0, 1), (0x407520, 1),
            (0x4075e0, 1), (0x4075d0, 1), (0x4075b0, 1),
            (0x4075c0, 1), (0x406910, 0), (0x406930, 0),
            (0x406920, 0), (0x4aa700, 0), (0x4f5150, 1),
            (0x4aa5a0, 4), (0x4f4db0, 0), (0x4eadb0, 0),
            (0x4eb860, 2), (0x4ebf20, 1), (0x4eb000, 2),
            (0x4eb4b0, 2), (0x4eb0c0, 2), (0x409010, 2),
            (0x40cff0, 2), (0x4068b0, 1), (0x4068e0, 1),
            (0x4068d0, 1), (0x4eb160, 0), (0x40e330, 1),
            (0x4aade0, 1),
        ]:
            self.callbacks[self.addr(address)] = (4 * nargs,
                lambda a=address, n=nargs: self.provider(a, n))
        # Import calls retain the executable's actual thunk but land in a
        # controlled C runtime provider after its indirect jump through IAT.
        pe = pefile.PE(str(path))
        self.runtime = {}
        for descriptor in pe.DIRECTORY_ENTRY_IMPORT:
            for symbol in descriptor.imports:
                name = symbol.name.decode() if symbol.name else ''
                if name in ('memcpy', 'memset', 'memchr', 'strcpy', 'sprintf', '_snprintf'):
                    target = STOP + 0x100 + len(self.runtime) * 16
                    self.runtime[symbol.address] = target
                    self.callbacks[target] = (0, lambda n=name: self.crt(n))

    def string(self, pointer):
        return self.read(pointer, 256).split(b'\0', 1)[0]

    def crt(self, name):
        result = 0
        if name == 'memcpy':
            dest, src, count = self.args(3)
            self.u.mem_write(dest, self.read(src, count))
            result = dest
        elif name == 'memset':
            dest, value, count = self.args(3)
            self.u.mem_write(dest, bytes([value & 255]) * count)
            result = dest
        elif name == 'memchr':
            src, value, count = self.args(3)
            index = self.read(src, count).find(bytes([value & 255]))
            result = src + index if index >= 0 else 0
        elif name == 'strcpy':
            dest, src = self.args(2)
            self.u.mem_write(dest, self.string(src) + b'\0')
            result = dest
        elif name == '_snprintf':
            dest, count, fmt, value = self.args(4)
            assert self.string(fmt) == b'%s'
            data = self.string(value)[:count - 1]
            self.u.mem_write(dest, data + b'\0')
            result = len(data)
        else:
            dest, fmt = self.args(2)
            data = self.string(fmt)
            if data == b'%s\\gamesave\\%s':
                _, _, hd, filename = self.args(4)
                data = self.string(hd) + b'\\gamesave\\' + self.string(filename)
            elif b'\\pps\\' in data:
                data = b'profile.pps'
            assert b'%' not in data
            self.u.mem_write(dest, data + b'\0')
            result = len(data)
        self.u.reg_write(UC_X86_REG_EAX, result)

    def provider(self, address, nargs):
        args = self.args(nargs)
        result = 0
        index = args[0] if args else 0
        if address in (0x405d70, 0x405d80, 0x405d90, 0x405da0):
            result = {0x405d70: self.players, 0x405d80: 1,
                      0x405d90: self.difficulty, 0x405da0: self.players > 1}[address]
        elif address == 0x4086f0: result = index & 1
        elif address == 0x408d60: result = HEAP + 0x1000 + index * 0x30
        elif address == 0x408860: result = HEAP + 0x2000 + index * 0x650
        elif address == 0x407610: result = HEAP + 0x4000 + index * 0x148
        elif address == 0x407630: result = HEAP + 0x5000 + index * 7
        elif address == 0x40cfe0: result = index + 1
        elif address in (0x407520, 0x4075e0, 0x4075d0, 0x4075b0, 0x4075c0):
            result = HEAP + {0x407520: 0x6000, 0x4075e0: 0x6100,
                             0x4075d0: 0x6200, 0x4075b0: 0x6300, 0x4075c0: 0x6400}[address]
        elif address == 0x406910: result = self.country
        elif address == 0x406930: result = self.stage
        elif address == 0x406920: result = 2
        elif address == 0x4aa700: result = HEAP + 0x9800
        elif address == 0x4f5150: self.u.mem_write(index, b'new.rcs\0')
        elif address == 0x4f4db0: result = 1
        elif address == 0x4aa5a0:
            name, mode, source, count = args
            assert mode == 0 and count == 0x7f4
            self.written = self.read(source, count)
            result = self.write_result
        elif address == 0x4eadb0:
            self.u.mem_write(HEAP + 0x1000, b'\xa5' * (16 * 0x30))
            self.u.mem_write(HEAP + 0x2000, bytes(4 * 0x650))
            for i in range(4):
                self.put(HEAP + 0x2000 + i * 0x650 + 0x5c, '<I', 0xa5a00000)
        elif address == 0x40cff0: self.tiebreak[index] = args[1] & 255
        elif address in (0x4068b0, 0x4068e0, 0x4068d0):
            self.selection[address] = index & 255
        self.u.reg_write(UC_X86_REG_EAX, result)

    def prepare(self, players, difficulty, country, stage):
        self.reset()
        for iat, target in self.runtime.items(): self.put(iat, '<I', target)
        self.players, self.difficulty = players, difficulty
        self.country, self.stage = country, stage
        self.write_result = 1
        self.written = None
        self.tiebreak, self.selection = {}, {}
        self.u.mem_write(self.image, bytes(0x7f4))
        self.u.mem_write(self.image - 4, b'HEAD')
        self.u.mem_write(self.image + 0x7f4, b'TAIL')
        self.u.mem_write(HEAP + 0x9800, b'C:\\GAME\0')
        self.u.mem_write(HEAP + 0x9900, b'session\0')
        self.put(self.gameinfo + 0x9c, '<I', 0xa5b00055)
        for i in range(16):
            self.put(HEAP + 0x1000 + i * 0x30, '<Ii', (2 * i) | ((12 + i) << 6), 9000 + i)
        for i in range(4):
            profile = HEAP + 0x2000 + i * 0x650
            self.u.mem_write(profile + 0x10, bytes([ord('P'), ord('0') + i, 0, 0]))
            self.put(profile + 0x14, '<2I', 0x123400 + i, 0)
            self.put(profile + 0x5c, '<I', 0xa5a01234 + i)
            self.u.mem_write(HEAP + 0x4000 + i * 0x148, bytes([0x21 + i]) * 0x148)
            self.u.mem_write(HEAP + 0x5000 + i * 7, bytes([0x41 + i]) * 7)
        for offset, size, value in [(0x6000, 0xa0, 0x61), (0x6100, 0x50, 0x62),
                                    (0x6200, 0x14, 0x63), (0x6300, 0x50, 0x64),
                                    (0x6400, 0x50, 0x65)]:
            self.u.mem_write(HEAP + offset, bytes([value]) * size)

    def model(self):
        image = bytearray(0x7f4)
        image[0x10:0x18] = b'session\0'
        struct.pack_into('<I', image, 0x30, 1 | (self.difficulty << 7) |
                         (self.players << 10) | ((self.players > 1) << 14))
        for i in range(16):
            struct.pack_into('<Ii', image, 0x34 + i * 8,
                             (2 * i) | (((i & 1) if i < self.players else 0) << 6) |
                             ((12 + i) << 7), 9000 + i)
            image[0x7dc + i] = i + 1
        for i in range(self.players):
            struct.pack_into('<I', image, 0xb4 + i * 4, (0xa5a01234 + i) & 0x1fff)
            image[0xc4 + i * 12:0xc4 + (i + 1) * 12] = self.read(HEAP + 0x2000 + i * 0x650 + 0x10, 12)
            image[0xf4 + i * 0x148:0xf4 + (i + 1) * 0x148] = bytes([0x21 + i]) * 0x148
            image[0x614 + i * 7:0x614 + (i + 1) * 7] = bytes([0x41 + i]) * 7
        for offset, size, value in [(0x630, 0xa0, 0x61), (0x6d0, 0x50, 0x62),
                                    (0x720, 0x14, 0x63), (0x734, 0x50, 0x64),
                                    (0x784, 0x50, 0x65)]:
            image[offset:offset + size] = bytes([value]) * size
        image[0x7d4] = 2
        struct.pack_into('<I', image, 0x7d8, self.country | (self.stage << 5))
        return bytes(image)

    def save(self):
        immutable = self.read(HEAP, 0x9800)
        expected = self.model()
        self.invoke(0x4f5190, [0xffffffff, HEAP + 0x9900])
        assert self.u.reg_read(UC_X86_REG_EAX) & 255 == 1
        assert self.written == expected, 'session bytes disagree with original layout model'
        assert self.read(HEAP, 0x9800) == immutable, 'save modified source records'
        assert self.read(self.image - 4, 4) == b'HEAD' and self.read(self.image + 0x7f4, 4) == b'TAIL'
        return expected

    def overwrite(self, expected):
        self.u.mem_write(HEAP + 0x8000, expected)
        self.put(self.addr(0x81b14c), '<I', HEAP + 0x8000)
        self.put(self.addr(0x81b150), '<I', HEAP + 0x8f00)
        self.put(HEAP + 0x8f00, '<I', HEAP + 0x9000)
        self.u.mem_write(HEAP + 0x9000, b'old.rcs\0')
        self.invoke(0x4f5190, [0, 0])
        assert self.u.reg_read(UC_X86_REG_EAX) & 255 == 1
        assert self.written == expected, 'overwrite changed the displayed name or disk image'

    def load(self, image):
        self.u.mem_write(HEAP + 0x7000, image)
        self.invoke(0x4fb400, [HEAP + 0x7000])
        assert self.u.reg_read(UC_X86_REG_EAX) == 1
        assert self.read(HEAP + 0x7000, 0x7f4) == image
        return (self.read(HEAP + 0x1000, 16 * 0x30), self.read(HEAP + 0x2000, 4 * 0x650),
                self.read(HEAP + 0x4000, 0x520), self.read(HEAP + 0x5000, 28),
                self.read(HEAP + 0x6000, 0x450), self.read(self.gameinfo + 0x14, 4),
                self.read(self.gameinfo + 0x9c, 4), self.tiebreak, self.selection)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Session(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Session(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = 0
    for args in itertools.product(range(1, 5), range(3), (0, 5, 7), (0, 9, 10)):
        original.prepare(*args)
        rebuilt.prepare(*args)
        a, b = original.save(), rebuilt.save()
        assert a == b
        original.overwrite(a)
        rebuilt.overwrite(b)
        assert original.load(a) == rebuilt.load(b), ('restore mismatch', args)
        cases += 1
    for players in (0, 5, 15):
        rebuilt.prepare(players, 0, 0, 0)
        rebuilt.invoke(0x4f5190, [0xffffffff, HEAP + 0x9900])
        assert rebuilt.u.reg_read(UC_X86_REG_EAX) & 255 == 0 and rebuilt.written is None
    for result in (0, 2):
        rebuilt.prepare(1, 0, 0, 0)
        rebuilt.write_result = result
        rebuilt.invoke(0x4f5190, [0xffffffff, HEAP + 0x9900])
        assert rebuilt.u.reg_read(UC_X86_REG_EAX) & 255 == 0, 'write failure reported success'
    rebuilt.prepare(1, 0, 0, 0)
    good = rebuilt.model()
    for offset, word in [(0x30, 0), (0x30, 5 << 10), (0x7d8, 11 << 5), (0x7d8, 8)]:
        broken = bytearray(good)
        struct.pack_into('<I', broken, offset, word)
        rebuilt.u.mem_write(HEAP + 0x7000, bytes(broken))
        state = rebuilt.read(rebuilt.gameinfo, 0x9c + 4)
        heap = rebuilt.read(HEAP + 0x1000, 0x6000)
        rebuilt.invoke(0x4fb400, [HEAP + 0x7000])
        assert rebuilt.u.reg_read(UC_X86_REG_EAX) == 0
        assert rebuilt.read(rebuilt.gameinfo, 0x9c + 4) == state and rebuilt.read(HEAP + 0x1000, 0x6000) == heap
    print(cases, 'original/rebuilt saved-session cases: identical disk bytes, profiles, cars, controllers, times, tables, unlocks and guards; invalid player counts/stages rejected')
    return 0


if __name__ == '__main__':
    sys.exit(main())
