#!/usr/bin/env python3
"""Five whole result merges, arcade minimum and packed time getter vs original."""
import json
from pathlib import Path
import random
import struct
import sys
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP
from differential_stage_lighting import Lighting, ROOT, HEAP, STACK

MERGES = (0x4cf5b0, 0x4cf660, 0x4cf740, 0x4cf830, 0x4cf8e0)


def case(seed, kind):
    rng = random.Random(seed)
    heap = bytearray(rng.randbytes(0x10000))
    country, stage, level, group = seed % 8, (seed // 8) % 12, (seed // 96) % 3, (seed // 32) % 3
    offset = (0x150 + (country * 12 + stage) * 8, 0x30 + (country * 3 + level) * 12,
              level * 16, 0x4bc + (group * 3 + stage % 3) * 8,
              0x450 + (group * 3 + level) * 12)[kind]
    source_offset = (0x14, 0xc, 0, 0x20, 0x18)[kind]
    source = rng.randrange(1 << 32)
    dest = rng.randrange(1 << 32)
    # Include equal, better and worse levels/times and absent records.
    if seed % 5 == 0:
        dest = source
    elif seed % 5 == 1:
        source, dest = 1, 2
    elif seed % 5 == 2:
        source, dest = 2, 1
    header = rng.randrange(1 << 32)
    header = (header | 0x80) if seed % 2 else (header & ~0x80)
    struct.pack_into('<I', heap, 0x3000 + source_offset, source)
    struct.pack_into('<2I', heap, 0x1000 + offset, header, dest)
    if kind in (1, 2, 4):
        struct.pack_into('<I', heap, 0x3000 + source_offset + 4, rng.randrange(1 << 32))
    return heap, (country, stage, level, group), offset, source_offset


def oracle(heap, kind, offset, source_offset, missing):
    expected = bytearray(heap)
    header, dest = struct.unpack_from('<2I', heap, 0x1000 + offset)
    source = struct.unpack_from('<I', heap, 0x3000 + source_offset)[0]
    better = False
    if not missing:
        if kind in (0, 3):
            better = source < dest
        elif kind == 1:
            extra = struct.unpack_from('<I', heap, 0x3000 + source_offset + 4)[0]
            old_extra = struct.unpack_from('<I', heap, 0x1000 + offset + 8)[0]
            better = (source & 15) < (dest & 15) or ((source & 15) == (dest & 15) and extra < old_extra)
        else:
            level_mask, score_mask, shift = (15, 0x3fc0, 6) if kind == 2 else (7, 0x7e0, 5)
            better = (source & level_mask) < (dest & level_mask) or (
                (source & level_mask) == (dest & level_mask) and
                ((source & score_mask) >> shift) > ((dest & score_mask) >> shift))
        better = better or not (header & 0x80)
    if better:
        header = ((header | 0x80) & ~0x1f00) | ((0x7654 & 31) << 8)
        header = (header & ~63) | (0x45 & 63)
        header = (header & ~64) | 64
        mask = (0xffffffff, 0x3fff, 0x3fff, 0xffffffff, 0x7ff)[kind]
        dest = (dest & ~mask) | (source & mask)
        struct.pack_into('<2I', expected, 0x1000 + offset, header, dest)
        if kind in (1, 2, 4):
            extra = heap[0x3000 + source_offset + 4:0x3000 + source_offset + 8]
            expected[0x1000 + offset + 8:0x1000 + offset + 12] = extra
    return bytes(expected), int(better)


class Results(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address in (0x4d0280, 0x4d02d0, 0x408c70, 0x408cb0, 0x4086b0, 0x4086f0, 0x408d00):
            self.callbacks[self.addr(address)] = (4, lambda a=address: self.provider(a))
        self.callbacks[self.addr(0x4c6676)] = (0, lambda: self.provider(0x4c6676))
        self.callbacks[self.addr(0x405fe0)] = (0, lambda: self.provider(0x405fe0))

    def provider(self, address):
        args = self.args(1) if address not in (0x4c6676, 0x405fe0) else ()
        if args:
            assert args[0] & 255 == self.index
        value = {0x4d0280: 0 if self.missing else HEAP + 0x3014,
                 0x4d02d0: 0 if self.missing else HEAP + 0x300c,
                 0x408c70: 0 if self.missing else HEAP + 0x3000,
                 0x408cb0: HEAP + 0x1000, 0x4086b0: 0x45,
                 0x4086f0: 3, 0x408d00: 0, 0x4c6676: 0x7654,
                 0x405fe0: HEAP + 0x6000}[address]
        self.trace.append(address)
        self.u.reg_write(UC_X86_REG_EAX, value)

    def merge(self, seed, kind):
        self.reset()
        heap, (country, stage, level, group), offset, source_offset = case(seed, kind)
        self.index = seed % 4
        self.missing = seed % 13 == 0 and kind in (0, 1, 2)
        self.u.mem_write(HEAP, bytes(heap))
        for address, value in ((0x8173f8, country), (0x8173fc, stage if kind == 0 else stage % 3),
                               (0x817400, level), (0x817404, group)):
            self.put(self.addr(address), '<I', value)
        dirty = bytearray(b'\xa5' * 40)
        self.u.mem_write(self.addr(0x817420), bytes(dirty))
        expected, result = oracle(heap, kind, offset, source_offset, self.missing)
        if result and kind != 3:
            dirty[self.index * 10 + (0, 1, 2, 0, 3)[kind]] = 1
        args = [self.index] if kind == 3 else [self.index, HEAP + 0x1000]
        self.invoke(MERGES[kind], args)
        assert self.read(HEAP, 0x10000) == expected, ('whole record/heap', seed, kind)
        assert self.read(self.addr(0x817420), 40) == bytes(dirty), ('dirty flags', seed, kind)
        assert self.u.reg_read(UC_X86_REG_EAX) & 255 == result
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff04 + len(args) * 4
        assert self.trace.count(0x408d00) == result and self.trace.count(0x4c6676) == result
        return expected, bytes(dirty), result, self.trace

    def lookup(self, seed, getter):
        self.reset()
        heap = random.Random(seed).randbytes(0x10000)
        self.u.mem_write(HEAP, heap)
        self.index, self.missing = seed % 4, False
        minimum = seed * 1234567 & 0xffffffff
        self.put(self.addr(0x817570), '<I', minimum)
        group, stage = seed % 3, seed // 3 % 3
        self.put(self.addr(0x817404), '<I', group)
        self.put(self.addr(0x8173fc), '<I', stage)
        if getter:
            expected = struct.unpack_from('<I', heap, 0x6000 + 0x1214 + (group * 3 + stage) * 8)[0] >> 7 & 65535
        else:
            value = struct.unpack_from('<I', heap, 0x3020)[0]
            expected = int(value < minimum)
            minimum = min(value, minimum)
        args = [] if getter else [0, self.index]
        self.invoke(0x4cfe50 if getter else 0x4cfe20, args)
        assert self.u.reg_read(UC_X86_REG_EAX) == expected
        assert self.read(HEAP, 0x10000) == heap
        assert self.read(self.addr(0x817570), 4) == struct.pack('<I', minimum)
        return expected, minimum, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    rebuilt = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe'
    impl = [Results(ROOT / 'cmr2bin/CMR2.exe'), Results(rebuilt, entities)]
    count = 0
    for seed in range(576):
        for kind in range(5):
            assert impl[0].merge(seed, kind) == impl[1].merge(seed, kind)
            count += 1
        for getter in (False, True):
            assert impl[0].lookup(seed, getter) == impl[1].lookup(seed, getter)
            count += 1
    print(f'{count} category best-result cases: original/rebuilt/model, full heap and dirty flags')


if __name__ == '__main__':
    main()
