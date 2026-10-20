#!/usr/bin/env python3
"""Collision face decision, records and provider ABI against an independent model.

Usage: differential_collision_face_decision.py entities.json [rebuilt.exe]
The deciding entry uses controlled face selection, box construction and vertex
classification. It exercises unsigned count boundaries and both selection paths.
A second path executes the complete vertex classifier, including direction
refresh and fixed-point helpers, against original code and an independent
segment/plane model. It guards geometry, global writes, candidates and ABI.
"""
import itertools
import json
import math
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting
from differential_stage_lighting import mul, signed
from differential_car_info_records import Records, put


class Classifier(Records):
    """Execute the geometry classifier; only the deciding wrapper uses leaves."""

    def classify(self, direction, pattern, shift, secondary, backside, dirty):
        h = self.begin(37)
        face = HEAP + 0x5000
        target, start = (0, 8192, 65536), (0, -8192, -65536)
        patterns = (
            ((-65536, 0, -32768), (65536, 0, -32768),
             (65536, 0, 32768), (-65536, 0, 32768)),
            ((0, 0, -65536), (0, 1, 65536),
             (-1, -8192, -196608), (1, 16384, 196608)),
            ((0x7fffffff, -32768, 0), (-0x80000000, 32768, 0),
             (-32768, -1, 65536), (32768, 1, -65536)),
        )
        primary = patterns[pattern]
        secondary_points = tuple((signed(x + shift), y, signed(z - shift))
                                 for x, y, z in primary)
        for index, point in enumerate(primary + secondary_points):
            put(h, face + 0x30 + 12 * index, '<3i', *point)
        put(h, face + 0x2c, '<i', secondary)
        self.put(self.addr(0x591984), '<I', face)
        for address, value in ((0x591910, direction), (0x591ae0, target), (0x591ac0, start)):
            self.put(self.addr(address), '<3i', *value)
        self.put(self.addr(0x59192c), '<i', backside)
        self.put(self.addr(0x591adc), '<i', dirty)
        self.put(self.addr(0x6e0ef4), '<4096H',
                 *(math.isqrt((8 + 16 * i) * 65536) for i in range(4096)))
        counters = (0x591934, 0x591935, 0x591944, 0x591945)
        for address in counters:
            self.put(self.addr(address), '<B', 0xa5)
            self.allowed.add((self.addr(address), 1))
        for address in (0x591988, 0x59198c):
            self.put(self.addr(address), '<4B', 0xf1, 0xf2, 0xf3, 0xf4)
            self.allowed.update((self.addr(address) + i, 1) for i in range(4))
        self.allowed.update((self.addr(0x5919a4) + i * 4, 4) for i in range(4))
        self.allowed.update((self.addr(0x591910) + i * 4, 4) for i in range(3))
        self.allowed.add((self.addr(0x591adc), 4))
        assert self.invoke_guarded(0x490720, []) == bytes(h), 'classifier changed input geometry'
        normal = struct.unpack('<3i', self.read(self.addr(0x591910), 12))
        if not dirty:
            assert normal == direction, 'clean direction changed'
        assert self.read(self.addr(0x591adc), 4) == bytes(4)

        # An independent segment/plane model: retain vertices whose tangent
        # coordinate lies between the endpoints, then reject a selected-side
        # point only when its secondary counterpart lies farther from the plane.
        tangent = (signed(-normal[2]), 0, normal[0])
        def projection(endpoint, point, axis):
            return signed(sum(mul(signed(a - b), n) for a, b, n in zip(endpoint, point, axis)))
        distances = [projection(target, p, normal) for p in primary]
        candidates = {False: [], True: []}
        for i, (point, distance) in enumerate(zip(primary, distances)):
            t = projection(target, point, tangent)
            s = projection(start, point, tangent)
            if t * s > 0:
                continue
            positive = distance >= 0
            if secondary and positive != bool(backside):
                other = projection(target, secondary_points[i], normal)
                if (distance > 0 and other > distance) or (distance < 0 and other < distance):
                    continue
            candidates[positive].append(i)
        expected_counts = (sum(d < 0 for d in distances), sum(d >= 0 for d in distances),
                           len(candidates[False]), len(candidates[True]))
        assert tuple(self.read(self.addr(a), 1)[0] for a in counters) == expected_counts
        assert struct.unpack('<4i', self.read(self.addr(0x5919a4), 16)) == tuple(distances)
        for positive, address in ((False, 0x591988), (True, 0x59198c)):
            expected = bytearray((0xf1, 0xf2, 0xf3, 0xf4))
            expected[:len(candidates[positive])] = candidates[positive]
            assert self.read(self.addr(address), 4) == bytes(expected), 'candidate order/tail guard'
        return (normal, expected_counts, tuple(distances),
                self.read(self.addr(0x591988), 4) + self.read(self.addr(0x59198c), 4))


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
    left = Classifier(ROOT / 'cmr2bin/CMR2.exe')
    right = Classifier(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    geometry_count = 0
    directions = ((65536, 0, 0), (-65536, 0, 0), (0, 0, 65536), (0, 0, 0))
    for case in itertools.product(directions, range(3), (-65536, 0, 65536), (0, 1), (0, 1), (0, 1)):
        assert left.classify(*case) == right.classify(*case), ('complete geometry', case)
        geometry_count += 1
    print(f"{count} face-decision fixtures and {geometry_count} complete vertex-classification cases: independent models, guarded memory and ABI; real classifier geometry/fixed-point helpers")
    return 0


if __name__ == "__main__":
    sys.exit(main())
