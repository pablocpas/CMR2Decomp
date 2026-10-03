#!/usr/bin/env python3
"""Collision face decision, records and provider ABI against an independent model.

Usage: differential_collision_face_decision.py entities.json [rebuilt.exe]
Face selection, vertex transformation and classification are controlled leaves.
The complete deciding entry executes. Exercises unsigned count boundaries,
both back-side settings and selection branches, including failed selection.
Does not validate the controlled geometry helpers or complete car collision.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting


class Decision(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        self.initial_image = self.read(self.base, self.size)
        for address, nargs in ((0x490570, 0), (0x490640, 0), (0x486C30, 4), (0x490720, 0)):
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        args = self.args(nargs)
        self.trace.append((address, args))
        if address == 0x486C30:
            assert args == (HEAP + 0x5000, HEAP + 0x1360, HEAP + 0x12D0, HEAP + 0x1270)
            self.put(args[3], "<12i", *range(12))
        if address == 0x490720:
            for global_, value in zip((0x591945, 0x591944, 0x591935, 0x591934), self.counts):
                self.put(self.addr(global_), "<B", value)
        self.u.reg_write(UC_X86_REG_EAX, self.found & 0xFFFFFFFF if address in (0x490570, 0x490640) else 0x12345678)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def run_decision(self, mode, found, positive, negative, positive_vertices, negative_vertices, backside):
        self.u.mem_write(self.base, self.initial_image)
        self.u.mem_write(STACK, bytes(0x10000))
        rnd = random.Random(mode * 127 + positive * 13 + negative_vertices)
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.put(self.addr(0x5918DC), "<I", HEAP + 0x1000)
        self.put(self.addr(0x591984), "<I", HEAP + 0x5000)
        self.put(self.addr(0x59192C), "<i", backside)
        self.found = found
        self.counts = positive, negative, positive_vertices, negative_vertices
        self.trace = []
        before = self.read(HEAP, 0x10000)
        expected = bytearray(before)
        expected_trace = [(0x490640 if mode == 0 else 0x490570, ())]
        result = 0
        if found:
            struct.pack_into("<12i", expected, 0x1270, *range(12))
            expected_trace += [(0x486C30, (HEAP + 0x5000, HEAP + 0x1360, HEAP + 0x12D0, HEAP + 0x1270)),
                               (0x490720, ())]
            result = int((positive > 0 and negative > 0) or (
                (positive > 0 or negative > 0) and
                not (positive_vertices == 4 and backside == 0) and
                negative_vertices == 4 and backside != 0))
        self.invoke(0x490B90, [mode & 0xFFFFFFFF])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 8
        actual_result = self.u.reg_read(UC_X86_REG_EAX)
        assert actual_result == result, (mode, found, self.counts, backside, "decision mismatch")
        assert self.read(HEAP, 0x10000) == expected
        assert self.trace == expected_trace
        return actual_result, self.read(HEAP, 0x10000), self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Decision(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = Decision(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe", entities)
    count = 0
    for case in itertools.product((0, 1, -1), (0, 1, -1), (0, 1, 4, 255),
                                  (0, 1, 4, 255), range(5), range(5), (0, 1)):
        if original.run_decision(*case) != rebuilt.run_decision(*case):
            print("FAIL collision face decision", case)
            return 1
        count += 1
    print(f"{count} face-decision fixtures: independent decisions, guarded memory, provider arguments and stack cleanup; controlled geometry")
    return 0


if __name__ == "__main__":
    sys.exit(main())
