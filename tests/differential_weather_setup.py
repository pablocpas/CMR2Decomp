#!/usr/bin/env python3
"""Compare weather setup state, untouched fields and scene-provider arguments.

Usage: differential_weather_setup.py entities.json [rebuilt.exe]
Runs the complete setup entry with controlled option and subsystem providers.
Poisoned stack bytes exercise its overlapping DWORD reads when packing RGB.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_menu_list import ROOT, STACK
from differential_stage_lighting import Lighting


class WeatherSetup(Lighting):
    REGIONS = ((0x543DA0, 0x90), (0x543ED0, 0x20), (0x543F00, 0x20),
               (0x547908, 0x20), (0x543EF8, 4), (0x543D50, 4),
               (0x547950, 0x178), (0x5477F8, 12))
    PROVIDERS = ((0x45F260, 0, 0), (0x405CD0, 0, 0), (0x45EEA0, 0, 0),
                 (0x45EED0, 0, 0), (0x45F240, 0, 0), (0x463070, 0, 0),
                 (0x462CB0, 1, 4), (0x492E30, 1, 12), (0x4B5760, 1, 12),
                 (0x4B3F20, 0, 0))

    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        for address, nargs, size in self.PROVIDERS:
            self.callbacks[self.addr(address)] = (4 * nargs,
                lambda a=address, n=nargs, s=size: self.setup_provider(a, n, s))

    def setup_provider(self, address, nargs, size):
        args = self.args(nargs)
        if size:
            if address == 0x4B5760:
                assert args[0] == self.addr(0x5477F8), "wrong shadow-light vector"
            args = (self.read(args[0], size),)
        self.trace.append((address, args))
        for register, value in ((UC_X86_REG_EAX, self.quality if address == 0x405CD0 else 0x12345678),
                                (UC_X86_REG_ECX, 0xA5A5A5A5), (UC_X86_REG_EDX, 0x5A5A5A5A)):
            self.u.reg_write(register, value)

    def run_setup(self, seed, quality, stack_byte):
        self.reset()
        self.quality = quality
        rnd = random.Random(seed)
        self.u.mem_write(STACK, bytes([stack_byte]) * 0x10000)
        expected = {}
        for address, size in self.REGIONS:
            data = rnd.randbytes(size)
            self.u.mem_write(self.addr(address), data)
            expected[address] = bytearray(data)
        struct.pack_into('<I', expected[0x543EF8], 0, 1)
        struct.pack_into('<I', expected[0x543D50], 0,
                         0x10000 if quality == 0 else 0x4000 if quality == 1 else 0)
        for offset, value in ((0x20, 0x969696), (0x24, 0xFF000000),
                              (0x28, 0x3F000000), (0x2C, 0),
                              (0x50, 0x1E969696), (0x54, 0xFF000000),
                              (0x58, 0x3F600000), (0x5C, 0x3F7FFF58),
                              (0x80, 0x1E969696), (0x84, 0xFF000000),
                              (0x88, 0x3E000000), (0x8C, 0x3F7FFF58)):
            struct.pack_into('<I', expected[0x543DA0], offset, value)
        for address, extents, colour in (
                (0x543F00, (-0x2666, 0x2666, 0x2666, -0x2666), b'\xff\xff\xff\xfe'),
                (0x543ED0, (-0xA3D, 0x11EB, 0xA3D, -0x11EB), b'\xaa' * 4),
                (0x547908, (-0xF5C, 0xF5C, 0xF5C, -0xF5C), b'\xaa' * 4)):
            struct.pack_into('<4i', expected[address], 0xC, *extents)
            expected[address][0x1C:0x20] = colour
        lighting = expected[0x547950]
        ambient = bytes((struct.unpack_from('<I', lighting, i * 4)[0] >> 16) & 0xFF
                        for i in (0x12, 0x13, 0x14)) + b'\xff'
        provider_args = {0x462CB0: (ambient,), 0x492E30: (bytes(lighting[0x54:0x60]),),
                         0x4B5760: (bytes(expected[0x5477F8]),)}
        expected_trace = [(address, provider_args.get(address, ()))
                          for address, _, _ in self.PROVIDERS]
        self.invoke(0x45ECA0, [])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 4, "wrong stack cleanup"
        actual = {}
        for address, size in self.REGIONS:
            actual[address] = self.read(self.addr(address), size)
            assert actual[address] == expected[address], (seed, quality, stack_byte, hex(address))
        assert self.trace == expected_trace, (seed, quality, stack_byte, "provider arguments/order")
        return actual, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = WeatherSetup(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = WeatherSetup(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = 0
    for case in itertools.product(range(32), (0, 1, 2, 3, 255, 0xFFFFFFFF), (0, 0xA5, 0xFF)):
        assert original.run_setup(*case) == rebuilt.run_setup(*case), case
        cases += 1
    print(f'{cases} weather setup cases: identical complete records, untouched fields and scene calls; '
          'independent model and poisoned stack')
    return 0


if __name__ == '__main__':
    sys.exit(main())
