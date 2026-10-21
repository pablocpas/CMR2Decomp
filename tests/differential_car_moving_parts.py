#!/usr/bin/env python3
"""Execute moving-part attachment and initialization in both complete images.

The initializer, visibility getter, node attachment and matrix getters are real;
the installed update callbacks are recorded but never invoked. Independent models
check the entire poisoned heap, global write widths and stdcall/callee-saved ABI.
"""
import itertools
import json
import math
from pathlib import Path
import struct
import sys

from differential_car_info_records import Records, put
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import signed, mul
from unicorn.x86_const import UC_X86_REG_EAX

CAR, ROOT_NODE, BODY_NODE = HEAP + 0x1000, HEAP + 0x2000, HEAP + 0x2200
NODES, STATES, SETS, CURRENT, SENTINEL = (HEAP + n for n in (0x2400, 0x4000, 0x9000, 0xC000, 0xE000))
CALLBACKS = (0x4814D0, 0x4816F0, 0x484310, 0x483570)
SQRT = [math.isqrt((8 + 16 * i) * 65536) for i in range(4096)]


def vector_length(values):
    total = sum((v * v) >> 16 for v in values) & 0xFFFFFFFF
    if not total:
        return 0
    exponent = total.bit_length() - 16
    exponent += exponent & 1
    shift = exponent + 4
    index = total >> shift if shift >= 0 else total << -shift
    root = SQRT[index]
    exponent //= 2
    return signed(root << exponent if exponent >= 0 else root >> -exponent)


def node(slot):
    return NODES + slot * 0x200


def state(slot, car):
    return STATES + slot * 0xD00 + car * 0x1A0


def read(heap, address, fmt):
    return struct.unpack_from(fmt, heap, address - HEAP)


def copy(heap, target, source, size):
    heap[target - HEAP:target - HEAP + size] = heap[source - HEAP:source - HEAP + size]


def attach_model(heap, slot, car):
    p = state(slot, car)
    put(heap, p, '<3I', node(slot), p + 0xC, p + 0xCC)
    copy(heap, p + 0xC, node(slot) + 0x98, 64)
    copy(heap, p + 0xCC, BODY_NODE + 0xD8, 64)
    put(heap, p + 0x10C, '<I', ROOT_NODE + 0x98)
    for axis in range(3):
        copy(heap, p + 0x17C + axis * 12, node(slot) + 0x98 + axis * 16, 12)


def init_model(heap, slot, car, kind, model):
    attach_model(heap, slot, car)
    p = state(slot, car)
    centre = read(heap, CURRENT + 0xB4 + model * 12, '<3i')
    extents = read(heap, CURRENT + 0x168 + model * 12, '<3i')
    x, y, z = extents
    if slot == 0:
        offset = (-x, y, 0)
    elif slot == 1:
        offset = (x, 0, -z) if kind in (9, 11) else (mul(x, 0x8000), y, 0)
    elif slot == 2:
        offset = {11: (0, y, -z), 9: (-x, 0, z), 8: (-x, -y, 0)}.get(kind, (x, y, 0))
    else:
        a, b = read(heap, CURRENT + 0x248, '<2i')
        offset = (0, y, 0) if kind == 8 else (0, -y, z if a > b else -z)
    put(heap, p + 0x150, '<B', slot * 16 + (3 if slot == 3 else 1))
    put(heap, node(slot) + 0x184, '<i', 0)
    put(heap, p + 0x110, '<I', CALLBACKS[slot])
    put(heap, p + 0x114, '<3i', *extents)
    put(heap, p + 0x120, '<3i', *(signed(a + b) for a, b in zip(centre, offset)))
    put(heap, p + 0x12C, '<3i', *centre)
    put(heap, p + 0x138, '<6i', *([0] * 6))
    put(heap, p + 0x154, '<i', 0)
    put(heap, p + 0x158, '<h', 0x288 if slot in (0, 2) else 0)
    put(heap, p + 0x15C, '<ii', vector_length(extents), 0)
    put(heap, p + 0x164, '<3i', 0, 0, 0)
    put(heap, p + 0x170, '<3i', 0x10E5, 0x1EB8, 0x10E5 if slot == 1 else 0x6CCC)


class MovingParts(Records):
    def setup(self, car, kind, side, seed):
        h = self.begin(seed)
        put(h, CAR + 0xB1A, '<bb', car, kind)
        put(h, CAR + 0x71C, '<2I', ROOT_NODE, BODY_NODE)
        self.put(self.addr(0x590D74), '<I', CAR)
        self.put(self.addr(0x590D78), '<I', CURRENT)
        self.put(self.addr(0x588B94), '<I', SETS)
        self.put(self.addr(0x590C20), '<I', SENTINEL)
        self.put(self.addr(0x590D7C), '<4I', *(STATES + slot * 0xD00 for slot in range(4)))
        self.put(self.addr(0x6E0EF4), '<4096H', *SQRT)
        self.models = [(seed + slot * 7) % 15 for slot in range(4)]
        for slot in range(4):
            self.put(self.addr(0x590B7C) + slot * 32 + car * 4, '<I', node(slot))
            self.put(self.addr(0x590C24) + slot * 8 + car, '<B', self.models[slot])
            put(h, state(slot, car), '<I', 0)
            put(h, SETS + car * 0x4D0 + 0x4B0 + slot * 4, '<i', 0)
        for model in range(15):
            put(h, CURRENT + 0xB4 + model * 12, '<3i', -0x21000 + model * 117, 0x42000 - model * 251, 0x31000 + model * 37)
            put(h, CURRENT + 0x168 + model * 12, '<3i', 0x4001 + model * 19, 0x8003 + model * 23, 0xC005 + model * 43)
        put(h, CURRENT + 0x248, '<2i', 0x5000 + side, 0x5000)
        return h

    def check(self, function, args, expected, selected, initialized=(), result=None):
        actual = bytearray(self.invoke_guarded(function, args))
        # The two images install the same four functions at different addresses.
        for slot, car in initialized:
            address = state(slot, car) + 0x110
            assert read(actual, address, '<I')[0] == self.addr(CALLBACKS[slot]), 'installed callback'
            put(actual, address, '<I', CALLBACKS[slot])
        assert actual == expected, (hex(function), 'heap/guards', next(
            ((hex(i), a, b) for i, (a, b) in enumerate(zip(actual, expected)) if a != b), None))
        assert self.read(self.addr(0x590C20), 4) == struct.pack('<I', selected), 'selected state / reverse order'
        if result is not None:
            assert self.u.reg_read(UC_X86_REG_EAX) == result, 'return value'
        return actual, selected

    def direct(self, initialize, slot, car, kind, side, present, occupied, seed):
        h = self.setup(car, kind, side, seed)
        self.put(self.addr(0x590B7C) + slot * 32 + car * 4, '<I', node(slot) if present else 0)
        put(h, state(slot, car), '<I', node(slot) if occupied else 0)
        selected = state(slot, car)
        self.put(self.addr(0x590C20), '<I', SENTINEL if initialize else selected)
        if initialize:
            self.allowed.add((self.addr(0x590C20), 4))
        expected, changed = bytearray(h), ()
        if present and not occupied:
            if initialize:
                init_model(expected, slot, car, kind, self.models[slot])
                changed = ((slot, car),)
            else:
                attach_model(expected, slot, car)
        return self.check(0x480E50 if initialize else 0x4813B0, [slot], expected, selected,
                          changed, None if initialize else int(present and not occupied))

    def wake(self, modes, car, kind, side, seed):
        h = self.setup(car, kind, side, seed)
        indices = (22, 20, 21, 18)
        thresholds = ((-0x80000000, -1, 0x8000, 0x7FFFFFFF) if seed & 1 else
                      (0x8000, 0x8000, 0x8000, 0x3333))
        self.put(self.addr(0x590C60), '<4B', *indices)
        self.put(self.addr(0x590C44), '<4i', *thresholds)
        initialized, selected = [], SENTINEL
        for slot, mode in enumerate(modes):
            # 0: eligible; 1: hidden; 2: occupied; 3: missing model node;
            # 4: exactly the threshold; 5: strictly below the threshold.
            hidden = (-1 if slot & 1 else 1) if mode == 1 else 0
            put(h, SETS + car * 0x4D0 + 0x4B0 + slot * 4, '<i', hidden)
            put(h, state(slot, car), '<I', node(slot) if mode == 2 else 0)
            self.put(self.addr(0x590B7C) + slot * 32 + car * 4, '<I', 0 if mode == 3 else node(slot))
            damage = (thresholds[slot] if mode == 4 else
                      max(-0x80000000, thresholds[slot] - 1) if mode == 5 else
                      min(0x7FFFFFFF, thresholds[slot] + 1))
            put(h, CURRENT + 0x240 + indices[slot] * 4, '<i', damage)
            if not hidden and mode != 2 and damage > thresholds[slot]:
                initialized.append(slot)
        expected = bytearray(h)
        successful = []
        for slot in reversed(initialized):
            selected = state(slot, car)
            if modes[slot] != 3:
                init_model(expected, slot, car, kind, self.models[slot])
                successful.append((slot, car))
        self.allowed.add((self.addr(0x590C20), 4))
        return self.check(0x480DE0, [], expected, selected, successful)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    rebuilt = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe'
    images = (MovingParts(ROOT / 'cmr2bin/CMR2.exe'), MovingParts(rebuilt, entities))
    cases = [('direct', args) for args in itertools.product(
        (0, 1), range(4), (0, 7), (0, 8, 9, 11), (-1, 0, 1), (0, 1), (0, 1))]
    modes = [(mode,) * 4 for mode in range(6)] + list(itertools.permutations((0, 1, 2, 3)))
    modes += [(0, 0, 0, 3), (3, 0, 0, 0), (4, 0, 5, 3), (0, 4, 0, 5)]
    cases += [('wake', (mode, car, kind, side)) for mode, car, kind, side in itertools.product(
        modes, (0, 7), (0, 8, 9, 11), (-1, 0, 1))]
    for i, (method, args) in enumerate(cases):
        try:
            results = [getattr(image, method)(*args, 7919 + i) for image in images]
            assert results[0] == results[1], 'original / rebuilt'
        except Exception as error:
            raise AssertionError((method, args, 7919 + i)) from error
    print(f'{len(cases)} moving-part cases: 0 differences; real attachment/initializer, all four slots, '
          'body variants, paired damage, signed thresholds, hidden/occupied/missing nodes, reverse order, '
          'callbacks, full heap guards, global widths and ABI checked')


if __name__ == '__main__':
    main()
