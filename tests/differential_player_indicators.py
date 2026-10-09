#!/usr/bin/env python3
"""Execute original/rebuilt player indicator dispatch and controller arithmetic.

Queries, controller mapping and camera actions are controlled leaves. The whole
indicator body runs. Independent signed arithmetic checks guarded car/device
memory, merged buttons, preserved controls, and camera/provider trace parity.
"""
import itertools
import json
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_fixed_matrix_ops import FixedOps
from differential_menu_list import ROOT, HEAP, STACK
from differential_auto_steering import signed

CAR, DEVICE, SOURCE, RECORDS = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x4000, HEAP + 0x5000


def intensity(raw):
    # The original keeps the middle 32 product bits, then shifts that signed
    # result. Wrapping before the final shift matters for large axis values.
    return signed((signed(abs(raw)) * 0x3f0000) >> 16) >> 16


def separate_axis(raw):
    half = abs(raw) // 2 * (-1 if raw < 0 else 1)
    return (63 - intensity(signed(half + 32768))) & 255


class Indicators(FixedOps):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x404f20, 0), (0x405e00, 0), (0x4074f0, 0),
                               (0x47bca0, 1), (0x49ead0, 1), (0x41b390, 0),
                               (0x44a130, 0), (0x41f3a0, 0), (0x422f50, 1),
                               (0x47b970, 1), (0x41f3d0, 1), (0x47bad0, 2),
                               (0x40c210, 2), (0x40be60, 1), (0x40be30, 1),
                               (0x4063f0, 1)):
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        args = list(self.args(nargs))
        if address in (0x422f50, 0x41f3d0):
            args[0] &= 255
        if address == 0x40be60:
            args[0] &= 65535
        self.trace.append((address, tuple(args)))
        value = 0
        if address == 0x4074f0:
            value = 4
        elif address == 0x49ead0:
            value = DEVICE if args[0] == 3 else SOURCE
        elif address == 0x41b390:
            value = RECORDS
        elif address == 0x44a130:
            value = 1
        elif address == 0x41f3a0:
            value = self.multiplayer
        elif address == 0x422f50:
            value = self.camera
        elif address == 0x41f3d0:
            value = self.finished
        elif address == 0x40c210:
            value = self.axes[args[1]] & 0xffffffff
        elif address in (0x40be60, 0x40be30):
            value = self.inverted
        elif address == 0x4063f0:
            value = self.cheat
        self.u.reg_write(UC_X86_REG_EAX, value)
        self.u.reg_write(UC_X86_REG_ECX, 0xa5a5a5a5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5a5a5a5a)

    def run(self, player, state, multiplayer, camera, finished, kind, axes, raw, inverted, buttons, cheat,
            seed=0, enabled=(1, 1)):
        self.prepare(seed)
        self.u.mem_write(STACK, bytes([0xA5 if seed & 1 else 0]) * 0x10000)
        self.trace = []
        self.multiplayer, self.camera, self.finished = multiplayer, camera, finished
        self.axes, self.inverted, self.cheat = axes, inverted, cheat
        self.put(self.addr(0x58e0a0), '<I', CAR)
        self.put(self.addr(0x58e0a8), '<2B', *enabled)
        self.put(self.addr(0x51f4b0), '<8H', *(1 << i for i in range(8)))
        self.put(CAR + 0xb1a, '<b', player)
        self.put(CAR + 0xb9c, '<I', 1)
        self.put(DEVICE, '<4I', kind, buttons, buttons, buttons)
        source_buttons = (0, 1, 4, 8, 16, 32, 64, 128)[seed % 8]
        self.put(SOURCE, '<4I', 1, source_buttons, source_buttons, source_buttons)
        raw = [raw] * 4 if isinstance(raw, int) else list(raw)
        for i in range(4):
            self.put(DEVICE + (i * 5 + 0x11f) * 4, '<i', raw[i])
        self.put(RECORDS + 4, '<I', RECORDS + 0x100)
        self.put(RECORDS + 0x100 + player * 8, '<B', state)
        expected = bytearray(self.read(HEAP, 0x10000))
        car = CAR - HEAP

        def byte(offset, value):
            expected[car + offset] = value & 255

        def word(offset, value):
            struct.pack_into('<I', expected, car + offset, value)

        if kind != 1:
            buttons |= source_buttons
            struct.pack_into('<3I', expected, DEVICE - HEAP + 4, buttons, buttons, buttons)
        if not finished:
            steer, _, throttle, brake = axes
            steer = steer if enabled[0] else -1
            throttle, brake = (throttle, brake) if enabled[1] else (-1, -1)
            if kind in (2, 3):
                if steer >= 0:
                    word(0xB88, 1 if inverted else 2)
                    byte(0x1D0 + int(raw[steer] >= 0), intensity(raw[steer]))
                elif buttons & 1:
                    byte(0x1D0, 63)
                elif buttons & 2:
                    byte(0x1D1, 63)
                if throttle >= 0:
                    word(0xB8C, 1)
                    if throttle == brake:
                        accelerating = (raw[throttle] > 0) == bool(inverted)
                        if not accelerating:
                            word(0xB90, 1)
                        byte(0x1D2 + int(not accelerating), intensity(raw[throttle]))
                    else:
                        byte(0x1D2, separate_axis(raw[throttle]))
                else:
                    word(0xB8C, 0)
                    if buttons & 4:
                        byte(0x1D2, 63)
                if brake >= 0:
                    if brake != throttle:
                        word(0xB90, 1)
                        value = separate_axis(raw[brake])
                        if value:
                            byte(0x1D3, value)
                else:
                    word(0xB90, 0)
                    if buttons & 8:
                        byte(0x1D3, 63)
            else:
                for offset in (0xB88, 0xB8C, 0xB90):
                    word(offset, 0)
                for mask, offset in ((4, 0x1D2), (8, 0x1D3)):
                    if buttons & mask:
                        byte(offset, 63)
                if buttons & 1:
                    byte(0x1D0, 63)
                elif buttons & 2:
                    byte(0x1D1, 63)
            if buttons & 16:
                word(0x1D8, 1)
            if buttons & 32:
                byte(0x1D4, 1)
            if buttons & 64:
                byte(0x1D4, 255)
        if cheat:
            expected[car + 0x1D0], expected[car + 0x1D1] = expected[car + 0x1D1], expected[car + 0x1D0]
        self.allowed_globals = set()
        self.checked_invoke(0x47b0e0, [player, 3])
        after = self.read(HEAP, 0x10000)
        assert after == expected, ('indicator model/guards', player, kind, axes, raw, seed,
                                   [hex(i-car) for i, (a,b) in enumerate(zip(after, expected)) if a!=b][:16])
        return after, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Indicators(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Indicators(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    cases = []
    # Camera checks use the spectator view slot in multiplayer, even for player 0.
    for player, multiplayer, camera, state in itertools.product(range(3), range(2), (4, 7, 10), (0, 10)):
        cases.append((player, state, multiplayer, camera, 0, 1, (-1, -1, -1, -1), 0, 0, 128, 0))
    for kind, axes, raw, inverted, buttons, cheat in itertools.product(
            (1, 2, 3), ((0, -1, 1, 2), (0, -1, 1, 1), (-1, -1, -1, -1)),
            (-65536, -32769, -1, 0, 1, 32769, 65536), range(2), (0, 127), range(2)):
        cases.append((0, 0, 0, 4, 0, kind, axes, raw, inverted, buttons, cheat))
    # Independent axis values, preserved nonzero controls, missing bindings,
    # merged source-device buttons, disabling axes and completed racers.
    limits = (-0x80000000, -65537, -65536, -32769, -1, 0, 1, 32769,
              65536, 65537, 0x7fffffff)
    bindings = ((0, -1, 1, 2), (0, -1, 1, 1), (-1, -1, 1, 2),
                (0, -1, -1, 2), (0, -1, 1, -1), (-1, -1, -1, -1))
    for seed, axes, inverted, finished, enabled in itertools.product(
            range(len(limits)), bindings, range(2), range(2), ((1, 1), (0, 1), (1, 0))):
        raw = tuple(limits[(seed + i * 3) % len(limits)] for i in range(4))
        cases.append((seed % 3, seed % 2 * 10, seed & 1, (4, 8, 10)[seed % 3],
                      finished, 2 + seed % 2, axes, raw, inverted,
                      (0, 1, 2, 16, 32, 64, 127, 128)[seed % 8], seed & 1,
                      seed, enabled))
    for case in cases:
        a, b = original.run(*case), rebuilt.run(*case)
        if a != b:
            print('FAIL player indicators', case)
            for x, y in itertools.zip_longest(a[1], b[1]):
                if x != y:
                    print('first different call:', x, y)
                    break
            print('car state equal:', a[0] == b[0])
            return 1
    mutant = Indicators(ROOT / 'cmr2bin/CMR2.exe')
    memory = bytearray(mutant.memory)
    at = 0x47B36F - mutant.base
    assert memory[at:at + 5] == bytes.fromhex('ba00003f00')
    memory[at + 3] = 0x3E
    mutant.memory = bytes(memory)
    try:
        mutant.run(0, 0, 0, 4, 0, 2, (0, -1, -1, -1), 65536, 0, 0, 0)
    except AssertionError as error:
        assert 'indicator model/guards' in str(error), error
    else:
        raise AssertionError('wrong indicator scale survived')
    print(f'{len(cases)} player indicator cases: independent signed model, full heap/guards, '
          'preserved controls, device merging, camera/provider parity and ABI; scale mutation rejected')
    return 0


if __name__ == '__main__':
    sys.exit(main())
