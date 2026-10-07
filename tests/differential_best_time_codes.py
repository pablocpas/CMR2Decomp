#!/usr/bin/env python3
"""Execute all five best-time encodings and their real string builders.

Only the selected save-record provider is controlled. Compare its call count,
packed words/tags, every output byte, unused cells, guards and the stack ABI.
Usage: differential_best_time_codes.py entities.json [rebuilt.exe]
"""
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP

TABLES = [(0x825398, 0x4c), (0x8253e4, 0x98), (0x82547c, 0x898),
          (0x825d14, 0x258), (0x825f6c, 0x1cc)]
TIMES = [0, 1, 99, 100, 5999, 6000, 95999, 96000,
         383999, 384000, 0x7fffffff, 0x80000000, 0xffffffff]


class BestTimeCodes(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {self.addr(0x408cb0): (4, self.record)}
        self.threshold_mutation = False

    def record(self):
        assert self.args(1) == (0,), 'selected record index'
        self.calls += 1
        self.u.reg_write(UC_X86_REG_EAX, HEAP + 0x1000)

    def cell(self, pointer):
        for table, size in TABLES:
            offset = pointer - self.addr(table)
            if 0 <= offset < size:
                assert offset % 25 == 0
                return table, offset // 25
        raise AssertionError(('output outside the five tables', hex(pointer)))

    def hook(self, u, address, size, data):
        if address == self.addr(0x4fb8d0):
            profile, word, tag, output = self.args(4)
            assert profile == 0
            self.trace.append((self.cell(output), self.read(word, 4), self.read(tag, 1)))
        super().hook(u, address, size, data)

    def run(self, seed, stack_byte):
        self.u.mem_write(self.base, bytes(self.memory))
        if self.threshold_mutation:
            assert self.entities is None
            assert self.read(0x4f8ba8, 3) == b'\x83\xfa\x10'
            self.u.mem_write(0x4f8baa, b'\x11')
        self.u.mem_write(HEAP, bytes([0xa5]) * 0x10000)
        self.u.mem_write(STACK, bytes([stack_byte]) * 0x10000)
        rng = random.Random(seed)
        saved = bytearray(rng.randbytes(0x600))
        records = [(0x150 + country * 0x60 + stage * 8, 4)
                   for country in range(8) for stage in range(10 + country % 2)]
        records += [(0x30 + difficulty * 12 + country * 36, 8)
                    for difficulty in range(3) for country in range(8)]
        records += [(difficulty * 16, 4) for difficulty in range(3)]
        records += [(0x450 + difficulty * 12 + country * 36, 4)
                    for difficulty in range(3) for country in range(2)]
        records += [(0x4bc + event * 8, 4) for event in range(8)]
        for index, (offset, time_offset) in enumerate(records):
            flags = rng.getrandbits(32)
            valid = seed % 4 == 1 or (seed % 4 >= 2 and (index + seed) % 3 != 0)
            flags = flags | 0x80 if valid else flags & ~0x80
            struct.pack_into('<I', saved, offset, flags)
            if 0x150 <= offset < 0x450 or offset >= 0x4bc or 0x30 <= offset < 0x150:
                value = TIMES[(index + seed // 4) % len(TIMES)]
                if seed % 4 == 1:
                    value = rng.randrange(96000)
                struct.pack_into('<I', saved, offset + time_offset, value)
        self.u.mem_write(HEAP + 0x1000, bytes(saved))
        initial = {}
        for table, size in TABLES:
            initial[table] = rng.randbytes(size + 16)
            self.u.mem_write(self.addr(table), initial[table])
        ranges = [(self.addr(table), self.addr(table) + size) for table, size in TABLES]
        guards = {address: self.read(address, 1)
                  for start, end in ranges
                  for address in [*range(start - 16, start), *range(end, end + 16)]
                  if not any(low <= address < high for low, high in ranges)}
        self.trace, self.calls = [], 0
        sp = STACK + 0xff00
        self.put(sp, '<I', STOP)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.u.emu_start(self.addr(0x4f8b30), STOP, count=2000000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 4, 'stack ABI'
        assert self.calls == 250, ('save record calls', self.calls)
        assert self.read(HEAP + 0x1000, len(saved)) == saved, 'save record modified'
        outputs = [self.read(self.addr(table), size) for table, size in TABLES]
        # All writes must stay within the string actually produced, or be the
        # single zero byte that clears an invalid record. Spare cells stay intact.
        cells = {table: set() for table, _ in TABLES}
        cells[0x825398] = set(range(3))
        cells[0x8253e4] = set(range(6))
        cells[0x82547c] = {country * 11 + stage for country in range(8)
                          for stage in range(10 + country % 2)}
        cells[0x825d14] = set(range(24))
        cells[0x825f6c] = set(range(8))
        for (table, size), output in zip(TABLES, outputs):
            allowed = set()
            for cell in cells[table]:
                start = cell * 25
                end = output.index(0, start, start + 25) + 1
                allowed.update(range(start, end))
            assert all(byte == initial[table][i] for i, byte in enumerate(output)
                       if i not in allowed), ('table guards', hex(table))
        assert all(self.read(address, 1) == byte for address, byte in guards.items()), 'outer guards'
        return outputs, self.trace, self.calls


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = BestTimeCodes(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = BestTimeCodes(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0
    for seed in range(64):
        for poison in (0, 0xa5):
            a, b = original.run(seed, poison), rebuilt.run(seed, poison)
            assert a == b, ('best-time codes', seed, poison,
                            next(((x, y) for x, y in zip(a[1], b[1]) if x != y), None))
            count += 1
    mutation = BestTimeCodes(ROOT / 'cmr2bin/CMR2.exe')
    mutation.threshold_mutation = True
    assert original.run(2, 0xa5) != mutation.run(2, 0xa5), 'missed 16-minute threshold mutation'
    print(f'{count} best-time code fixtures: five tables, real scrambled strings, '
          'time thresholds, tags, retained fields, guards and ABI agree; threshold mutation detected')
    return 0


if __name__ == '__main__':
    sys.exit(main())
