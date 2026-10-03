#!/usr/bin/env python3
"""Compare car-preview loading and actual scene searches against original x86.

Usage: differential_car_preview_load.py entities.json [rebuilt.exe]
Files, geometry conversion, final damage setup and texture writes are controlled
leaves. SceneNode_FindByType runs for real on three distinct valid trees. Raw
C3D buffers contain small integers, exposing a search on unconverted file data.
"""
import itertools
import json
from pathlib import Path
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK


class PreviewLoad(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {
            self.addr(0x501510): (0, lambda: self.result(HEAP + 0xf000)),
            self.addr(0x405d80): (0, lambda: self.result(1)),
            self.addr(0x4086b0): (4, lambda: self.result(0)),
            self.addr(0x40ee90): (4, lambda: self.result(2)),
            self.addr(0x420060): (12, lambda: self.result(HEAP + 0xe000)),
            self.addr(0x405d10): (0, lambda: self.result(self.standard)),
            self.addr(0x50a050): (8, lambda: self.result(self.has_light)),
            self.addr(0x50a020): (8, lambda: self.result(self.has_snow)),
            self.addr(0x405620): (0, self.format),
            self.addr(0x4c6694): (0, self.copy_string),
            self.addr(0x4aa220): (8, self.file),
            self.addr(0x4a9d70): (8, self.archive),
            self.addr(0x4b9380): (12, self.convert),
            self.addr(0x4addd0): (8, self.mask),
            self.addr(0x507080): (4, lambda: self.event('meshes', 1)),
            self.addr(0x507a10): (8, self.damage),
            self.addr(0x4a4d30): (32, lambda: self.event('texture-alpha', 8)),
            self.addr(0x507650): (4, lambda: self.event('finish', 1)),
            self.addr(0x50f120): (4, lambda: self.event('variant', 1)),
        }

    def result(self, value):
        self.u.reg_write(UC_X86_REG_EAX, value)

    def event(self, name, nargs):
        self.trace.append((name, self.args(nargs)))

    def string(self, address):
        for length in range(260):
            if self.read(address + length, 1) == b'\0':
                return self.read(address, length)
        raise AssertionError('unterminated path')

    def format(self):
        dest, pattern, text = self.args(3)
        result = self.string(pattern) % self.string(text)
        self.u.mem_write(dest, result + b'\0')
        self.result(len(result))

    def copy_string(self):
        dest, src, length = self.args(3)
        self.u.mem_write(dest, self.string(src)[:length].ljust(length, b'\0'))
        self.result(dest)

    def file(self):
        path, flag = self.args(2)
        name = self.string(path)
        part = 1 if name.endswith(b'L.c3d') else 2 if name.endswith(b'S.c3d') else 0
        self.trace.append(('file', name, flag))
        self.result(0 if self.missing and part == 0 else HEAP + 0x2000 + part * 0x100)

    def archive(self):
        loader, path = self.args(2)
        self.trace.append(('archive', loader - self.addr(0x831088), self.string(path)))

    def convert(self):
        data, stage, loader = self.args(3)
        part = (data - HEAP - 0x2000) // 0x100
        assert 0 <= part < 3 and stage == HEAP + 0xf000
        self.trace.append(('convert', part, loader - self.addr(0x831088)))
        self.result(HEAP + 0x4000 + part * 0x2000)

    def mask(self):
        node, mask = self.args(2)
        self.trace.append(('mask', node, mask))

    def damage(self):
        record, index = self.args(2)
        self.trace.append(('damage', record - self.addr(0x82d220), index))

    def hook(self, u, address, size, data):
        if address == self.addr(0x4ad890):
            node, kind = self.args(2)
            assert node == 0 or HEAP + 0x4000 <= node < HEAP + 0xa000, (
                'scene search received raw C3D file data', hex(node), kind)
        super().hook(u, address, size, data)

    def run(self, index, standard, light, snow, missing, headlight):
        self.reset()
        self.standard, self.has_light, self.has_snow, self.missing = standard, light, snow, missing
        self.put(self.addr(0x831318), '<I', 1)
        self.u.mem_write(self.addr(0x82cb78), bytes([0xa5]) * (16 * 0x54))
        self.u.mem_write(HEAP + 0xe000, b'GAME\\CARS\\FORD\\FORDA1\0')
        # A raw C3D header is not a SceneNode: its first word can be 6,
        # matching the invalid pointer in the Finland preview crash.
        for part in range(3):
            self.put(HEAP + 0x2000 + part * 0x100, '<I', 6)
            tree = HEAP + 0x4000 + part * 0x2000
            self.put(tree + 4, '<I', tree + 0x200)
            self.put(tree + 0x30, '<I', 0xfd)
            kinds = [1, 2, 3, 4, 5, 14 if headlight else 15]
            for slot, kind in enumerate(kinds):
                node = tree + (slot + 1) * 0x200
                self.put(node, '<I', node + 0x200 if slot < 5 else 0)
                self.put(node + 0xc, '<I', HEAP + 0xc000 if slot == 5 else HEAP + 0xb000 + slot * 0x20)
                self.put(node + 0x30, '<I', kind)
        self.put(HEAP + 0xc024, '<I', HEAP + 0xc100)
        self.put(HEAP + 0xc104, '<I', 2)
        self.put(self.addr(0x520b78), '<I', HEAP + 0xd000)
        self.put(HEAP + 0xd000 + 908, '<I', HEAP + 0xd800)
        self.invoke(0x5062d0, [index])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff08, 'ret 4 ABI'
        return (self.u.reg_read(UC_X86_REG_EAX), self.trace,
                self.read(self.addr(0x82cb78), 16 * 0x54))


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = PreviewLoad(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = PreviewLoad(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0
    for case in itertools.product([0, 3, 7], range(2), range(2), range(2), range(2), range(2)):
        try:
            left, right = original.run(*case), rebuilt.run(*case)
            assert left == right, ('preview calls or record differ', left[:2], right[:2])
        except AssertionError as error:
            print('FAIL car preview', case, error)
            return 1
        count += 1
    print(count, 'car previews: identical asset selection, scene searches, records and headlight alpha; raw-data search rejected')
    return 0


if __name__ == '__main__':
    sys.exit(main())
