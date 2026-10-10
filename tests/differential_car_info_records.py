#!/usr/bin/env python3
"""CIN directory, pointer caches, cockpit sweeps, camera offsets and archives.

Independent models execute complete original/rebuilt bodies with poisoned heap,
global widths, signed overflow, untouched records, provider order and stdcall
checks. The directory reader and camera fixed-point helpers are real. Lookup,
weather, random, matrix leaves of the wiper wrapper and releases are controlled.
The trailing [1] source views are not treated as fixed file capacities.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn import UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                              UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_FPCW)
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting, mul, signed

CAR, PROFILE, ORDER = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x5000
DIRECTORY, SECOND_DIRECTORY = HEAP + 0x6000, HEAP + 0x7000
NODE_A, NODE_B, CAMERA, REFERENCE = HEAP + 0x8000, HEAP + 0x9000, HEAP + 0xA000, HEAP + 0xB000
REGISTERS = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
LINE_TABLES = ((0x590B5C, 0), (0x590B30, 12), (0x590C00, 16))


def put(heap, address, fmt, *values):
    struct.pack_into(fmt, heap, address - HEAP, *values)


def short(value):
    return (value + 32768) % 65536 - 32768


def trunc(a, b):
    value = abs(a) // abs(b)
    return -value if (a < 0) != (b < 0) else value


def wiper_model(state, kind, intensity, maximum, draws):
    mode, forward, angle, step = state
    used = []

    def speed(base):
        value = draws[len(used)]
        used.append(value)
        return value % 11 + base

    if kind == 1:
        if intensity > 0x3333 and mode == 0:
            mode, step = 2, speed(45)
        if intensity > 0x8000 and mode == 2:
            mode, step = 3, speed(91)
        if intensity < 0x1999 and mode == 2:
            mode = 1
        if intensity < 0x6666 and mode == 3:
            mode, step = 2, speed(45)
    else:
        mode = 0
    if mode:
        angle = short(angle + step if forward else angle - step)
        if forward and angle > maximum:
            angle, forward = maximum, 0
        elif not forward and angle < 0:
            angle, forward = 0, 1
            if mode == 1:
                mode = 0
    return (mode, forward, angle, step), used


def rotation_leaf(angle):
    return tuple(signed(angle * (i + 3) + 271 * i) for i in range(16))


def multiply_leaf(left, right):
    return tuple(signed(left[i] + right[i] + i * 97) for i in range(16))


class Records(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.running, self.watched = False, None
        self.u.hook_add(UC_HOOK_MEM_WRITE, self.write_guard,
                        begin=self.base, end=self.base + self.size - 1)
        self.u.hook_add(UC_HOOK_MEM_READ, self.directory_read)

    def write_guard(self, u, access, address, size, value, data):
        if self.running:
            assert (address, size) in self.allowed, ('global write/width', hex(address), size)

    def directory_read(self, u, access, address, size, value, data):
        if self.running and address == self.watched and size == 4:
            self.reads += 1
            if self.swap and self.reads == 2:
                self.put(address, '<I', SECOND_DIRECTORY)

    def begin(self, seed):
        self.running, self.watched = False, None
        self.reset()
        self.heap = bytearray(random.Random(seed).randbytes(0x10000))
        self.u.mem_write(STACK, bytes([seed & 255]) * 0x10000)
        self.u.reg_write(UC_X86_REG_FPCW, 0x037F)
        self.callbacks, self.allowed, self.reads, self.swap = {}, set(), 0, False
        self.kind, self.intensity, self.draws, self.order_count = 0, 0, [], 0
        self.head, self.split, self.multiplayer = 0, 0, 0
        return self.heap

    def intercept(self, *entries):
        for address, nargs in entries:
            self.callbacks[self.addr(address)] = (4 * nargs,
                lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        args = tuple(self.args(nargs))
        value = 0
        if address == 0x42B5F0:
            value = CAR + args[0] * 0xC24
            self.trace.append(('car', *args))
        elif address == 0x457E10:
            index = struct.unpack('<b', self.read(args[0] + 0xB1A, 1))[0]
            value = PROFILE + index * 0x100
            self.trace.append(('section', index, args[1]))
        elif address == 0x42B700:
            value = self.order_count
        elif address == 0x460BF0:
            value = self.kind
        elif address == 0x460C10:
            value = self.intensity
        elif address == 0x4C6676:
            value = self.draws.pop(0)
            self.trace.append(('rand', value))
        elif address == 0x49C0A0:
            assert args == (self.addr(0x456B70), 0), 'release registration'
            self.trace.append(('register', 0x456B70, 0))
        elif address == 0x4AADE0:
            self.trace.append(('free', *args))
        elif address == 0x4AC4F0:
            self.trace.append(('set_rotation', args[0], self.read(args[1], 8)))
        elif address == 0x4ADB10:
            assert args[0] == self.addr(0x58D260), 'wiper scratch output'
            # The real Rodrigues helper consumes only the low 12 angle bits.
            # Original short call sites may leave unrelated bits in the slot.
            angle = args[2] & 0xFFF
            self.trace.append(('axis_rotation', struct.unpack('<3i', self.read(args[1], 12)), angle))
            self.put(args[0], '<16i', *rotation_leaf(angle))
        elif address == 0x4B9F20:
            assert args[0] == args[1] and args[2] == self.addr(0x58D260), 'wiper multiply aliases / scratch'
            left = struct.unpack('<16i', self.read(args[1], 64))
            right = struct.unpack('<16i', self.read(args[2], 64))
            self.trace.append(('multiply', args[0], left, right))
            self.put(args[0], '<16i', *multiply_leaf(left, right))
        elif address == 0x411880:
            value = self.head
            self.trace.append(('head', value))
        elif address == 0x405DC0:
            value = self.split
            self.trace.append(('split', value))
        elif address == 0x41F3A0:
            value = self.multiplayer
            self.trace.append(('multiplayer', value))
        else:
            raise AssertionError(hex(address))
        self.u.reg_write(UC_X86_REG_EAX, value & 0xFFFFFFFF)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def invoke_guarded(self, address, args):
        self.u.mem_write(HEAP, bytes(self.heap))
        saved = {reg: 0x12345678 + i * 0x11111111 for i, reg in enumerate(REGISTERS)}
        for reg, value in saved.items():
            self.u.reg_write(reg, value)
        self.running = True
        try:
            self.invoke(address, [value & 0xFFFFFFFF for value in args])
        finally:
            self.running = False
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF04 + 4 * len(args), 'stdcall'
        assert all(self.u.reg_read(reg) == value for reg, value in saved.items()), 'preserved registers'
        return self.read(HEAP, 0x10000)

    def directory(self, index, section, offset, swap, seed):
        h = self.begin(seed)
        put(h, CAR + 0xB1A, '<B', index & 255)
        put(h, DIRECTORY + max(section, 0) * 2, '<H', offset)
        self.watched = self.addr(0x5429C8) + index * 4
        self.put(self.watched, '<I', DIRECTORY)
        assert self.invoke_guarded(0x457E00, [index]) == h
        assert self.u.reg_read(UC_X86_REG_EAX) == DIRECTORY and self.reads == 1, 'directory getter'
        self.reads, self.swap = 0, swap
        assert self.invoke_guarded(0x457E10, [CAR, section]) == h
        assert self.reads == 2, 'two directory reloads'
        expected = (SECOND_DIRECTORY if swap else DIRECTORY) + offset
        assert self.u.reg_read(UC_X86_REG_EAX) == expected, 'unsigned offset / signed car index / negative section'
        return expected

    def line_cache(self, order, count, seed):
        h = self.begin(seed)
        self.intercept((0x42B5F0, 1), (0x457E10, 2))
        for i in range(8):
            put(h, CAR + i * 0xC24 + 0xB1A, '<b', i)
        put(h, ORDER, '<8h', *order)
        expected, before = {}, {}
        for address, displacement in LINE_TABLES:
            before[address] = bytearray(random.Random(seed + address).randbytes(32))
            self.u.mem_write(self.addr(address), bytes(before[address]))
            expected[address] = before[address][:]
        trace = []
        for i in reversed(range(max(count, 0))):
            car = order[i]
            trace += [('car', car), ('section', car, 3)]
            for address, displacement in LINE_TABLES:
                struct.pack_into('<I', expected[address], car * 4, PROFILE + car * 0x100 + displacement)
                self.allowed.add((self.addr(address) + car * 4, 4))
        assert self.invoke_guarded(0x4809E0, [ORDER, count & 65535]) == h
        assert self.trace == trace, 'reverse list cache order'
        assert all(self.read(self.addr(a), 32) == bytes(v) for a, v in expected.items()), 'line cache / untouched slots'
        return expected, trace

    def interior_cache(self, index, model, seed):
        h = self.begin(seed)
        self.intercept((0x42B5F0, 1), (0x457E10, 2))
        put(h, CAR + index * 0xC24 + 0xB1A, '<2B', index, model & 255)
        block = bytearray(random.Random(seed + 5).randbytes(0x430))
        self.u.mem_write(self.addr(0x58D2A0), bytes(block))
        expected = block[:]
        struct.pack_into('<i', expected, 0x50 + index * 4, int((model & 255) in (7, 8, 9, 13)))
        struct.pack_into('<7I', expected, 0x250 + index * 28,
                         *(PROFILE + index * 0x100 + offset for offset in (0, 8, 12, 24, 28, 36, 44)))
        self.allowed = {(self.addr(0x58D2A0) + 0x50 + index * 4, 4)} | {
            (self.addr(0x58D2A0) + 0x250 + index * 28 + i * 4, 4) for i in range(7)}
        assert self.invoke_guarded(0x476540, [index]) == h
        assert self.read(self.addr(0x58D2A0), 0x430) == expected, 'interior pointers / neighbouring aliases'
        assert self.trace == [('car', index), ('section', index, 5)]
        return expected, self.trace

    def camera_cache(self, count, seed):
        h = self.begin(seed)
        self.intercept((0x42B5F0, 1), (0x457E10, 2), (0x42B700, 0))
        self.order_count = count
        for i in range(8):
            put(h, CAR + i * 0xC24 + 0xB1A, '<b', 7 - i)
        expected = bytearray(random.Random(seed + 99).randbytes(32))
        self.u.mem_write(self.addr(0x590D90), bytes(expected))
        trace = []
        for i in range(max(count, 0)):
            struct.pack_into('<I', expected, i * 4, PROFILE + (7 - i) * 0x100)
            self.allowed.add((self.addr(0x590D90) + i * 4, 4))
            trace += [('car', i), ('section', 7 - i, 4)]
        assert self.invoke_guarded(0x486700, []) == h
        assert self.read(self.addr(0x590D90), 32) == expected
        assert self.trace == trace, 'camera cache order / untouched entries'
        return expected, trace

    def sweep(self, index, state, kind, intensity, maximum, seed, wrapper=False, presence=3, reverse=0, second_max=700):
        h = self.begin(seed)
        self.intercept((0x460BF0, 1), (0x460C10, 1), (0x4C6676, 0))
        self.kind, self.intensity, self.draws = kind, intensity, [seed % 32768, (seed * 7919) % 32768]
        draws = self.draws[:]
        block = bytearray(random.Random(seed + 5).randbytes(0x430))
        struct.pack_into('<2i2h', block, 0x238 + index * 12, *state)
        struct.pack_into('<7I', block, 0x250 + index * 28,
                         *(PROFILE + offset for offset in (0, 8, 12, 24, 28, 36, 44)))
        put(h, PROFILE + 8, '<h', maximum)
        put(h, PROFILE + 44, '<h', second_max)
        self.u.mem_write(self.addr(0x58D2A0), bytes(block))
        expected = block[:]
        updated, used = wiper_model(state, kind, intensity, maximum, draws)
        struct.pack_into('<2i2h', expected, 0x238 + index * 12, *updated)
        base = self.addr(0x58D2A0) + 0x238 + index * 12
        self.allowed = {(base, 4), (base + 4, 4), (base + 8, 2), (base + 10, 2)}
        expected_heap = h[:]
        trace = []
        scratch = self.read(self.addr(0x58D260), 64)
        if wrapper:
            self.intercept((0x4AC4F0, 2), (0x4ADB10, 3), (0x4B9F20, 3))
            struct.pack_into('<i', block, 0x50 + index * 4, reverse)
            struct.pack_into('<2I', block, 0x288 + index * 28 + 20,
                             NODE_A if presence & 1 else 0, NODE_B if presence & 2 else 0)
            self.u.mem_write(self.addr(0x58D2A0), bytes(block))
            expected = block[:]
            struct.pack_into('<2i2h', expected, 0x238 + index * 12, *updated)
            angle = state[2]
            for j, node in enumerate((NODE_A, NODE_B)):
                if not presence & (1 << j):
                    continue
                matrix = struct.unpack_from('<16i', h, node - HEAP + 0x98)
                zeroed = list(matrix)
                zeroed[12:15] = [0, 0, 0]
                value = angle if j == 0 else trunc(second_max * angle, maximum)
                value = (short(-value) if j == 0 else -value) if reverse else value
                value &= 0xFFF
                rotation = rotation_leaf(value)
                scratch = struct.pack('<16i', *rotation)
                output = list(multiply_leaf(zeroed, rotation))
                output[12:15] = matrix[12:15]
                put(expected_heap, node + 0x98, '<16i', *output)
                trace += [('set_rotation', node, h[PROFILE - HEAP + j * 36:PROFILE - HEAP + j * 36 + 8]),
                          ('axis_rotation', matrix[4:7], value),
                          ('multiply', node + 0x98, tuple(zeroed), rotation)]
        trace += [('rand', value) for value in used]
        actual_heap = self.invoke_guarded(0x476A40 if wrapper else 0x476C70, [index])
        assert actual_heap == expected_heap, (wrapper, presence, reverse, maximum, second_max, index, state, [(hex(HEAP + i), a, b) for i, (a, b) in enumerate(zip(actual_heap, expected_heap)) if a != b][:12], self.trace, trace)
        assert self.read(self.addr(0x58D2A0), 0x430) == expected, 'signed wiper lifecycle / neighbouring records'
        assert self.trace == trace, 'wiper provider order / signed limits / preserved translation'
        assert self.read(self.addr(0x58D260), 64) == scratch, 'final scratch / no-node guard'
        return expected_heap, expected, trace

    def camera_position(self, index, mode, car, head, split, multiplayer, seed):
        h = self.begin(seed)
        self.intercept((0x411880, 0), (0x405DC0, 0), (0x41F3A0, 0))
        self.head, self.split, self.multiplayer = head, split, multiplayer
        multiply_count = 0x7FFFFFFF if seed % 2 else signed(seed * 179173)
        self.put(self.addr(0x72D67C), '<i', multiply_count)
        self.allowed.add((self.addr(0x72D67C), 4))
        put(h, CAMERA, '<B', index)
        matrix = tuple(random.Random(seed + i).randint(-65536, 65536) if i % 4 != 3 else
                       65536 if i == 15 else 0 for i in range(16))
        put(h, CAMERA + 8, '<16i', *matrix)
        self.put(self.addr(0x590D7C) + 16 + index, '<B', mode)
        self.put(self.addr(0x590EC0) + index, '<B', car)
        self.put(self.addr(0x590D90) + car * 4, '<I', PROFILE)
        level = signed(seed * 10000019)
        self.put(self.addr(0x590DB0) + index * 4, '<i', level)
        selected = 3 if mode == 0 else 4 if mode == 2 else mode
        if not (head and split and not multiplayer):
            selected = mode
        vector = struct.unpack_from('<3i', h, PROFILE - HEAP + selected * 12)
        reference = struct.unpack_from('<3i', h, REFERENCE - HEAP + 48)
        output = list(matrix)
        output[12:15] = [signed(sum(mul(vector[k], matrix[k * 4 + j]) for k in range(3)) + reference[j]) for j in range(3)]
        expected = h[:]
        put(expected, CAMERA + 8, '<16i', *output)
        put(expected, CAMERA + 0x48, '<6i', level, 0x1999, 0, 0xA000, 0, 65536)
        trace = [('head', head)] + ([('split', split)] if head else []) + ([('multiplayer', multiplayer)] if head and split else [])
        assert self.invoke_guarded(0x4869E0, [CAMERA, REFERENCE]) == expected, 'camera per-mode fixed-point model / guards'
        assert self.trace == trace
        assert self.read(self.addr(0x72D67C), 4) == struct.pack('<i', signed(multiply_count + 1)), 'real matrix multiply counter'
        return expected, trace

    def archives(self, initialise, pattern, seed):
        h = self.begin(seed)
        self.intercept((0x49C0A0, 2), (0x4AADE0, 1))
        records = bytearray(random.Random(seed + 19).randbytes(0x188))
        frees = []
        for i in range(32):
            present = pattern == 1 or pattern == 2 and (seed + i * 7) % 3 != 0
            pointer = HEAP + 0xC000 + i * 16 if present else 0
            struct.pack_into('<I', records, i * 12, pointer)
            if pointer:
                frees.append(('free', pointer))
        self.u.mem_write(self.addr(0x542AE8), bytes(records))
        expected = bytes(32 * 12) + records[32 * 12:]
        self.allowed = {(self.addr(0x542AE8) + i * 4, 4) for i in range(96)}
        assert self.invoke_guarded(0x456BB0 if initialise else 0x456B70, []) == h
        assert self.read(self.addr(0x542AE8), 0x188) == expected, 'archive fields / unchanged counters'
        trace = [('register', 0x456B70, 0)] if initialise else frees
        assert self.trace == trace, 'archive ownership / release order'
        return expected, trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    images = (Records(ROOT / 'cmr2bin/CMR2.exe'), Records(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities))
    cases = []
    for args in itertools.product((-128, -1, 0, 1, 7, 127), (-0x80000000, -1, 0, 1, 5, 15), (0, 1, 255, 65535), (False, True)):
        cases.append(('directory', args))
    for order, count in itertools.product((tuple(range(8)), tuple(reversed(range(8))), (7, 0, 7, 1, 3, 3, 2, 6)), (-3, 0, 1, 4, 8)):
        cases.append(('line_cache', (order, count)))
    for args in itertools.product((0, 1), (-128, -1, 0, 7, 8, 9, 13, 127)):
        cases.append(('interior_cache', args))
    for count in (-3, 0, 1, 2, 7, 8):
        cases.append(('camera_cache', (count,)))
    states = [(mode, forward, angle, step) for mode in (0, 1, 2, 3, 99, -1) for forward, angle, step in
              ((0, 0, 50), (1, 999, 50), (-1, 32767, 50), (0, -32768, -1), (1, -1, 0))]
    for args in itertools.product((0, 1), states, (0, 1, 2, 255), (-1, 0x1998, 0x1999, 0x3333, 0x3334, 0x6665, 0x6666, 0x8000, 0x8001), (-32768, -1, 0, 1000, 32767)):
        cases.append(('sweep', args))
    for args in itertools.product((0, 3), (0, 1, 2, 3, 4, 9), (0, 7), (0, 1), (0, 1), (0, 1)):
        cases.append(('camera_position', args))
    for args in itertools.product((False, True), (0, 1, 2)):
        cases.append(('archives', args))
    for i, (method, args) in enumerate(cases):
        try:
            a, b = [getattr(instance, method)(*args, 1977 + i) for instance in images]
            assert a == b, 'original/rebuilt'
        except Exception as error:
            raise AssertionError((method, args, i)) from error
    wrappers = 0
    for index, state, presence, reverse, maximum, second in itertools.product((0, 1), states[:10], range(4), (0, 1, -1), (-32768, -1, 1, 1000, 32767), (-32768, -1, 0, 700, 32767)):
        a, b = [instance.sweep(index, state, 1, 0x8001, maximum, 47391 + wrappers,
                               wrapper=True, presence=presence, reverse=reverse, second_max=second) for instance in images]
        assert a == b, ('wiper wrapper original/rebuilt', index, state, presence, reverse, maximum, second)
        wrappers += 1
    print(f'{len(cases) + wrappers} CIN/cockpit record cases: 0 differences; independent directory reloads, '
          'pointer caches, signed sweeps, camera fixed-point offsets and archive ownership; complete poisoned '
          'heap/block guards, widths, provider order and ABI; wiper geometry leaves controlled')


if __name__ == '__main__':
    main()
