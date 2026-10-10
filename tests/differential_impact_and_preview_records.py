#!/usr/bin/env python3
"""Independent packed-impact, pool, snapshot and signed preview-orientation models.

Full encoder/decoder, snapshot exporter, angle setter/animator and reference
projection bodies execute. Projection/rendering and menu providers are controlled
leaves. Check complete poisoned heaps/global records, untouched bytes, provider
ordering, preserved registers and stdcall. The angle model retains the original
single positive reflection/negative turn adjustment rather than normalising it.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn import UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX,
                              UC_X86_REG_EDX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP, UC_X86_REG_ESP, UC_X86_REG_FPCW)
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting, mul, signed

CAR, DAMAGE, SAVED, INPUT = HEAP + 0x1000, HEAP + 0x3000, HEAP + 0x6000, HEAP + 0x9000
REGISTERS = (UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP)
ANGLE_REGIONS = ((0x82D0B8, 32), (0x82D0D8, 32), (0x82D0F8, 32),
                 (0x82FCE0, 32), (0x831148, 80), (0x83131C, 4), (0x831080, 4))


def put(heap, address, fmt, *values):
    struct.pack_into(fmt, heap, address - HEAP, *values)


def trunc(a, b):
    q = abs(a) // abs(b)
    return -q if (a < 0) != (b < 0) else q


def div(a, b):
    return signed(trunc(signed(a) * 65536, signed(b)))


def short(value):
    return (value + 32768) % 65536 - 32768


def decode(payload, initial):
    position = struct.unpack_from('<3b', payload, 9)
    normal = struct.unpack_from('<3b', payload, 3)
    axis = struct.unpack_from('<3b', payload, 6)
    radius = initial[3]
    reciprocal = trunc(1 << 32, 0x7F0000)
    if payload[1] == 1:
        radius = mul(payload[2] << 16, mul(0xA0000, div(0x10000, 0xFF0000)))
    return (tuple(mul(v << 16, mul(0xA0000, div(0x10000, 0x7F0000))) for v in position),
            tuple(mul(v << 16, reciprocal) for v in normal),
            tuple(mul(v << 16, reciprocal) for v in axis),
            radius, div(payload[0] << 16, 0xFF0000),
            (initial[5] & 0xFFFFFF00) | payload[1])


def encode(initial, old_payload):
    position, normal, axis, radius, strength, mode = initial
    payload = bytearray(old_payload)
    payload[0] = min(mul(strength, 0xFF0000) >> 16, 255) & 255
    payload[1] = mode & 255
    if mode == 1:
        payload[2] = min(mul(radius, mul(div(0x10000, 0xA0000), 0xFF0000)) >> 16, 255) & 255
    for offset, vector, scale in ((9, position, mul(div(0x10000, 0xA0000), 0x7F0000)),
                                  (3, normal, 0x7F0000), (6, axis, 0x7F0000)):
        for i, value in enumerate(vector):
            payload[offset + i] = max(-127, min(mul(value, scale) >> 16, 127)) & 255
    return bytes(payload)


def pool_model(pool, initial, locked, scratch_byte):
    result = bytearray(pool)
    limit = min(mul(initial[4], 0xFF0000) >> 16, 255)
    chosen = None
    if not locked:
        count = result[0x104]
        if count < 20:
            if count:
                result[count * 13 - 1] = count
            chosen = count
            result[0x104] = count + 1
        else:
            index, prev, weakest, smallest, tail = result[0x105], None, None, 1000, None
            while index is not None:
                tail = index
                strength = result[index * 13]
                if strength < smallest:
                    smallest, weakest, before_weakest = strength, index, prev
                link = result[index * 13 + 12]
                if link == 255:
                    break
                prev, index = index, short(link) if link < 128 else link - 256
            if weakest is not None and smallest < (limit & 255):
                chosen = weakest
                if chosen != tail:
                    if before_weakest is None:
                        result[tail * 13 + 12] = result[0x105]
                        result[0x105] = result[chosen * 13 + 12]
                    else:
                        result[tail * 13 + 12] = result[before_weakest * 13 + 12]
                        result[before_weakest * 13 + 12] = result[chosen * 13 + 12]
                    result[chosen * 13 + 12] = 255
    old = result[chosen * 13:chosen * 13 + 12] if chosen is not None else bytes([scratch_byte]) * 12
    payload = encode(initial, old)
    if chosen is not None:
        result[chosen * 13:chosen * 13 + 12] = payload
    return bytes(result), decode(payload, initial)


class Records(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x4A9B80, 0), (0x405D70, 0), (0x4AC4F0, 2),
                               (0x500920, 3), (0x509DC0, 1), (0x5091C0, 1),
                               (0x42B5F0, 1), (0x41B370, 0), (0x407610, 1),
                               (0x5068B0, 4), (0x501690, 2)):
            self.callbacks[self.addr(address)] = (4 * nargs,
                lambda a=address, n=nargs: self.provider(a, n))
        self.running = False
        self.u.hook_add(UC_HOOK_MEM_WRITE, self.global_write,
                        begin=self.base, end=self.base + self.size - 1)

    def global_write(self, u, access, address, size, value, data):
        if self.running:
            assert any(start <= address and address + size <= start + length
                       for start, length in self.allowed), ('global width/guard', hex(address), size)

    def begin(self, seed):
        self.running = False
        self.reset()
        self.heap = bytearray(random.Random(seed).randbytes(0x10000))
        self.u.mem_write(STACK, bytes([seed & 255]) * 0x10000)
        self.u.reg_write(UC_X86_REG_FPCW, 0x037F)
        self.time, self.players, self.allowed = 0, 0, []
        return self.heap

    def invoke_guarded(self, target, args):
        self.u.mem_write(HEAP, bytes(self.heap))
        initial = {r: 0x12345678 + i * 0x11111111 for i, r in enumerate(REGISTERS)}
        for r, value in initial.items():
            self.u.reg_write(r, value)
        self.running = True
        try:
            self.invoke(target, [v & 0xFFFFFFFF for v in args])
        finally:
            self.running = False
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF04 + 4 * len(args), 'stdcall'
        assert all(self.u.reg_read(r) == value for r, value in initial.items()), 'callee ABI'
        return self.read(HEAP, 0x10000)

    def provider(self, address, nargs):
        args = tuple(self.args(nargs))
        value = 0
        if address == 0x4A9B80:
            value = self.time
        elif address == 0x405D70:
            value = self.players
        elif address == 0x4AC4F0:
            node, angles = args
            self.trace.append(('rotation', node, self.read(angles, 8)))
        elif address == 0x42B5F0:
            value = CAR
            self.trace.append(('car', *args))
        elif address == 0x41B370:
            value = 3
            self.trace.append(('event',))
        elif address == 0x407610:
            value = SAVED
            self.trace.append(('saved', *args))
        elif address == 0x5068B0:
            slot, source, target, angles = args
            vector = struct.unpack('<3i', self.read(source, 12))
            rotation = struct.unpack('<4H', self.read(angles, 8))
            self.trace.append(('reference', slot, vector, rotation))
            self.put(target, '<3i', signed(vector[0] + rotation[0]),
                     signed(vector[1] - rotation[1]), signed(vector[2] + rotation[2]))
        elif address == 0x501690:
            target, source = args
            v = struct.unpack('<3i', self.read(source, 12))
            self.trace.append(('project', v))
            self.put(target, '<2i', signed(v[0] + v[2]), signed(v[1] - v[2]))
        else:
            self.trace.append((address, *args))
        self.u.reg_write(UC_X86_REG_EAX, value & 0xFFFFFFFF)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def dent_globals(self, preview, initial=None):
        addresses = (0x82D120, 0x82D12C, 0x82D138, 0x82D144, 0x82D148, 0x82D14C) if preview else (
                     0x588A40, 0x588A4C, 0x588A58, 0x588A64, 0x588A68, 0x588A6C)
        if initial is not None:
            for a, values in zip(addresses[:3], initial[:3]):
                self.put(self.addr(a), '<3i', *values)
            for a, value in zip(addresses[3:], initial[3:]):
                self.put(self.addr(a), '<I', value & 0xFFFFFFFF)
        self.allowed = [(self.addr(a), 12 if i < 3 else 1 if i == 5 else 4)
                        for i, a in enumerate(addresses)]
        return tuple(struct.unpack('<3i', self.read(self.addr(a), 12)) if i < 3 else
                     struct.unpack('<I', self.read(self.addr(a), 4))[0] if i == 5 else
                     struct.unpack('<i', self.read(self.addr(a), 4))[0] for i, a in enumerate(addresses))

    def decoder(self, preview, payload, seed):
        h = self.begin(seed)
        put(h, INPUT, '<12B', *payload)
        # Preview mode is a BYTE global; the trailing three guarded bytes are
        # adjacent storage, distinct from the car's partially written int mode.
        initial = ((123, -456, 789), (12, 34, 56), (-4, -8, -12), -998877, 77123, 0x123456AB)
        self.dent_globals(preview, initial)
        expected = decode(payload, initial)
        actual = self.invoke_guarded(0x507710 if preview else 0x4688B0, [INPUT])
        assert actual == h, 'decoder heap guards'
        assert self.dent_globals(preview) == expected, ('decoder model', preview, payload)
        return actual, expected

    def writer(self, count, locked, chain_kind, initial, index, seed):
        h = self.begin(seed)
        put(h, CAR + 0xB1A, '<b', index)
        self.put(self.addr(0x588A70), '<I', CAR)
        self.put(self.addr(0x588B98), '<I', DAMAGE)
        record = DAMAGE + index * 0x290
        put(h, record + 0x28C, '<i', locked)
        pool = record + 0x106
        put(h, pool + 0x104, '<2B', count, 0)
        order = list(range(20)) if chain_kind < 4 else list(reversed(range(20)))
        if count >= 20:
            put(h, pool + 0x105, '<B', order[0])
        for i in range(20):
            put(h, pool + i * 13, '<B', 150)
            put(h, pool + i * 13 + 12, '<B', 255)
        if count >= 20:
            for prev, next_ in zip(order, order[1:]):
                put(h, pool + prev * 13 + 12, '<B', next_)
            weakest = order[0] if chain_kind % 4 == 0 else order[7] if chain_kind % 4 == 1 else order[-1]
            if chain_kind % 4 != 3:
                put(h, pool + weakest * 13, '<B', 7)
        self.dent_globals(False, initial)
        expected = bytearray(h)
        new_pool, decoded = pool_model(h[pool-HEAP:pool-HEAP+0x106], initial, locked, seed & 255)
        expected[pool-HEAP:pool-HEAP+0x106] = new_pool
        actual = self.invoke_guarded(0x468520, [])
        assert actual == expected, ('pool model / complete guards', count, locked, chain_kind, initial, index)
        assert self.dent_globals(False) == decoded, ('writer decoder model', count, locked, chain_kind, initial)
        return actual, decoded

    def export(self, index, seed):
        h = self.begin(seed)
        put(h, CAR + 0xB1A, '<b', index)
        self.put(self.addr(0x588B98), '<I', DAMAGE)
        expected = bytearray(h)
        source = DAMAGE - HEAP + index * 0x290
        target = SAVED - HEAP
        expected[target:target+0x106] = h[source+0x106:source+0x20C]
        expected[target+0x108:target+0x148] = h[source+0x24C:source+0x28C]
        actual = self.invoke_guarded(0x469B50, [index])
        assert actual == expected, 'two-copy snapshot model / alignment guards'
        assert self.trace == [('car', index), ('event',), ('saved', 3 + index)], 'export provider order'
        return actual, self.trace

    def orientation(self, slot, target, instant, now, elapsed, players, seed):
        h = self.begin(seed)
        rng = random.Random(seed + 10000)
        regions = {a: bytearray(rng.randbytes(n)) for a, n in ANGLE_REGIONS}
        for a, value in regions.items():
            self.u.mem_write(self.addr(a), bytes(value))
        self.u.mem_write(INPUT, struct.pack('<4h', *target))
        put(h, INPUT, '<4h', *target)
        self.time, self.players = now, players
        self.allowed = [(self.addr(a), n) for a, n in ANGLE_REGIONS]
        expected = {a: bytearray(value) for a, value in regions.items()}
        if seed % 2 == 0:
            struct.pack_into('<3h', regions[0x82D0B8], slot * 8, 0, 0, 0)
            self.u.mem_write(self.addr(0x82D0B8), bytes(regions[0x82D0B8]))
            expected[0x82D0B8] = bytearray(regions[0x82D0B8])
        before = struct.unpack_from('<3h', expected[0x82D0B8], slot * 8)
        struct.pack_into('<I', expected[0x831080], 0, 20)
        struct.pack_into('<I', expected[0x831148], slot * 4, now)
        expected[0x83131C][slot] = 1
        struct.pack_into('<3h', expected[0x82D0F8], slot * 8, *before)
        struct.pack_into('<3h', expected[0x82FCE0], slot * 8, *target[:3])
        if instant:
            value = expected[0x82FCE0][slot*8:slot*8+8]
            expected[0x82D0B8][slot*8:slot*8+8] = value
            expected[0x82D0F8][slot*8:slot*8+8] = value
            before = target[:3]
        deltas = []
        for t, old in zip(target[:3], before):
            angle = (t - old) * 5760
            if abs(angle) > 180 * 65536:
                angle = 360 * 65536 - angle if angle > 0 else angle + 360 * 65536
            deltas.append(short(round(angle / 5760)))
        struct.pack_into('<3h', expected[0x82D0D8], slot * 8, *deltas)
        actual = self.invoke_guarded(0x506930, [slot, INPUT, instant])
        assert actual == h, 'target heap guards'
        assert all(self.read(self.addr(a), len(v)) == v for a, v in expected.items()), ('target signed/wrap/padding model', slot, target, instant)
        self.time = (now + elapsed) & 0xFFFFFFFF
        # Other slots are deliberately dormant; untouched fourth words and the
        # remaining start-time capacity stay poisoned in the complete records.
        for i in range(4):
            if i != slot:
                expected[0x83131C][i] = 0
        self.u.mem_write(self.addr(0x83131C), bytes(expected[0x83131C]))
        trace = []
        for i in range(players):
            node = HEAP + 0xA000 + i * 0x18C
            self.put(self.addr(0x82CB78) + i * 0x54 + 8, '<I', node)
            if expected[0x83131C][i] and elapsed > 20:
                expected[0x83131C][i] = 0
            if expected[0x83131C][i]:
                start = struct.unpack_from('<I', expected[0x831148], i * 4)[0]
                t = (((self.time * 100 - start * 100) & 0xFFFFFFFF) // 20)
                k = trunc(signed(t*t), 100)
                delta = struct.unpack_from('<3h', expected[0x82D0D8], i * 8)
                origin = struct.unpack_from('<3h', expected[0x82D0F8], i * 8)
                angles = [short(trunc(signed(d*k), 100) + o) for d, o in zip(delta, origin)]
                trace.append((0x500920, i, 0, k & 0xFFFFFFFF))
            else:
                angles = struct.unpack_from('<3h', expected[0x82FCE0], i * 8)
                trace.append((0x500920, i, 1, 0))
            struct.pack_into('<3h', expected[0x82D0B8], i * 8, *angles)
            trace += [('rotation', node, bytes(expected[0x82D0B8][i*8:i*8+8])), (0x509DC0, i), (0x5091C0, i)]
        self.trace = []
        actual = self.invoke_guarded(0x506720, [])
        assert actual == h, 'animate heap guards'
        assert self.trace == trace, ('animation provider ordering', self.trace, trace)
        assert all(self.read(self.addr(a), len(v)) == v for a, v in expected.items()), ('animation model / whole records', slot, elapsed, players)
        return actual, tuple(bytes(v) for v in expected.values()), trace

    def projection(self, model, fixture, presets, seed):
        h = self.begin(seed)
        actual_fixture = self.read(self.addr(0x527420), 672 * 12)
        assert actual_fixture == fixture, 'all fourteen reference model layouts / initializer bytes'
        assert self.read(self.addr(0x5273C0), 12 * 8) == presets, 'rotation preset bytes'
        self.u.mem_write(self.addr(0x8313C8), bytes([0xA5]) * 384)
        self.allowed = [(self.addr(0x8313C8), 384)]
        expected, trace = bytearray(), []
        for part in range(12):
            angles = struct.unpack_from('<4H', presets, part * 8)
            for vertex in range(4):
                v = struct.unpack_from('<3i', fixture, (model * 48 + part * 4 + vertex) * 12)
                transformed = (signed(v[0] + angles[0]), signed(v[1] - angles[1]), signed(v[2] + angles[2]))
                trace += [('reference', 0, v, angles), ('project', transformed)]
                expected += struct.pack('<2i', signed(transformed[0] + transformed[2]), signed(transformed[1] - transformed[2]))
        actual = self.invoke_guarded(0x50F120, [model])
        assert actual == h, 'projection heap guards'
        assert self.trace == trace, 'reference traversal / provider order'
        assert self.read(self.addr(0x8313C8), 384) == expected, 'twelve projected quad records'
        return actual, bytes(expected), trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    images = (Records(ROOT / 'cmr2bin/CMR2.exe'), Records(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities))
    images[0].reset()
    fixture = images[0].read(0x527420, 672 * 12)
    presets = images[0].read(0x5273C0, 96)
    cases = []
    for mode, radius, value in itertools.product((0, 1, 2, 255), (0, 1, 127, 255), (-128, -127, -1, 0, 1, 127)):
        payload = bytes([255, mode, radius] + [value & 255, 0, 127, 255, value & 255, 128, 127, value & 255, 0])
        for preview in (False, True):
            cases.append(('decoder', (preview, payload)))
    rng = random.Random(79241)
    for _ in range(250):
        for preview in (False, True):
            cases.append(('decoder', (preview, rng.randbytes(12))))
    profiles = [((0, 0, 0), (65536, -65536, 32768), (-131072, 127, 0), 655360, 65536, 1),
                ((0x7FFFFFFF, -0x80000000, -655360), (-0x7FFFFFFF, 65535, 0), (1234567, -999999, 65536), -65536, -1, 0),
                ((655360, -655360, 8388608), (0, 0, 65536), (65536, 0, 0), 0x7FFFFFFF, 0x7FFFFFFF, 2),
                ((77777, -55555, 123), (-100, 32768, 65536), (65536, -65536, 127), 123456, 32768, 255)]
    for count in (0, 1, 7, 19, 20):
        for args in itertools.product((count,), (0, 1, -1), range(8) if count == 20 else (0,), profiles, (0, 7)):
            cases.append(('writer', args))
    for index in (0, 1, 7):
        cases.append(('export', (index,)))
    targets = [(0, 0, 0, -1234), (1024, -1024, 2048, 1234), (-2048, 2048, -2049, -32768),
               (-32768, 32767, 4095, 32767)]
    for args in itertools.product((0, 3), targets, (0, 1, -1), (17, 0xFFFFFFF8), (0, 1, 10, 20, 21, 50), (0, 1, 4)):
        cases.append(('orientation', args))
    for model in range(14):
        cases.append(('projection', (model, fixture, presets)))
    for i, (method, args) in enumerate(cases):
        try:
            a, b = [getattr(image, method)(*args, 833 + i) for image in images]
            assert a == b, 'original / rebuilt'
        except Exception as exc:
            raise AssertionError((method, args, i)) from exc
    print(f'{len(cases)} impact/preview record cases: 0 differences; independent encoder/decoder and pool, '
          'snapshot alignment guards, signed orientation lifecycles, all fourteen reference models, full heap/global records, '
          'provider order and ABI')


if __name__ == '__main__':
    main()
