#!/usr/bin/env python3
"""Apply network car packets with actual arithmetic and accumulator helpers.

Usage: differential_net_car_receive.py entities.json [rebuilt.exe]
Runtime sine/square-root tables are populated identically. Executes the complete
receiver and real accumulator helper, checks discard conditions independently,
all record bytes, steering, input preservation and guards in four x87 modes.
Does not validate transport or complete network sessions.
"""
import itertools
import json
import math
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP, UC_X86_REG_FPCW
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting
from matching_entities import entity_address


class Receive(Lighting):
    def __init__(self, path, entities=None):
        if entities is not None:
            entities = dict(entities)
            entities["0x539cc8"] = [entity_address(entities, 0x539CC8, 0x539B38),
                                   "g_netTriangleState.enabled"]
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        self.put(self.addr(0x6E0EF4), "<4096H", *(
            round(math.sqrt((8 + 16 * i) / 65536) * 65536) & 65535 for i in range(4096)))
        self.put(self.addr(0x6E2EF4), "<4096i", *(
            round(math.sin(i * math.tau / 4096) * 65536) for i in range(4096)))
        self.initial_image = self.read(self.base, self.size)

    def run_receive(self, seed, gate, rounding):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, self.initial_image)
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        car, output = HEAP + 0x3000, HEAP + 0x5000
        packet = self.addr(0x539388)
        last = 4000 + seed * 10
        seq = last + (99 if gate == 7 else 100 if gate == 3 else
                      0 if gate == 1 else -1 if gate == 2 else 1)
        if gate == 6:
            last, seq = 65535, 0
        self.u.mem_write(packet - 16, rnd.randbytes(62))
        self.put(packet, "<H", seq)
        for offset in (2, 4, 8, 10):
            self.put(packet + offset, "<h", rnd.choice((-32768, -32765, -1, 0, 1, 32765, 32767)))
        self.put(packet + 6, "<H", 0 if seed % 5 == 0 else rnd.randrange(1000, 65536))
        sector_index = 4 if gate == 5 else seed % 4
        self.put(packet + 12, "<H", sector_index)
        for offset in range(14, 24):
            self.put(packet + offset, "<B", rnd.choice((0, 1, 127, 128, 254, 255)))
        steering = (0, 1, 63, 126, 127)[seed % 5]
        flags = (seed % 16) << 7
        self.put(packet + 26, "<H", flags | steering)
        self.put(car + 0xCA, "<H", last)
        self.put(car + 0xC6, "<h", (0, -1, 100)[seed % 3])
        self.put(self.addr(0x539CC8), "<B", 0 if gate == 4 else 1)
        self.put(self.addr(0x72D568), "<i", 4)
        for i in range(4):
            sector = HEAP + 0x7000 + i * 0x100
            self.put(self.addr(0x71F608) + i * 4, "<I", sector)
            self.put(sector, "<3i", *(rnd.randrange(-0x1000000, 0x1000000) for _ in range(3)))
        before = self.read(HEAP, 0x10000)
        packet_before = self.read(packet - 16, 62)
        self.u.reg_write(UC_X86_REG_FPCW, 0x27F | rounding)
        self.invoke(0x425C40, [car, output])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 12
        result = self.u.reg_read(UC_X86_REG_EAX)
        after = self.read(HEAP, 0x10000)
        assert result == (1 if gate in (0, 7) else 0)
        assert self.read(packet - 16, 62) == packet_before
        for a, b in ((0, 0x3000), (0x30EC, 0x5000), (0x5004, 0x10000)):
            assert after[a:b] == before[a:b]
        if not result:
            assert after == before
        else:
            assert after[0x30C8:0x30CC] == struct.pack("<2H", seq, seq)
            assert after[0x30E0:0x30E4] == struct.pack("<i", 1)
            expected = -65536 if steering == 0 else 65536 if steering == 127 else steering * 1048 - 65536
            assert after[0x5000:0x5004] == struct.pack("<i", expected)
            assert after[0x30B8:0x30BC] == struct.pack("<i", 65536 if flags & 128 else 0)
        return result, after


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Receive(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Receive(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities)
    count = 0
    for case in itertools.product(range(128), range(8), (0, 0x400, 0x800, 0xC00)):
        left, right = original.run_receive(*case), rebuilt.run_receive(*case)
        if left != right:
            offsets = [hex(i - 0x3000) for i, (a, b) in enumerate(zip(left[1], right[1])) if a != b]
            print("FAIL network receive", case, "record/output offsets", offsets[:30])
            for offset in offsets[:10]:
                i = int(offset, 16) + 0x3000
                print(offset, left[1][i:i+8].hex(), right[1][i:i+8].hex())
            return 1
        count += 1
    print(f"{count} received packets: identical guarded records and steering; actual helpers, discard boundaries and four x87 rounding modes")
    return 0


if __name__ == "__main__":
    sys.exit(main())
