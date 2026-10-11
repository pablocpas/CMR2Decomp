#!/usr/bin/env python3
"""Original/rebuilt option-panel point transforms, including aliased outputs.

The result is two short output coordinates; callers consume no return value.
Check signed fixed-point arithmetic, full heap guards and the stdcall ABI.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP, UC_X86_REG_ESP)
from differential_stage_lighting import Lighting, ROOT, HEAP, STACK, mul, signed

SAVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


def trunc(a, b):
    q = abs(a) // abs(b)
    return -q if (a < 0) != (b < 0) else q


def short(value):
    return (value + 32768) % 65536 - 32768


class Points(Lighting):
    def run(self, seed, resolution, current, alias):
        self.reset()
        rng = random.Random(seed)
        heap = bytearray(rng.randbytes(0x10000))
        panel, graphics, xp = HEAP + 0x1000, HEAP + 0x2000, HEAP + 0x4000
        yp = xp if alias else xp + 2
        viewport = (-120, -35, 575, 320)
        dest = (-75, 42, 381, 215)
        struct.pack_into('<2I', heap, graphics - HEAP, *resolution)
        struct.pack_into('<4h', heap, panel - HEAP + 0xc, *dest)
        struct.pack_into('<i', heap, panel - HEAP + 0x1c, current)
        struct.pack_into('<2h', heap, xp - HEAP, rng.randrange(-500, 600), rng.randrange(-500, 600))
        self.u.mem_write(HEAP, bytes(heap))
        self.put(self.addr(0x520b74), '<I', graphics)
        self.put(self.addr(0x82c9ec), '<4h', *viewport)
        if current != 0x10000:
            for axis, pointer in enumerate((xp, yp)):
                value = struct.unpack_from('<h', heap, pointer - HEAP)[0]
                ratio = signed(trunc(signed((value - viewport[axis]) << 16) * 65536, viewport[axis + 2] << 16))
                value = short((mul(ratio, dest[axis + 2] << 16) >> 16) + dest[axis])
                struct.pack_into('<h', heap, pointer - HEAP, value)
        for axis, pointer in enumerate((xp, yp)):
            value = struct.unpack_from('<h', heap, pointer - HEAP)[0]
            value = short(trunc(signed(value * resolution[axis]), (640, 480)[axis]))
            # The alias observes each previous store, including X scaling.
            struct.pack_into('<h', heap, pointer - HEAP, value)
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rng.randrange(1 << 32))
        saved = [self.u.reg_read(r) for r in SAVED]
        self.invoke(0x503b70, [panel, xp, yp])
        assert self.read(HEAP, 0x10000) == bytes(heap), ('point/guard model', seed)
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff10, 'stdcall cleanup'
        assert [self.u.reg_read(r) for r in SAVED] == saved, 'callee-saved registers'
        return self.read(HEAP, 0x10000)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    rebuilt = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe'
    implementations = [Points(ROOT / 'cmr2bin/CMR2.exe'), Points(rebuilt, entities)]
    cases = itertools.product([(640, 480), (800, 600), (1024, 768), (1920, 1080)],
                              [0, 1, 16384, 32768, 49152, 65535, 65536, 98304], [False, True], range(16))
    count = 0
    for seed, (resolution, current, alias, _) in enumerate(cases):
        result = [i.run(seed, resolution, current, alias) for i in implementations]
        assert result[0] == result[1]
        count += 1
    print(f'{count} option-panel points: original/rebuilt/model agree, full heap and ABI checked')


if __name__ == '__main__':
    main()
