#!/usr/bin/env python3
"""Compare ambient/light/shadow colours and the D3D render-state call.

The complete setter and original shadow-colour helper execute. Only the COM
render-state method is intercepted; include unchanged RGB, alpha-only changes,
channel boundaries, shadow boost and an aliased input colour.
"""
import argparse
from pathlib import Path
import random

from differential_car_forces import object_body, SCRATCH
from differential_car_body_matrix import SAVED
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)

ADDRESS = 0x4b3740


class Ambient(Drawing):
    def __init__(self, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.code = code
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)
        self.callbacks = {STOP + 0x100: (12, self.render_state)}

    def render_state(self):
        self.trace.append(self.args(3))
        self.u.reg_write(UC_X86_REG_EAX, self.seed & 1)
        self.u.reg_write(UC_X86_REG_ECX, 0x12345678)
        self.u.reg_write(UC_X86_REG_EDX, 0x87654321)

    def run(self, seed):
        self.seed = seed
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        self.u.mem_write(0x6e0000, rnd.randbytes(0x1000))
        self.put(0x520b78, '<I', HEAP + 0x1000)
        self.put(HEAP + 0x1004, '<I', HEAP + 0x2000)
        self.put(HEAP + 0x2000, '<I', HEAP + 0x3000)
        self.put(HEAP + 0x3050, '<I', STOP + 0x100)
        edges = (0, 1, 127, 128, 254, 255)
        colour = [edges[(seed // (6**i)) % 6] for i in range(4)]
        self.put(HEAP, '<4B', *colour)
        previous = colour.copy() if seed % 3 else [rnd.randrange(256) for _ in range(4)]
        previous[3] = (previous[3] + 1) & 255
        self.put(0x6e01ec, '<4B', *previous)
        self.put(0x6e0244, '<i', seed % 2)
        self.put(0x6e01a0, '<3i', *(rnd.randrange(256) << 16 for _ in range(3)))
        pointer = 0x6e01ec if seed % 11 == 0 else HEAP
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<3I', STOP, pointer, (seed // 3) % 2)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(ADDRESS if self.code is None else SCRATCH, STOP, count=20000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'setter did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 12, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        assert len(self.trace) == 1 and self.trace[0][:2] == (HEAP + 0x2000, 139), self.trace
        return self.read(HEAP, 0x10000), self.read(0x6e0000, 0x1000), self.trace


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=1296)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    code = object_body(args.object, ADDRESS, 'SceneNode.cpp')
    original, rebuilt = Ambient(), Ambient(code)
    for seed in range(args.cases):
        if original.run(seed) != rebuilt.run(seed):
            print(f'FAIL scene ambient seed {seed}')
            return 1
    damaged = bytearray(code)
    at = damaged.find(bytes.fromhex('c1 e1 08'))
    assert at >= 0, 'ARGB packing shift missing'
    damaged[at + 2] = 7
    negative = Ambient(bytes(damaged))
    assert any(original.run(seed) != negative.run(seed) for seed in (3, 6, 9, 36, 216)), 'negative control passed'
    print(f'PASS scene ambient: {args.cases} cases, colours, shadow, D3D call, guarded memory and ABI identical; negative control detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
