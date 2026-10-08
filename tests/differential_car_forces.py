#!/usr/bin/env python3
"""Execute the three original force bodies against a relocated MSVC6 object.

Usage: python3 tests/differential_car_forces.py path/to/Car.obj [--cases 300]
The tested body runs in full. Game settings and the subsequent corner-friction
and wheel-torque passes are intercepted; vector/matrix helpers and wheel-pair
balancing execute from the original image. No full link is needed for iteration.
"""
import argparse
import math
from pathlib import Path
import random
import struct
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import fastcmp as F
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from unicorn.x86_const import (
    UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
    UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_EIP,
    UC_X86_REG_ESP, UC_X86_REG_EFLAGS,
)

SCRATCH = 0x31000000
TARGETS = (0x4387a0, 0x43b100, 0x441500)


def object_body(obj, address, source='Car.cpp'):
    """Capture the same resolved instructions used by the score/diff tool."""
    buffers = []
    dis = F.dis
    def capture(code, at):
        buffers.append(bytes(code))
        return dis(code, at)
    F.dis = capture
    try:
        names, sizes = F.load_meta()
        _, _, _, unknown = F.compare(address, str(obj), str(ROOT / 'CMR2Decomp' / source),
                                     names[address], sizes[address])
        assert not unknown, unknown
    finally:
        F.dis = dis
    code = bytearray(buffers[-1])
    for instruction in F.md.disasm(code, address):
        # Internal relative branches move with the body; external calls do not.
        if instruction.bytes[0] == 0xe8:
            target = instruction.address + 5 + struct.unpack('<i', instruction.bytes[1:])[0]
            # These bodies have no internal calls. A longer rebuilt tyre body
            # can overlap the original address of its following callee.
            struct.pack_into('<i', code, instruction.address - address + 1,
                             target - (SCRATCH + instruction.address - address + 5))
    return bytes(code)


class Forces(Drawing):
    def __init__(self, address, code=None, stack_pattern='uniform', vectors='identity'):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.address = address
        self.code = code
        self.stack_pattern = stack_pattern
        self.vectors = vectors
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)
        self.callbacks = {
            0x405d90: (0, lambda: self.setting(self.seed % 3)),
            0x406940: (0, lambda: self.setting((self.seed // 3) % 2)),
            0x406950: (0, lambda: self.setting((self.seed // 6) % 3)),
            0x43a920: (4, lambda: self.trace.append(('corner-friction', self.args(1)))),
            0x43c640: (0, lambda: self.trace.append(('wheel-torques',))),
        }

    def setting(self, value):
        self.trace.append(('setting', value))
        self.u.reg_write(UC_X86_REG_EAX, value)

    def run(self, seed):
        self.seed = seed
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        poison = [0, 0x1000, -0x1000][seed % 3]
        if self.stack_pattern == 'uniform':
            stack = struct.pack('<i', poison) * (0x10000 // 4)
        else:
            stack_random = random.Random(seed ^ 0x41500)
            stack = struct.pack('<16384i',
                *(stack_random.randrange(-4096, 4097) for _ in range(0x10000 // 4)))
        self.u.mem_write(STACK, stack)
        self.put(0x6e0ef4, '<4096H',
                 *(int(math.sqrt((8 + 16*i) / 65536) * 65536) for i in range(4096)))
        self.put(0x6e2ef4, '<4096i',
                 *(int(math.sin(i * math.tau / 4096) * 65536) for i in range(4096)))
        # atan2 uses the original statically initialised table.
        car, setup, matrix = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x4000
        self.put(0x53cc18, '<I', car)
        self.put(0x53cc1c, '<I', setup)
        self.u.mem_write(car, bytes(0xc24))
        self.u.mem_write(setup, bytes(0x500))
        self.u.mem_write(matrix, bytes(0x40))
        self.put(car + 0x750, '<I', matrix)
        for off, vector in [(0x360, (65536, 0, 0)), (0x36c, (0, 65536, 0)),
                            (0x378, (0, 0, 65536)), (0x3a8, (65536, 0, 0)),
                            (0x3b4, (0, 0, 65536)), (0x3f0, (0, 0, 65536)),
                            (0x48c, (0, 65536, 0))]:
            if seed % 17 == 0 and off in (0x3a8, 0x3b4, 0x3f0):
                vector = (0, 0, 0)
            self.put(car + off, '<3i', *vector)
        for off, vector in [(0, (65536, 0, 0)), (12, (0, 65536, 0)),
                            (24, (0, 0, 65536))]:
            self.put(matrix + off, '<3i', *vector)
        if self.vectors in ('rotated', 'degenerate'):
            # Use a separate RNG so the remaining inputs are identical to the
            # identity-basis cases. Nonzero components exercise projection,
            # cross products and fixed-point rounding on all three axes.
            direction_random = random.Random(seed ^ 0x4387a0)
            yaw = direction_random.uniform(-math.pi, math.pi)
            tilt = direction_random.uniform(-0.35, 0.35)
            cy, sy, cz, sz = math.cos(yaw), math.sin(yaw), math.cos(tilt), math.sin(tilt)
            right, up, forward = (cy*cz, cy*sz, -sy), (-sz, cz, 0), (sy*cz, sy*sz, cy)
            fixed = lambda v: tuple(round(c * 65536) for c in v)
            for off, v in ((0x360, right), (0x36c, up), (0x378, forward),
                           (0x3a8, right), (0x3b4, forward), (0x3f0, forward)):
                self.put(car + off, '<3i', *fixed(v))
            slope = direction_random.uniform(-0.3, 0.3)
            self.put(car + 0x48c, '<3i', 0, round(math.cos(slope)*65536),
                     round(math.sin(slope)*65536))
            for off, v in ((0, right), (12, up), (24, forward)):
                self.put(matrix + off, '<3i', *fixed(v))
            if seed % 17 == 0:
                for off in (0x3a8, 0x3b4, 0x3f0):
                    self.put(car + off, '<3i', 0, 0, 0)
        self.put(car + 0x204, '<3i', 65536, 32768, 131072)
        for off in (0x408, 0x414, 0x6e4, 0x6f0):
            self.put(car + off, '<3i', *(rnd.randrange(-15000, 15001) for _ in range(3)))
        for i in range(8):
            self.put(car + 0x42c + 12*i, '<3i',
                     *(rnd.randrange(-0x40000, 0x40001) for _ in range(3)))
            self.put(car + 0xbac + 4*i, '<i', int((seed >> (i % 4)) & 1))
            self.put(car + 0x80 + 0x24*i, '<5i', 65536, rnd.randrange(32768, 131073),
                     65536, rnd.randrange(32768, 131073), rnd.choice([0, 4096, 32768]))
        for i in range(4):
            for off in (0x850, 0x860):
                self.put(car + off + 4*i, '<i', rnd.randrange(-0x80000, 0x80001))
            for off in (0xa4c, 0xa5c):
                self.put(car + off + 4*i, '<i', rnd.randrange(8192, 131073))
            self.put(car + 0x1a8 + 12*i, '<i', rnd.randrange(0, 16385))
            self.put(setup + 0x3ec + 4*i, '<i', rnd.randrange(0, 4097))
        for off, value in [(0x778, rnd.randrange(0, 65537)), (0x794, 0x100000),
                           (0x830, rnd.randrange(0, 65537)), (0x838, rnd.randrange(0, 65537)),
                           (0x848, 65536), (0x8b4, 65536), (0x9b8, 32768),
                           (0x9bc, 32768), (0x1d8, seed % 2),
                           (0xb60, seed % 2), (0xb74, int(seed % 5 != 0)),
                           (0xb84, seed % 3), (0x7b4, [0, 32768, 65536][seed % 3])]:
            self.put(car + off, '<i', value)
        gear = seed % 4
        self.put(car + 0xb1e, '<b', gear)
        self.put(car + 0xb1d, '<b', (seed % 19) - 9)
        self.put(car + 0xb1f, '<b', seed % 2)
        self.put(car + 0xb28, '<B', [0, 1, 4][seed % 3])
        self.put(car + 0x1d2, '<2B', seed % 2, (seed // 2) % 2)
        self.put(car + 0x7bc, '<8i', 0, 65536, -65536, 131072, 65536, 65536, 65536, 65536)
        self.put(car + 0x7dc, '<8i', *([65536] * 8))
        for off in (0x3fc, 0x400):
            self.put(setup + off, '<i', rnd.randrange(8192, 131073))
        self.put(setup + 0x3dc, '<i', 32768)
        self.put(0x519c8c, '<2i', [65536, 32768, 8192][seed % 3], 65536)
        self.put(0x53c9d8, '<3i', *(rnd.randrange(-20000, 20001) for _ in range(3)))
        if self.vectors == 'degenerate':
            # Disable one direction/axis at a time with all corners grounded.
            # This exposes values carried between corners when normalisation
            # fails, including the original's reused drag-force scalar.
            for off in ((0x3b4,), (0x3f0,), (0x3a8,), (0x3b4, 0x3a8),
                        (0x3f0, 0x3a8), (0x3b4, 0x3f0))[seed % 6]:
                self.put(car + off, '<3i', 0, 0, 0)
            self.put(car + 0xbac, '<4i', 1, 1, 1, 1)
            self.put(car + 0xb28, '<B', 1)
            self.put(car + 0xb74, '<i', 1)
        for register in (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                         UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP):
            self.u.reg_write(register, 0)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<I', STOP)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(SCRATCH if self.code is not None else self.address, STOP, count=200000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'body did not return'
        globals_ = b''.join(self.read(a, n) for a, n in
                           [(0x53c9b4, 4), (0x53c9b8, 12), (0x53c9e8, 12),
                            (0x53ca4c, 4), (0x53ca88, 16), (0x53cae0, 4), (0x53cc20, 4)])
        return self.read(HEAP, 0x10000), globals_, self.trace


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=300)
    parser.add_argument('--start-seed', type=int, default=0)
    parser.add_argument('--functions', default='tyre,suspension,wheel')
    parser.add_argument('--stack-pattern', choices=('uniform', 'mixed'), default='uniform',
                        help='Mixed values expose dependence on uninitialised stack locations')
    parser.add_argument('--vectors', choices=('identity', 'rotated', 'degenerate'), default='identity')
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    functions = set(args.functions.split(','))
    if not functions.issubset({'tyre', 'suspension', 'wheel'}):
        parser.error('--functions must contain tyre, suspension or wheel, separated by commas')
    # Freeze relocation resolution once. A full rebuild can otherwise delete
    # build/*.obj between bodies and invalidate a running differential test.
    symbols = dict(F.symmap_for(str(ROOT / 'CMR2Decomp/Car.cpp')))
    F.symmap_for = lambda source: symbols
    failed = False
    for label, address in zip(('tyre', 'suspension', 'wheel'), TARGETS):
        if label not in functions:
            continue
        original = Forces(address, stack_pattern=args.stack_pattern, vectors=args.vectors)
        rebuilt = Forces(address, object_body(args.object, address), args.stack_pattern, args.vectors)
        for seed in range(args.start_seed, args.start_seed + args.cases):
            a, b = original.run(seed), rebuilt.run(seed)
            if a != b:
                offsets = [hex(i - 0x1000) for i, (x, y) in enumerate(zip(a[0], b[0])) if x != y]
                print(f'FAIL {label} seed {seed} ({args.stack_pattern} stack, {args.vectors} vectors): car offsets {offsets[:24]}, globals {a[1] != b[1]}, trace {a[2] != b[2]}', flush=True)
                failed = True
                break
        else:
            print(f'PASS {label}: {args.cases} cases ({args.stack_pattern} stack, {args.vectors} vectors), guarded car/matrices, scratch globals and provider calls identical', flush=True)
    return int(failed)


if __name__ == '__main__':
    sys.exit(main())
