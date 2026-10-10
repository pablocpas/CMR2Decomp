#!/usr/bin/env python3
"""Independent light-state, steering-transition and interior-release models.

Complete original/rebuilt bodies execute with poisoned neighbouring records,
heap and stage block, widths, aliased outputs, preserved ABI and call order.
Texture blits/remaps, node destruction and buffer release are controlled leaves.
The existing headlight-glow lifecycle harness covers the shared 100-record pool.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_car_info_records import Records, CAR, put, short, trunc
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import signed, mul

LIGHTS, BLOCK = 0x58D6D0, 0x58D2A0
TEXTURES = (HEAP + 0xD000, HEAP + 0xE000)
LEVEL_EDGES = (-32768, -1, 0, 1, 254, 255, 256, 32767)
TIMESTEPS = (-65536, 0, 1, 65536, 131072, 0x7FFFFFFF)
# Original display glyphs, including state 7's reverse glyph and the default
# middle segment. These describe the format, rather than a decimal algorithm.
GEAR_GLYPHS = (126, 12, 55, 31, 77, 91, 123, 33, 127, 79)


def gear_levels(state):
    mask = GEAR_GLYPHS[state] if 0 <= state < len(GEAR_GLYPHS) else 1
    return tuple(255 if mask & (1 << i) else 0 for i in range(7))


def gear_trace(texture, levels, slot):
    return [('blit', texture, slot),
            ('alpha', texture, 128, levels[0], 224, levels[1], 208, levels[2], slot),
            ('alpha', texture, 192, levels[3], 176, levels[4], 160, levels[5], slot),
            ('alpha', texture, 144, levels[6], 144, levels[6], 144, levels[6], slot)]


def animate_model(record, timestep):
    result = bytearray(record)
    for row, channel in itertools.product(range(2), range(5)):
        faded = 48 + row * 10 + channel * 2
        current = struct.unpack_from('<h', result, faded)[0]
        if record[68 + row] & (1 << channel):
            value = min(current + mul(128, timestep), 255)
        else:
            value = max(current - mul(80, timestep), 0)
        value = short(value) if row == 0 else value & 255
        struct.pack_into('<h', result, faded, value)
        struct.pack_into('<h', result, 8 + row * 10 + channel * 2, value)
    for row in range(2):
        base = 8 + row * 10
        first, second = struct.unpack_from('<2h', result, base)
        if record[70]:
            struct.pack_into('<h', result, base + 2, min(second + trunc(first, 3) * 2, 255) & 255)
        else:
            struct.pack_into('<h', result, base, short(trunc(first, 3) * 2))
    levels = struct.unpack_from('<10h', result, 8)
    repaint = levels != struct.unpack_from('<10h', record, 28)
    trace = []
    if repaint:
        values = [max(levels[i], levels[i + 5]) & 65535 for i in range(5)]
        for texture in struct.unpack_from('<2I', record):
            if texture:
                trace.extend([('blit', texture),
                              ('alpha', texture, 0xBF, values[1], 0x40, values[3], 0x80, values[2]),
                              ('alpha', texture, 0x60, values[0], 0xE0, values[4], 0xE0, values[4])])
        result[28:48] = result[8:28]
    return result, trace


def steering_model(mode, frame, last, requested, handbrake):
    result = 0
    changed = lambda: last != requested or handbrake != 0
    if mode == 1 and changed():
        last, mode, frame = requested & 255, 2, 0
    if mode == 2:
        result = frame * 65536 // 7
        last = requested & 255
        frame = (frame + 1) & 255
        if frame > 7:
            mode, frame = 3, 0
    if mode == 3:
        result = 65536
        frame = (frame + 1) & 255
        if changed():
            frame, last = 0, requested & 255
        if frame > 3:
            mode, frame = 4, 0
    if mode == 4:
        result = 65536 - frame * 65536 // 7
        if not changed():
            frame = (frame + 1) & 255
            if frame > 7:
                mode, frame, result = 1, 0, 0
        else:
            last, mode, frame = requested & 255, 2, (7 - frame) & 255
    return mode, frame, last, result


class LightState(Records):
    def provider(self, address, nargs):
        args = tuple(self.args(nargs))
        if address == 0x4A5080:
            self.trace.append(('blit', *args))
        elif address == 0x4A4D30:
            self.trace.append(('alpha', args[0], *(a & 65535 for a in args[1:7]), args[7]))
        elif address == 0x4AC620:
            self.trace.append(('destroy', *args))
        elif address == 0x4AADE0:
            self.trace.append(('free', *args))
        else:
            return super().provider(address, nargs)
        self.u.reg_write(UC_X86_REG_EAX, 0x76543210)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def allow(self, address, offset, length, widths=(1, 2, 4)):
        base = self.addr(address) + offset
        self.allowed.update((base + p, width) for p in range(length)
                            for width in widths if p + width <= length)

    def light_begin(self, seed):
        heap = self.begin(seed)
        records = bytearray(random.Random(seed + 13).randbytes(8 * 72 + 8))
        self.u.mem_write(self.addr(LIGHTS) - 4, bytes(records))
        return heap, records

    def finish_lights(self, address, args, heap, records, trace=()):
        assert self.invoke_guarded(address, args) == heap, 'poisoned heap / alias outputs'
        assert self.read(self.addr(LIGHTS) - 4, len(records)) == records, 'records / guards / preserved fields'
        assert self.trace == list(trace), 'provider order / signed level arguments'

    def animate(self, car, seed, mask, presence, combine, same):
        heap, records = self.light_begin(seed)
        base = 4 + car * 72
        for i in range(10):
            struct.pack_into('<h', records, base + 48 + 2 * i, LEVEL_EDGES[(seed + i) % len(LEVEL_EDGES)])
        struct.pack_into('<2I', records, base,
                         TEXTURES[0] if presence & 1 else 0, TEXTURES[1] if presence & 2 else 0)
        records[base + 68:base + 71] = bytes((mask, mask ^ 31, combine))
        timestep = TIMESTEPS[seed % len(TIMESTEPS)]
        expected_record, _ = animate_model(records[base:base + 72], timestep)
        if same:
            records[base + 28:base + 48] = expected_record[8:28]
        else:
            records[base + 28:base + 48] = b'\xFF\x7F' * 10
        expected_record, trace = animate_model(records[base:base + 72], timestep)
        expected = records[:]
        expected[base:base + 72] = expected_record
        self.u.mem_write(self.addr(LIGHTS) - 4, bytes(records))
        self.put(self.addr(0x51BD3C), '<i', timestep)
        self.intercept((0x4A5080, 2), (0x4A4D30, 8))
        self.allow(LIGHTS, car * 72 + 8, 60, (2,))
        self.finish_lights(0x477CE0, [car], heap, expected, [(*event, car) for event in trace])

    def reset_levels(self, car, seed, invalidate=False):
        heap, records = self.light_begin(seed)
        expected = records[:]
        if invalidate:
            for i in range(8):
                expected[4 + i * 72 + 28:4 + i * 72 + 48] = b'\xFF' * 20
                self.allow(LIGHTS, i * 72 + 28, 20)
        else:
            base = 4 + car * 72
            expected[base + 8:base + 28] = bytes(20)
            expected[base + 28:base + 48] = b'\xFF' * 20
            expected[base + 48:base + 70] = bytes(22)
            self.allow(LIGHTS, car * 72 + 8, 62)
        self.finish_lights(0x477F30 if invalidate else 0x477AC0,
                           [] if invalidate else [car], heap, expected)

    def flags(self, car, set_a, set_b, mask, seed):
        heap, records = self.light_begin(seed)
        expected = records[:]
        for row, flag in enumerate((set_a, set_b)):
            p = 4 + car * 72 + 68 + row
            enabled = flag & 255
            expected[p] = records[p] | mask if enabled else records[p] & (~mask & 255)
            self.allow(LIGHTS, car * 72 + 68 + row, 1, (1,))
        self.finish_lights(0x477C20, [car, set_a, set_b, mask], heap, expected)

    def levels(self, car, slot, outputs, seed):
        heap, records = self.light_begin(seed)
        for row in range(2):
            struct.pack_into('<h', records, 4 + car * 72 + 8 + row * 10 + slot * 2,
                             LEVEL_EDGES[(seed + row) % len(LEVEL_EDGES)])
        self.u.mem_write(self.addr(LIGHTS) - 4, bytes(records))
        pointers = ((0, 0), (HEAP + 0xF000, 0), (0, HEAP + 0xF004),
                    (HEAP + 0xF000, HEAP + 0xF000),
                    (self.addr(LIGHTS) + car * 72 + 18 + slot * 2, HEAP + 0xF004))[outputs]
        expected_heap, expected = heap[:], records[:]
        for row, pointer in enumerate(pointers):
            if not pointer:
                continue
            value = struct.unpack_from('<h', expected, 4 + car * 72 + 8 + row * 10 + slot * 2)[0] * 256
            if HEAP <= pointer < HEAP + 0x10000:
                put(expected_heap, pointer, '<i', value)
            else:
                struct.pack_into('<i', expected, pointer - (self.addr(LIGHTS) - 4), value)
                self.allowed.add((pointer, 4))
        self.finish_lights(0x477C80, [car, *pointers, slot], expected_heap, expected)

    def steering(self, index, mode, frame, requested, same, handbrake, seed):
        heap = self.begin(seed)
        block = bytearray(random.Random(seed + 37).randbytes(0x430))
        last = requested & 255 if same else (requested + 1) & 255
        block[0x30 + index], block[0x1D8 + index] = last, frame
        struct.pack_into('<i', block, 0xC0 + index * 4, mode)
        put(heap, CAR + 0xB20, '<b', requested)
        put(heap, CAR + 0x1D8, '<i', handbrake)
        expected = block[:]
        new_mode, new_frame, new_last, result = steering_model(mode, frame, last, requested, handbrake)
        expected[0x30 + index], expected[0x1D8 + index] = new_last, new_frame
        struct.pack_into('<i', expected, 0xC0 + index * 4, new_mode)
        self.u.mem_write(self.addr(BLOCK), bytes(block))
        self.allow(BLOCK, 0x30 + index, 1, (1,))
        self.allow(BLOCK, 0x1D8 + index, 1, (1,))
        self.allow(BLOCK, 0xC0 + index * 4, 4, (4,))
        assert self.invoke_guarded(0x476850, [index, CAR]) == heap
        assert self.read(self.addr(BLOCK), 0x430) == expected, (index, mode, frame, requested, same, handbrake)
        assert signed(self.u.reg_read(UC_X86_REG_EAX)) == result, 'steering fraction / sequential phases'
        assert not self.trace

    def digit(self, index, state, cache_base, same, seed):
        heap = self.begin(seed)
        block = bytearray(random.Random(seed + 53).randbytes(0x430))
        levels = gear_levels(state)
        encoded = struct.pack('<7H', *levels)
        if same:
            block[0x410 + index * 14:0x41E + index * 14] = encoded
        else:
            block[0x410 + index * 14:0x41E + index * 14] = b'\xFF\x7F' * 7
        expected = block[:]
        expected[0x34 + index * 14:0x42 + index * 14] = encoded
        expected[0x410 + index * 14:0x41E + index * 14] = encoded
        self.u.mem_write(self.addr(BLOCK), bytes(block))
        self.intercept((0x4A5080, 2), (0x4A4D30, 8))
        self.allow(BLOCK, 0x34 + index * 14, 14)
        self.allow(BLOCK, 0x410 + index * 14, 14)
        assert self.invoke_guarded(0x4775F0, [TEXTURES[0], state, cache_base, index]) == heap
        assert self.read(self.addr(BLOCK), 0x430) == expected, 'digit format / applied cache / neighbours'
        assert self.trace == ([] if same else gear_trace(TEXTURES[0], levels, index + cache_base * 8))

    def rev(self, index, revs, presence, same, seed):
        heap = self.begin(seed)
        block = bytearray(random.Random(seed + 61).randbytes(0x430))
        struct.pack_into('<2I', block, 0x220 + index * 8,
                         TEXTURES[0] if presence & 1 else 0, TEXTURES[1] if presence & 2 else 0)
        gear = (-128, -1, 0, 1, 6, 7, 9, 127)[seed % 8]
        put(heap, CAR + index * 0xC24 + 0xB1E, '<b', gear)
        self.put(self.addr(0x53CE54) + index * 4, '<i', revs)
        limit = mul(revs, 11 * 65536) >> 16
        new = [255 if i < limit else 0 for i in range(11)]
        new.append(108 if presence & 1 else struct.unpack_from('<H', block, 0x70 + index * 24 + 22)[0])
        encoded = struct.pack('<12H', *new)
        levels = gear_levels(gear)
        digit = struct.pack('<7H', *levels)
        block[index * 24:index * 24 + 24] = encoded if same else b'\xFF\x7F' * 12
        block[0x410 + index * 14:0x41E + index * 14] = digit if same else b'\xFF\x7F' * 7
        expected, trace = block[:], []
        expected[0x70 + index * 24:0x88 + index * 24] = encoded
        if presence & 2:
            trace.append(('car', index))
            expected[0x34 + index * 14:0x42 + index * 14] = digit
            expected[0x410 + index * 14:0x41E + index * 14] = digit
            if not same:
                trace.extend(gear_trace(TEXTURES[1], levels, index + 16))
        if presence & 1 and not same:
            expected[index * 24:index * 24 + 24] = encoded
            slot = index + 8
            trace.append(('blit', TEXTURES[0], slot))
            trace.append(('alpha', TEXTURES[0], 224, 255, 208, new[1], 192, new[2], slot))
            for a, b, c, p in ((176, 160, 144, 3), (128, 112, 96, 6), (80, 64, 48, 9)):
                trace.append(('alpha', TEXTURES[0], a, new[p], b, new[p + 1], c, new[p + 2], slot))
        self.u.mem_write(self.addr(BLOCK), bytes(block))
        self.intercept((0x42B5F0, 1), (0x4A5080, 2), (0x4A4D30, 8))
        self.allow(BLOCK, index * 24, 24)
        self.allow(BLOCK, 0x70 + index * 24, 24)
        self.allow(BLOCK, 0x34 + index * 14, 14)
        self.allow(BLOCK, 0x410 + index * 14, 14)
        assert self.invoke_guarded(0x477460, [index]) == heap
        assert self.read(self.addr(BLOCK), 0x430) == expected, 'rev thresholds / last alpha / absent textures / neighbours'
        assert self.trace == trace, 'gear update precedes rev repaint / cache slots'

    def release(self, roots, references, buffers, archives, seed):
        heap = self.begin(seed)
        block = bytearray(random.Random(seed + 79).randbytes(0x430))
        for i in range(2):
            struct.pack_into('<I', block, 0x1FC + i * 4, HEAP + 0x6000 + i * 0x200 if roots & (1 << i) else 0)
            struct.pack_into('<I', block, 0x288 + i * 28 + 8, HEAP + 0x6800 + i * 0x200 if references & (1 << i) else 0)
            struct.pack_into('<I', block, 0x400 + i * 4, HEAP + 0x7000 + i * 0x100 if buffers & (1 << i) else 0)
        for i in range(16):
            used = archives == 1 or archives == 2 and i % 3 == 0
            struct.pack_into('<3I', block, 0x118 + i * 12, HEAP + 0x8000 + i * 0x80 if used else 0, 97 + i, i & 1)
        expected, trace = block[:], []
        for i in range(2):
            root = struct.unpack_from('<I', block, 0x1FC + i * 4)[0]
            reference = struct.unpack_from('<I', block, 0x288 + i * 28 + 8)[0]
            if root:
                if reference:
                    trace.append(('destroy', reference))
                trace.append(('destroy', root))
                expected[0x290 + i * 28:0x2A4 + i * 28] = bytes(20)
                self.allow(BLOCK, 0x290 + i * 28, 20, (4,))
        for i in range(2):
            pointer = struct.unpack_from('<I', block, 0x400 + i * 4)[0]
            if pointer:
                trace.append(('free', pointer))
                expected[0x400 + i * 4:0x404 + i * 4] = bytes(4)
                self.allow(BLOCK, 0x400 + i * 4, 4, (4,))
        expected[0x220:0x230] = bytes(16)
        self.allow(BLOCK, 0x220, 16, (4,))
        for i in range(16):
            pointer = struct.unpack_from('<I', block, 0x118 + i * 12)[0]
            if pointer:
                trace.append(('free', pointer))
        expected[0x118:0x1D8] = bytes(192)
        self.allow(BLOCK, 0x118, 192, (4,))
        self.u.mem_write(self.addr(BLOCK), bytes(block))
        self.intercept((0x4AC620, 1), (0x4AADE0, 1))
        assert self.invoke_guarded(0x4779E0, []) == heap, 'controlled leaves must not mutate heap'
        assert self.read(self.addr(BLOCK), 0x430) == expected, 'release leaves roots/rest/blend/flags untouched'
        assert self.trace == trace, 'ownership and release order'
        assert self.u.reg_read(UC_X86_REG_EAX) == 1


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    runs = (LightState(ROOT / 'cmr2bin/CMR2.exe'), LightState(ROOT / 'build/CMR2.exe', entities))
    cases = 0
    def check(method, args):
        nonlocal cases
        for run in runs:
            getattr(run, method)(*args, cases + 41)
        cases += 1
    for car, seed, mask, presence, combine, same in itertools.product((0, 7), range(6), (0, 1, 31, 255), range(4), (0, 1), (0, 1)):
        for run in runs:
            run.animate(car, seed, mask, presence, combine, same)
        cases += 1
    for car in range(8):
        check('reset_levels', (car,))
    for run in runs:
        run.reset_levels(0, 67, True)
    cases += 1
    for args in itertools.product((0, 7), (-256, -128, -1, 0, 1, 256), (-256, -128, -1, 0, 1, 256), (0, 1, 2, 4, 8, 16, 128, 255)):
        check('flags', args)
    for args in itertools.product((0, 7), range(5), range(5)):
        check('levels', args)
    for args in itertools.product(range(2), (-1, 0, 1, 2, 3, 4, 99), (0, 1, 3, 4, 6, 7, 8, 127, 254, 255), (-128, -1, 0, 127), (False, True), (0, 1, -1)):
        check('steering', args)
    for args in itertools.product(range(2), (-128, -1, *range(10), 10, 127), (0, 2), (False, True)):
        check('digit', args)
    thresholds = sorted({k * 65536 // 11 + delta for k in range(13) for delta in (-1, 0, 1)})
    for args in itertools.product(range(2), thresholds, range(4), (False, True)):
        check('rev', args)
    for args in itertools.product(range(4), range(4), range(4), range(3)):
        check('release', args)
    print(cases, 'light/interior cases: 0 differences; independent signed fades, alias outputs, byte masks, gear/rev cache thresholds, sequential steering phases and release ownership; full poisoned records/block/heap, guards, widths, provider order and ABI; render/release leaves controlled')
    return 0


if __name__ == '__main__':
    sys.exit(main())
