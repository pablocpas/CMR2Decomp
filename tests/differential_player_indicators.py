#!/usr/bin/env python3
"""Execute original/rebuilt player indicator dispatch and controller arithmetic.

Queries and camera actions are controlled leaves; the complete indicator body
runs, including axis conversion, button masks and multiplayer camera selection.
"""
import itertools
import json
from pathlib import Path
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK

CAR, DEVICE, RECORDS = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x5000


class Indicators(Lighting):
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
            value = DEVICE
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

    def run(self, player, state, multiplayer, camera, finished, kind, axes, raw, inverted, buttons, cheat):
        self.reset()
        self.multiplayer, self.camera, self.finished = multiplayer, camera, finished
        self.axes, self.inverted, self.cheat = axes, inverted, cheat
        self.put(self.addr(0x58e0a0), '<I', CAR)
        self.put(self.addr(0x58e0a8), '<2B', 1, 1)
        self.put(self.addr(0x51f4b0), '<8H', *(1 << i for i in range(8)))
        self.put(CAR + 0xb1a, '<b', player)
        self.put(CAR + 0xb9c, '<I', 1)
        self.put(DEVICE, '<4I', kind, buttons, buttons, buttons)
        for i in range(4):
            self.put(DEVICE + (i * 5 + 0x11f) * 4, '<i', raw)
        self.put(RECORDS + 4, '<I', RECORDS + 0x100)
        self.put(RECORDS + 0x100 + player * 8, '<B', state)
        before = self.read(HEAP, 0x10000)
        self.invoke(0x47b0e0, [player, 3])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff00 + 12
        after = self.read(HEAP, 0x10000)
        allowed = {*(CAR - HEAP + i for i in range(0x1d0, 0x1dc)),
                   *(CAR - HEAP + i for i in range(0xb88, 0xb94))}
        assert all(x == y or i in allowed for i, (x, y) in enumerate(zip(before, after)))
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
    print(f'{len(cases)} player indicator cases: identical controls and camera/provider calls')
    return 0


if __name__ == '__main__':
    sys.exit(main())
