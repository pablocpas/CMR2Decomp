#!/usr/bin/env python3
"""Compare the complete network setup callback with controlled input/network leaves.

Usage: differential_network_session_setup.py entities.json [rebuilt.exe]
Checks category table indexing, chat edits, ready flags, menu transitions and
callee order against the original. It does not exercise network transport.
"""
import itertools
import json
from pathlib import Path
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting


class NetworkSetup(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in [
            (0x49ead0, 1), (0x4a03b0, 2), (0x408600, 2),
            (0x4ec2b0, 0), (0x4eb0c0, 2), (0x4b7c80, 0),
            (0x4ea5c0, 1), (0x4a0460, 5), (0x4a1af0, 0),
            (0x4b7cd0, 1), (0x4d0700, 1), (0x40b5b0, 2),
            (0x4a15a0, 0), (0x409cb0, 1), (0x40a470, 1),
            (0x4ec8d0, 0),
        ]:
            self.callbacks[self.addr(address)] = (
                nargs * 4, lambda a=address, n=nargs: self.provider(a, n))
        self.callbacks[self.addr(0x4c66b2)] = (0, self.find_character)

    def string(self, pointer):
        return self.read(pointer, 256).split(b'\0', 1)[0]

    def provider(self, address, nargs):
        args = self.args(nargs)
        result = 0
        if address == 0x4a03b0:
            assert args[0] == HEAP
            args = (args[1],)
            result = HEAP + 0x14 + args[0] * 20
        elif address in (0x408600, 0x4eb0c0):
            assert args[0] == 0
            args = (args[0], args[1] & 0xff)
        elif address == 0x49ead0:
            assert args == (0,)
            result = HEAP + 0x2000
        elif address == 0x4a0460:
            assert args[0] == HEAP
            args = args[1:]
        elif address == 0x4b7cd0:
            self.put(args[0], '<I', 0 if self.key is None else self.key)
            args = ()
            result = 0x12340000 | (self.key is not None)
        elif address == 0x4d0700:
            args = (self.string(args[0]),)
        elif address == 0x40b5b0:
            assert args[0] == 1
            args = (1, self.string(args[1]))
            result = self.width
        elif address == 0x4a15a0:
            result = self.hosting
        elif address == 0x409cb0:
            result = 0x12340000 | (args[0] in (0, 2, 6))
        elif address == 0x40a470:
            result = 0x12340000 | (args[0] != self.unready)
        self.trace.append((address, args))
        self.u.reg_write(UC_X86_REG_EAX, result)

    def find_character(self):
        pointer, char = self.args(2)
        assert pointer == self.addr(0x5252d4)
        self.trace.append(('character', char))
        index = self.string(pointer).find(bytes([char & 0xff]))
        self.u.reg_write(UC_X86_REG_EAX, 0 if index < 0 else pointer + index)

    def run(self, category, changed, key, hosting, unready=2,
            cursor=1, previous=1, device=0, length=3, width=20, ready=0,
            resolution=640):
        self.reset()
        self.key, self.hosting, self.unready, self.width = key, hosting, unready, width
        self.put(HEAP + 6, '<2b', 5, cursor)
        for i in range(5):
            self.put(HEAP + 0x14 + i * 20 + 6, '<B', 0xa6)
            self.put(HEAP + 0x14 + i * 20 + 8, '<h', i)
        self.put(HEAP + 0x14 + 20 + 11, '<B', category)
        self.put(HEAP + 0x14 + 40 + 11, '<B', 1)
        self.put(HEAP + 0x14 + 60 + 11, '<B', ready)
        self.put(HEAP + 0x2008, '<I', device)
        self.put(self.addr(0x520b74), '<I', HEAP + 0x1000)
        self.put(HEAP + 0x1000, '<2I', resolution, 480)
        self.put(self.addr(0x818ce8), '<I', category + changed)
        self.put(self.addr(0x818ce0), '<I', 0 if changed else 1)
        self.put(self.addr(0x818d00), '<I', previous)
        self.put(self.addr(0x818ed0), '<B', key is not None)
        self.put(self.addr(0x818ce4), '<B', ready)
        # Nonidentity values and nonzero upper bytes expose a byte-stride read.
        table = [0xa5b6c700 | ((i * 7 + 3) % 22) for i in range(22)]
        self.put(self.addr(0x818d18), '<22I', *table)
        chat = self.addr(0x818f14)
        self.u.mem_write(chat - 4, b'GUAR' + b'x' * length + b'\0' + b'\xa5' * (255 - length))
        self.u.mem_write(chat + 256, b'TAIL')
        self.put(self.addr(0x819028), '<I', length)
        immutable = [(self.addr(0x818d18), 88), (HEAP + 0x1000, 8)]
        before = [self.read(p, n) for p, n in immutable]
        self.invoke(0x4ed840, [HEAP])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff08, 'ret 4 ABI'
        assert before == [self.read(p, n) for p, n in immutable], 'input changed'
        assert self.read(chat - 4, 4) == b'GUAR' and self.read(chat + 256, 4) == b'TAIL'
        categories = [args[1] for address, args in self.trace if address == 0x408600]
        assert categories == ([table[category] & 0xff] if changed else []), 'category model'
        can_start = not hosting or unready not in (0, 2, 6)
        assert self.read(HEAP + 0x14 + 60 + 6, 1) == bytes([0xa6 | can_start]), 'ready flag model'
        state = [self.read(self.addr(a), n) for a, n in [
            (0x818ce8, 4), (0x818ce0, 4), (0x818d00, 4),
            (0x818ed0, 1), (0x818ce4, 1), (0x819028, 4), (0x818f14, 256)]]
        return self.trace, state, self.read(HEAP, 0x1e0)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = NetworkSetup(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = NetworkSetup(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = list(itertools.product(range(22), (0, 1), (None, 8, 13, 27, ord('a'), ord('@')), (0, 1)))
    for cursor, previous, device, length, width, ready in itertools.product(
            (0, 1, 4), (0, 1), (0, 4, 8), (0, 255), (20, 400), (0, 1)):
        cases.append((7, 1, ord('b'), 0, 2, cursor, previous, device, length, width, ready))
    for unready in (-1, 0, 2, 6):
        cases.append((7, 1, None, 1, unready))
    for resolution, limit in ((640, 330), (1024, 528)):
        for width in (limit - 1, limit):
            cases.append((7, 1, ord('b'), 0, 2, 1, 1, 0, 3, width, 0, resolution))
    for case in cases:
        try:
            a, b = original.run(*case), rebuilt.run(*case)
            assert a == b, (a, b)
        except Exception as error:
            print('FAIL network setup', case, error)
            return 1
    print(len(cases), 'network setup cases: identical categories, chat, flags, calls, guards and ret 4')
    return 0


if __name__ == '__main__':
    sys.exit(main())
