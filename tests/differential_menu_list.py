#!/usr/bin/env python3
"""Compare original/rebuilt menu drawing calls with valid menu records.

Usage: differential_menu_list.py entities.json [rebuilt.exe]
Runs both complete MenuList bodies in Unicorn. Only the text provider and
rendering leaves are intercepted; rectangles, text positions, colours and call
order must agree, including the first row and a poisoned prior rectangle.
"""
import itertools
import json
from pathlib import Path
import struct
import sys

import pefile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP

from matching_entities import entity_address

ROOT = Path(__file__).resolve().parents[1]
HEAP, STACK, STOP = 0x10000000, 0x20000000, 0x30000000


class Drawing:
    def __init__(self, path, entities=None):
        pe = pefile.PE(str(path))
        self.memory = pe.get_memory_mapped_image()
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.size = (pe.OPTIONAL_HEADER.SizeOfImage + 4095) & ~4095
        self.entities = entities
        self.u = Uc(UC_ARCH_X86, UC_MODE_32)
        self.u.mem_map(self.base, self.size)
        self.u.mem_map(HEAP, 0x10000)
        self.u.mem_map(STACK, 0x10000)
        self.u.mem_map(STOP, 4096)
        self.callbacks = {
            self.addr(0x4a3c60): (4, self.text),
            self.addr(0x4a5e40): (16, self.fill),
            self.addr(0x4a3290): (36, self.sprite),
            self.addr(0x40b880): (24, self.font),
        }
        self.u.hook_add(UC_HOOK_CODE, self.hook)

    def addr(self, original):
        return original if self.entities is None else self.entities[hex(original)][0]

    def member(self, address, owner):
        """Address of a member of a real block (view of `owner`'s entity)."""
        return address if self.entities is None else entity_address(self.entities, address, owner)

    def read(self, address, size):
        return bytes(self.u.mem_read(address, size))

    def put(self, address, fmt, *values):
        self.u.mem_write(address, struct.pack(fmt, *values))

    def args(self, count):
        return struct.unpack('<' + 'I' * count, self.read(self.u.reg_read(UC_X86_REG_ESP) + 4, count * 4))

    def rect(self, address):
        return struct.unpack('<4h', self.read(address, 8))

    def text(self):
        index, = self.args(1)
        self.trace.append(('string', index))
        self.u.reg_write(UC_X86_REG_EAX, HEAP + 0x8000 + index * 16)

    def fill(self):
        context, rect, colour, layer = self.args(4)
        self.trace.append(('fill', context, self.rect(rect), self.read(colour, 4), layer))

    def sprite(self):
        src, dst, texture, layer, angle, centre, unused, colour, flags = self.args(9)
        self.trace.append(('sprite', self.rect(src), self.rect(dst), texture, layer,
                           angle, centre, unused, self.read(colour, 4), flags))

    def font(self):
        index, text, x, y, colour, flags = self.args(6)
        self.trace.append(('font', index, text, x & 0xffff, y & 0xffff,
                           self.read(colour, 4), flags))

    def hook(self, u, address, size, data):
        if address in self.callbacks:
            popped, callback = self.callbacks[address]
            callback()
            sp = u.reg_read(UC_X86_REG_ESP)
            target, = struct.unpack('<I', self.read(sp, 4))
            u.reg_write(UC_X86_REG_ESP, sp + 4 + popped)
            u.reg_write(UC_X86_REG_EIP, target)

    def run(self, resolution, count, cursor, first, title, offset, y, active, mask):
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, bytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        self.put(self.addr(0x520b74), '<I', HEAP + 0x1000)
        self.put(HEAP + 0x1000, '<2I', *resolution)
        for global_, texture in [(0x818548, HEAP + 0x2000), (0x8185b8, HEAP + 0x3000)]:
            self.put(self.addr(global_), '<I', texture)
            self.put(texture + 0x11c, '<4H', 0, 0, 282, 36)
        self.put(HEAP + 6, '<2b', count, cursor)
        for i in range(count):
            self.put(HEAP + 0x14 + i * 0x14, '<IhB', HEAP + 0x9000 + i * 16,
                     -1 if i % 2 else i + 1, (2 if mask & (1 << i) else 0) | (i % 3 != 1))
        self.put(self.addr(0x8189a8), '<4h', -1777, -2333, 123, 45)
        args = [HEAP, HEAP + 0x9500 if title else 0, y, offset, first, active]
        sp = STACK + 0xff00
        self.put(sp, '<7I', STOP, *(v & 0xffffffff for v in args))
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(self.addr(0x4d3360), STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'menu did not return'
        return self.trace, self.rect(self.addr(0x8189a8))


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Drawing(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Drawing(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = 0
    for resolution, count, title, offset, y, active in itertools.product(
            [(640, 480), (800, 600), (1024, 768)], [1, 2, 5], [False, True],
            [-1, 0, 57], [-1, 300], [0, 1]):
        masks = {(1 << count) - 1, 1 << (count - 1)}
        for mask in masks:
            for cursor in range(count):
                for first in range(count):
                    # MenuList's 'first' counts skipped rows, so use a visible
                    # prefix when testing nonzero starting indices.
                    if first and mask != (1 << count) - 1:
                        continue
                    args = (resolution, count, cursor, first, title, offset, y, active, mask)
                    a, b = original.run(*args), rebuilt.run(*args)
                    if a != b:
                        print('FAIL menu case:', args)
                        for n, pair in enumerate(itertools.zip_longest(a[0], b[0])):
                            if pair[0] != pair[1]:
                                print('call', n, 'original:', pair[0], 'rebuilt:', pair[1])
                                break
                        print('final rectangles:', a[1], b[1])
                        return 1
                    cases += 1
    print(f'{cases} menu cases: identical drawing calls, first-row positions and final rectangles')
    return 0


if __name__ == '__main__':
    sys.exit(main())
