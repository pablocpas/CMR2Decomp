#!/usr/bin/env python3
"""Compare the complete car network packet, preserved bits and car side effects.

Usage: differential_net_car_state.py entities.json [rebuilt.exe]
Sequence, game-mode and route queries are controlled providers. Matrix reads,
fixed-point arithmetic and quantization execute their actual machine code.
This checks packing; transport and unpacking remain outside this harness.
"""
import itertools
import json
import math
from pathlib import Path
import random
import sys

from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP, UC_X86_REG_FPCW


class Packet(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        for address, cleanup in (
            (0x41D2A0, 0),
            (0x404F20, 0),
            (0x4582F0, 4),
            (0x4582D0, 4),
            (0x4209D0, 4),
        ):
            self.callbacks[self.addr(address)] = (
                cleanup,
                lambda a=address, n=cleanup // 4: self.provider(a, n),
            )
        self.put(
            self.addr(0x6E8FF4),
            "<512H",
            *(int(math.atan(i / 512) * 2048 / math.pi) for i in range(512)),
        )
        self.put(
            self.addr(0x6E6EF4),
            "<4096h",
            *(int(math.asin(i / 4095) * 2048 / math.pi) for i in range(4096)),
        )

    def provider(self, address, count):
        args = self.args(count)
        self.trace.append((address, args))
        self.u.reg_write(UC_X86_REG_EAX, self.values[address] & 0xFFFFFFFF)

    def run_packet(self, seed, rounding):
        r = random.Random(seed)
        self.u.mem_write(HEAP, r.randbytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        self.trace = []
        self.values = {
            0x41D2A0: r.randrange(0x100000),
            0x404F20: seed % 2,
            0x4582F0: (-1, 0, 1, 1023, 1024, 1025)[seed % 6],
            0x4582D0: (-16, -1, 0, 1, 15, 16)[seed % 6],
            0x4209D0: (-0x10000, 0, 1, 0x10000, 0x20000)[seed % 5],
        }
        car, matrix, sector = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x4000
        self.put(car + 0x750, "<I", matrix)
        self.put(car + 0xB00, "<h", 0)
        self.put(self.addr(0x71F608), "<I", sector)
        self.put(sector, "<3i", 0x1000000, -0x10000, -0x1000000)
        for offset in (0x2DC, 0x2E4, 0x2E8, 0x2F0, 0x960, 0x964):
            self.put(car + offset, "<i", r.randrange(-12 << 16, 12 << 16))
        for index in range(4):
            for offset in (
                0x1A4 + index * 12,
                0x1A8 + index * 12,
                0x88 + index * 36,
                0x8C + index * 36,
            ):
                self.put(car + offset, "<i", r.randrange(-0x10000, 0x10001))
        self.put(
            car + 0x420, "<3i", *(r.randrange(-0x10000, 0x10001) for _ in range(3))
        )
        self.put(car + 0x79C, "<i", seed % 2)
        self.put(car + 0xB10, "<h", (-500, -1, 0, 1, 500)[seed % 5])
        self.put(car + 0xB16, "<h", 500)
        self.put(car + 0xB1A, "<b", seed % 8)
        for offset in (0xB35, 0xB54, 0xC00):
            self.put(car + offset, "<B", seed % 2)
        self.put(car + 0xB45, "<B", seed % 4)
        for offset in (0, 0x20):
            vector = [r.randrange(-0x10000, 0x10001) for _ in range(3)]
            length = math.sqrt(sum(x * x for x in vector)) or 1
            vector = [int(x * 65536 / length) for x in vector]
            self.put(matrix + offset, "<3i", *vector)
        self.put(
            matrix + 0x30,
            "<3i",
            0x1000000 + r.randrange(-0x1000000, 0x1000000),
            r.randrange(-0x1000000, 0x1000000),
            -0x1000000 + r.randrange(-0x1000000, 0x1000000),
        )
        packet = self.addr(0x539388)
        initial = r.randbytes(62)
        self.u.mem_write(packet - 16, initial)
        self.u.reg_write(UC_X86_REG_FPCW, 0x27F | rounding)
        self.invoke(0x424F20, [car])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 8
        data = self.read(packet - 16, 62)
        assert data[:16] == initial[:16] and data[46:] == initial[46:]
        return data, self.read(HEAP, 0x10000), self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Packet(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Packet(
        Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities
    )
    count = 0
    for case in itertools.product(range(240), (0, 0x400, 0x800, 0xC00)):
        left, right = original.run_packet(*case), rebuilt.run_packet(*case)
        if left != right:
            print(
                "FAIL car state packet",
                case,
                "original packet",
                left[0].hex(),
                "rebuilt packet",
                right[0].hex(),
                "calls",
                left[2],
                right[2],
            )
            return 1
        count += 1
    print(
        f"{count} car state packets: identical 30 bytes, preserved bits, guarded heap and query traces in all four x87 rounding modes"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
