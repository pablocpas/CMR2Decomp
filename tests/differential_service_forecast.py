#!/usr/bin/env python3
"""Compare service-park forecasts with the original, using the actual panel stage bytes.

The first stage lives inside the first panel at 0x82c710. Initialising a
separate symbol with the same original address would hide a broken alias.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                              UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_EIP)
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP

SAVED = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)


class Forecast(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address in (0x4075E0, 0x407520, 0x4075C0):
            self.callbacks[self.addr(address)] = (4, lambda a=address: self.provider(a))

    def provider(self, address):
        index, = self.args(1)
        self.calls.append((address, index))
        pointer = HEAP + {0x4075E0: 0x1000, 0x407520: 0x2000, 0x4075C0: 0x3000}[address]
        pointer += index * (8 if address == 0x407520 else 4)
        for register, value in ((UC_X86_REG_EAX, pointer), (UC_X86_REG_ECX, 0xA5A5A5A5),
                                (UC_X86_REG_EDX, 0x5A5A5A5A)):
            self.u.reg_write(register, value)

    def run(self, first, stages, missing, seed):
        self.u.mem_write(self.base, bytes(self.memory))
        rng = random.Random(seed)
        heap = bytearray(rng.randbytes(0x10000))
        pairs = [rng.randrange(9) for _ in range(20)]
        skills = [rng.randrange(9) for _ in range(20)]
        for i in range(20):
            struct.pack_into('<2i', heap, 0x2000 + i * 8, pairs[i], pairs[i])
            struct.pack_into('<i', heap, 0x1000 + i * 4, skills[i])
            struct.pack_into('<i', heap, 0x3000 + i * 4, missing)
        self.u.mem_write(HEAP, bytes(heap))
        self.u.mem_write(STACK, bytes([seed & 255]) * 0x10000)
        panels = bytearray(rng.randbytes(4 * 0x50))
        for i in range(stages):
            panels[i * 0x50 + 0x48] = first + i
        # Use the table's storage, never addr(0x82c710), even if it is annotated.
        self.u.mem_write(self.addr(0x82C6C8), bytes(panels))
        self.put(self.addr(0x82C694), '<i', stages)
        self.u.mem_write(self.addr(0x82CA04), b'\xcc' * 20)
        self.put(self.addr(0x82CA18), '<B', 0xCC)
        expected = bytes([100] * (stages + 1) if missing else
                         [pairs[first], *skills[first:first + stages]])
        self.calls = []
        saved = [0x13579BDF + i for i in range(len(SAVED))]
        for register, value in zip(SAVED, saved):
            self.u.reg_write(register, value)
        sp = STACK + 0xFF00
        self.put(sp, '<I', STOP)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.u.emu_start(self.addr(0x505E70), STOP, count=10000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 4, 'stdcall cleanup'
        assert [self.u.reg_read(r) for r in SAVED] == saved, 'callee-saved registers'
        assert self.read(self.addr(0x82CA18), 1) == bytes([stages + 1]), ('forecast count', first, stages)
        output = self.read(self.addr(0x82CA04), 20)
        assert output == expected + b'\xcc' * (20 - len(expected)), ('forecast values/guards', first, stages)
        assert self.read(self.addr(0x82C6C8), len(panels)) == panels, 'panel guards'
        assert self.read(HEAP, len(heap)) == heap, 'provider record guards'
        assert self.calls == [(0x4075E0, 0), (0x407520, 0)] + [(0x4075C0, first)] * (stages + 1)
        return output, self.calls


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    rebuilt = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe'
    original = Forecast(ROOT / 'cmr2bin/CMR2.exe')
    candidate = Forecast(rebuilt, entities)
    count = 0
    # Issue #30 first, followed by every stage and remaining stage count.
    cases = [(6, 2, 0, 0)] + list(itertools.product(range(11), range(1, 5), (0, 1), range(4)))
    for first, stages, missing, seed in cases:
        if first + stages > 11:
            continue
        expected = original.run(first, stages, missing, seed)
        assert candidate.run(first, stages, missing, seed) == expected
        count += 1
    print(f'{count} service forecasts: original/rebuilt/model agree, panel alias, values, counts, guards and ABI')


if __name__ == '__main__':
    main()
