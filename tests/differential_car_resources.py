#!/usr/bin/env python3
"""Car resource teardown: independent ownership/order model and guarded storage.

Scene searches and record getters run their actual bodies. Destruction, buffer
release and cache release are controlled leaves; their order and wheel objects
at destruction are checked, along with complete heap/image and the callee ABI.
"""
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ESI,
                              UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP)
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting

BUFFER_TABLES = (0x542984, 0x542904, 0x542944, 0x542884)


class Resources(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x4866a0, 0), (0x4ac620, 1),
                               (0x4aade0, 1), (0x4a5ba0, 0), (0x4b5380, 0)):
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs: self.release(a, n))

    def release(self, address, nargs):
        args = self.args(nargs)
        objects = ()
        if address == 0x4ac620 and args[0] in self.roots:
            objects = tuple(struct.unpack('<I', self.read(node + 12, 4))[0]
                            for node in self.roots[args[0]])
        self.trace.append((address, args, objects))
        self.u.reg_write(UC_X86_REG_EAX, 1)

    def teardown(self, seed):
        self.reset()
        rnd = random.Random(seed)
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.roots = {}
        expected_trace = [(0x4866a0, (), ())]
        heap = bytearray(self.read(HEAP, 0x10000))
        table = bytearray(rnd.randbytes(16 * 0x24))

        def present():
            return False if seed == 0 else True if seed == 1 else bool(rnd.getrandbits(1))

        extras = [HEAP + 0xf000 + j * 0x100 if present() else 0 for j in range(2)]
        alternate = []
        for i in range(16):
            root = HEAP + i * 0xa00
            wheels = [root + (j + 1) * 0x200 for j in range(4)]
            # Genuine flags-based sibling tree, exercising the actual search.
            for node in [root] + wheels:
                self.put(node, '<3I', 0, 0, 0)
                self.put(node + 0x30, '<I', 0xfd)
            self.put(root + 4, '<I', wheels[0])
            originals = []
            current = []
            for j, node in enumerate(wheels):
                self.put(node, '<I', wheels[j + 1] if j < 3 else 0)
                self.put(node + 0x30, '<I', j + 1)
                current.append(struct.unpack('<I', self.read(node + 12, 4))[0])
                originals.append(HEAP + 0xc000 + i * 16 + j * 4 if present() else 0)
            self.roots[root] = wheels
            root_value = root if present() else 0
            body = HEAP + 0xd000 + i * 16 if present() else 0
            wheel_scene = HEAP + 0xd400 + i * 16 if present() else 0
            alternate.append(HEAP + 0xd800 + i * 16 if present() else 0)
            struct.pack_into('<7I', table, i * 0x24 + 4,
                             body, root_value, wheel_scene, *originals)
            if root_value:
                restored = tuple(old or value for old, value in zip(originals, current))
                expected_trace.append((0x4ac620, (root,), restored))
            for ptr in (wheel_scene, alternate[-1], body):
                if ptr:
                    expected_trace.append((0x4ac620, (ptr,), ()))
            if i == 0:
                for ptr in extras:
                    if ptr:
                        expected_trace.append((0x4ac620, (ptr,), ()))
        # Capture initialized tree bytes, then model only the restoration stores.
        heap[:] = self.read(HEAP, 0x10000)
        for i in range(16):
            record = struct.unpack_from('<7I', table, i * 0x24 + 4)
            if record[1]:
                for node, saved in zip(self.roots[record[1]], record[3:]):
                    if saved:
                        struct.pack_into('<I', heap, node - HEAP + 12, saved)
        self.u.mem_write(self.addr(0x542630), bytes(table))
        self.put(self.addr(0x5428c4), '<16I', *alternate)
        for address, ptr in zip((0x547fec, 0x547ff0), extras):
            self.put(self.addr(address), '<I', ptr)
        buffers = {a: [HEAP + 0xe000 + k * 0x100 + i * 4 if present() else 0
                       for i in range(16)] for k, a in enumerate(BUFFER_TABLES)}
        info = [HEAP + 0xe800 + i * 4 if present() else 0 for i in range(8)]
        for a, values in buffers.items():
            self.put(self.addr(a), '<16I', *values)
        self.put(self.addr(0x5429c8), '<8I', *info)
        for i in range(16):
            for a in BUFFER_TABLES:
                if buffers[a][i]:
                    expected_trace.append((0x4aade0, (buffers[a][i],), ()))
        expected_trace.extend((0x4aade0, (ptr,), ()) for ptr in info if ptr)
        expected_trace.extend(((0x4a5ba0, (), ()), (0x4b5380, (), ())))
        image = bytearray(self.read(self.base, self.size))
        for a, size in [(a, 64) for a in BUFFER_TABLES] + [
                (0x5429c8, 32), (0x547fec, 4), (0x547ff0, 4)]:
            offset = self.addr(a) - self.base
            image[offset:offset + size] = bytes(size)
        # All 16 getters must select the same record layout before teardown.
        for i in range(16):
            self.invoke(0x456be0, [i])
            assert self.u.reg_read(UC_X86_REG_EAX) == self.addr(0x542630) + i * 0x24
        saved = {reg: 0x13579bdf + n * 0x11111111 for n, reg in enumerate(
            (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP))}
        for reg, value in saved.items():
            self.u.reg_write(reg, value)
        self.trace = []
        self.invoke(0x457ed0, [])
        assert self.u.reg_read(UC_X86_REG_EAX) == 1
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xff04
        assert all(self.u.reg_read(reg) == value for reg, value in saved.items())
        assert self.trace == expected_trace, (seed, 'release order/restoration')
        assert self.read(HEAP, 0x10000) == heap, (seed, 'heap/guards')
        assert self.read(self.base, self.size) == image, (seed, 'global/guards')
        return self.trace, self.read(HEAP, 0x10000), self.read(self.addr(0x542630), 0x240)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Resources(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Resources(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    for seed in range(128):
        assert original.teardown(seed) == rebuilt.teardown(seed), seed
    print('128 car resource teardowns: 16 records, real search/getters, ownership order, wheel restoration, full heap/image guards and ABI')
    return 0


if __name__ == '__main__':
    sys.exit(main())
