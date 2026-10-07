#!/usr/bin/env python3
"""Run the camera update followed by its stage deformation consumer.

Both full bodies, camera state copying and fixed-point helpers execute. Camera
placement/update and terrain queries are controlled leaves. Compare complete
event/node state and terrain traces, including the camera field shared at
0x538d7c (record +0x54), rather than initializing its consumer separately.
"""
import itertools
import json
import math
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import Lighting

SQRT = struct.pack('<4096H', *(math.isqrt((8 + 16 * i) * 65536) for i in range(4096)))
EVENT, NODE, CAR = HEAP + 0x1000, HEAP + 0x5000, HEAP + 0x8000


class Deformation(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x423b20, 1), (0x423460, 1), (0x42b5f0, 1),
                               (0x406910, 0), (0x406930, 0), (0x490c90, 5)):
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        args = self.args(nargs)
        value = 0
        if address == 0x42b5f0:
            self.trace.append(('car', args[0]))
            value = CAR
        elif address == 0x490c90:
            point, normal, tri, surface, previous = args
            x, y, z = struct.unpack('<3i', self.read(point, 12))
            self.trace.append(('ground', x, y, z, previous))
            self.put(normal, '<3i', 0, 0x10000, 0)
            self.put(tri, '<h', 2)
            self.put(surface, '<H', 3)
            value = ((x // 16 + z // 32) % 0x20000) - 0x10000
        elif address == 0x423b20:
            self.trace.append(('place', args[0] & 0xff))
        elif address == 0x423460:
            self.trace.append(('camera', (args[0] - self.addr(0x539018)) // 100))
        self.u.reg_write(UC_X86_REG_EAX, value & 0xffffffff)
        self.u.reg_write(UC_X86_REG_ECX, 0xa5a5a5a5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5a5a5a5a)

    def run(self, view, active, offset, kind, nodes, step, update, seed):
        self.reset()
        rng = random.Random(seed)
        self.u.mem_write(self.addr(0x6e0ef4), SQRT)
        self.put(self.addr(0x538d20) + view * 4, '<i', -1)
        self.put(self.addr(0x538e0c) + view, '<B', active)
        self.put(self.addr(0x538f00), '<2i', 0, 0)
        self.put(self.addr(0x538e2c) + view * 4, '<I', NODE)
        matrix = (0x10000, 0, 0, 0, 0, 0x10000, 0, 0,
                  0x8000, 0x1000, 0xe000, 0, -0x30000, 0x80000, 0x50000, 0x10000)
        self.put(NODE + 0x98, '<16i', *matrix)
        record = self.addr(0x539018) + (view * 2 + active) * 100
        self.put(record, '<4Bi', view * 2 + active, view, view, 0, 4)
        self.put(record + 8, '<16i', *matrix)
        self.put(record + 0x48, '<7i', 0, 0, 0, offset, 0, 0, 0)
        self.put(CAR + 0x8dc, '<4i', 0x10000, 0x18000, 0x20000, 0x28000)
        self.put(self.addr(0x51bd3c), '<i', step)
        self.put(self.addr(0x51bd40), '<i', 0x8000)
        self.put(self.addr(0x543da0), '<i', 0x10000)
        self.put(self.addr(0x547930), '<3i', 0x4000, 0x8000, -0xc000)
        self.put(self.addr(0x547940), '<i', 0x8000)
        self.put(EVENT, '<i', kind)
        self.put(EVENT + 0x58, '<3i', 0x8000, 0x10000 if update else 0, 0x9000)
        self.put(EVENT + 0x74, '<2h', nodes, 0)
        for i in range(nodes):
            values = [rng.randrange(-0x300000, 0x300000) for _ in range(3)]
            self.put(self.addr(0x543fb0) + i * 36, '<9i', *values,
                     0x10000, 0, 0x167ffff, 0x10000, 0, 0)
        self.invoke(0x4219b0, [view])
        camera = self.read(self.addr(0x538d2c), 196)
        self.invoke(0x45f9d0, [update, EVENT, view])
        return (camera, self.read(EVENT, 0x180),
                self.read(self.addr(0x543fb0), 36 * nodes), list(self.trace))


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    a = Deformation(ROOT / 'cmr2bin/CMR2.exe')
    b = Deformation(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0
    for case in itertools.product(range(2), range(2), (0xa000, 0, 1, 0x5000, 0x14000, -0xa000),
                                  (1, 2), (0, 1, 5), (0, 0x8000, 0x10000), range(2)):
        aa, bb = a.run(*case, count + 73), b.run(*case, count + 73)
        if aa != bb:
            print('FAIL', case, 'camera equal:', aa[0] == bb[0], 'event equal:', aa[1] == bb[1],
                  'nodes equal:', aa[2] == bb[2], 'trace equal:', aa[3] == bb[3])
            print('original event position:', struct.unpack('<3i', aa[1][8:20]))
            print('rebuilt event position:', struct.unpack('<3i', bb[1][8:20]))
            return 1
        count += 1
    print(count, 'camera/deformation lifecycles: complete event/node state and terrain traces identical')
    return 0


if __name__ == '__main__':
    sys.exit(main())
