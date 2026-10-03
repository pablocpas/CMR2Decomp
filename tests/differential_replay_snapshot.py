#!/usr/bin/env python3
"""Compare replay matrix snapshots, counters and car updates with controlled leaves.

Usage: differential_replay_snapshot.py entities.json [rebuilt.exe]
Car lookup, frame loading, event/geometry updates and matrix interpolation are
controlled providers. The complete tick, snapshot and fixed-point/float arithmetic
execute in both images; this does not validate those provider implementations.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting


class Snapshot(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x42B5F0, 1), (0x418BA0, 3), (0x46DE20, 8),
                               (0x46D2A0, 1), (0x46E340, 2), (0x4224E0, 7)):
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        args = self.args(nargs)
        observation = self.read(HEAP + 0x3044, 64) if address == 0x46E340 else b""
        if address == 0x4224E0:
            observation = self.read(args[1], 64), self.read(args[2], 64)
            assert args[3] == args[4] == args[5] and args[6] == 1
            left, right = (struct.unpack("<16i", self.read(p, 64)) for p in args[1:3])
            values = [a + ((b - a) * args[3] >> 16) for a, b in zip(left, right)]
            self.put(args[0], "<16i", *values)
        self.trace.append((address, args, observation))
        # Exercise the ordinary x86 callee-clobbered register contract.
        self.u.reg_write(UC_X86_REG_EAX, HEAP + 0x1000 if address == 0x42B5F0 else 0x12345678)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def run_snapshot(self, seed, phase, live, expired, gate, missing=False):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(STACK, bytes(0x10000))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        car, record, world = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x5000
        self.put(car + 0x750, "<I", world)
        self.put(car + 0xB1A, "<b", (0, 1, 7)[seed % 3])
        self.put(car + 0xB16, "<h", (0, 1, 3)[seed % 3])
        self.put(car + 0xB43, "<B", 0 if gate == 3 else 1)
        self.put(car + 0x788, "<i", rnd.randrange(65536))
        self.put(car + 0x408, "<3i", *(rnd.randrange(-0x100000, 0x100000) for _ in range(3)))
        self.put(record + 4, "<i", 0 if gate == 1 else 1)
        self.put(record + 0x1C, "<i", 1 if gate == 2 else 2)
        self.put(record + 0x20, "<B", seed % 8)
        for offset in (0x44, 0x84):
            self.put(record + offset, "<16i", *(rnd.randrange(-65536, 65537) for _ in range(16)))
        self.put(record + 0xC4, "<3i", *(rnd.randrange(-65536, 65537) for _ in range(3)))
        self.put(record + 0xD0, "<4i", -5000, 12000, 3000, 20000)
        self.put(record + 0xE0, "<2i", -1, 2 if seed % 2 else 0)
        self.put(record + 0xEC, "<3i", seed, 0, live)
        self.put(record + 0xF8, "<B", phase)
        self.put(record + 0xFE, "<h", 2)
        self.put(record + 0x40, "<I", HEAP + 0x7000)
        self.put(record + 0x104, "<I", HEAP + 0x6000)
        self.put(record + 0x108, "<h", 1)
        self.put(record + 0x10A, "<h", 0)
        self.put(HEAP + 0x6000, "<2h", 10, 1 if expired else 10)
        before = self.read(HEAP, 0x10000)
        self.trace = []
        self.invoke(0x46D610, [0 if missing else record])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 8
        after = self.read(HEAP, 0x10000)
        for a, b in ((0, 0x1000), (0x1C24, 0x3000), (0x310C, 0x5000), (0x5040, 0x10000)):
            assert after[a:b] == before[a:b]
        if missing or gate:
            assert after == before
            assert len(self.trace) == (0 if missing else 1)
        else:
            assert after[0x1414:0x1420] == before[0x1408:0x1414]
            assert after[0x30F8] == (1 if phase >= 3 else phase + 1)
            if phase >= 3:
                assert after[0x3044:0x3084] == before[0x3084:0x30C4]
        return after, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Snapshot(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Snapshot(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities)
    count = 0
    for case in itertools.product(range(24), (0, 1, 2, 3, 6), (0, 1), (0, 1), range(4)):
        if original.run_snapshot(*case) != rebuilt.run_snapshot(*case):
            print("FAIL replay snapshot", case)
            return 1
        count += 1
    assert original.run_snapshot(0, 0, 0, 0, 0, True) == rebuilt.run_snapshot(0, 0, 0, 0, 0, True)
    print(f"{count + 1} replay ticks: identical snapshots, counters, car state, guarded heap and provider traces")
    return 0


if __name__ == "__main__":
    sys.exit(main())
