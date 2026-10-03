#!/usr/bin/env python3
"""Compare every in-race mode dispatch, resume timer and emitted transitions.

Usage: differential_race_handler.py entities.json [rebuilt.exe]
Game/rally queries, phase reads/reset/increment and timer out-parameter are real.
Stage teardown, replay/view operations and sound/render actions are controlled;
this tests the handler's dispatch, arguments/order and state, not their bodies.
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


# Typed boundary: number of stack words, then optional narrow arguments.
ACTIONS = {
    0x49D940: (4, ()), 0x4A2F00: (0, ()), 0x427660: (0, ()),
    0x40A230: (1, ()), 0x4CF140: (0, ()), 0x403500: (0, ()),
    0x40D010: (0, ()), 0x4057E0: (1, (0,)), 0x4068E0: (1, (0,)),
    0x49C1C0: (4, ()), 0x408290: (0, ()), 0x406820: (0, ()),
    0x41E670: (0, ()), 0x41B300: (0, ()), 0x408390: (0, ()),
    0x409B60: (0, ()), 0x409E30: (2, (0, 1)), 0x41E6B0: (3, (2,)),
    0x420100: (0, ()), 0x4B7910: (0, ()), 0x418F20: (0, ()),
    0x41F250: (0, ()), 0x4069C0: (0, ()), 0x472CA0: (0, ()),
    0x406960: (1, (0,)), 0x4A2B50: (1, ()), 0x469B50: (1, ()),
    0x4CF260: (0, ()), 0x4067C0: (1, (0,)), 0x409AB0: (2, (0, 1)),
    0x4728C0: (0, ()), 0x46D2A0: (1, ()), 0x46CC60: (1, ()),
    0x466080: (0, ()), 0x4660A0: (3, (2,)),
    # External selection providers; their implementation is outside this fixture.
    0x4071C0: (2, (0, 1)), 0x4728D0: (0, ()), 0x4729F0: (0, ()),
    0x4072D0: (0, ()), 0x4A9B80: (0, ()),
}
PLAYERS = HEAP + 0x1000


class RaceHandler(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.dispatch_indices = {a: set() for a in (0x41E988, 0x41ECAA, 0x41ECC4, 0x41EE7A, 0x41EF55)}
        for address, (nargs, narrow) in ACTIONS.items():
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs, b=narrow: self.provider(a, n, b))
        self.u.mem_write(self.base, bytes(self.memory))
        self.initial_image = self.read(self.base, self.size)

    def hook(self, u, address, size, data):
        if self.entities is None and address in self.dispatch_indices:
            self.dispatch_indices[address].add(u.reg_read(UC_X86_REG_EAX))
        if address == self.addr(0x406450):
            self.trace.append((0x406450, ('timer_out',)))
        elif address == self.addr(0x41B360):
            self.trace.append((0x41B360, ()))
        elif address == self.addr(0x41B340):
            assert self.args(1)[0] & 255 == 0
            self.trace.append((0x41B340, (0,)))
        elif address == self.addr(0x4074A0):
            self.trace.append((0x4074A0, ()))
        super().hook(u, address, size, data)

    def provider(self, address, nargs, narrow):
        args = list(self.args(nargs))
        for i in narrow:
            args[i] &= 255
        if address == 0x49C1C0:
            assert args[0] == PLAYERS, ('lost player-list pointer', self.case, args)
            assert args[1] < max(self.count, 1), ('invalid view slot', self.case, args)
        if address == 0x41E6B0:
            assert args[0] == PLAYERS
            assert args[1] == self.saved_argument, ('lost saved phase argument', self.case, args)
        if address == 0x4660A0:
            args[0] -= self.addr(0x537F3C)
            assert args[0] == args[1] * 4 and args[2] == (self.car + args[1]) & 255
        self.trace.append((address, tuple(args)))
        value = 0x13579BDF
        if address == 0x427660:
            value = HEAP + 0x4000
        elif address == 0x4071C0:
            value = self.stage_end
        elif address == 0x4728D0:
            value = self.poll
        elif address == 0x4729F0:
            value = self.finished
        elif address == 0x4072D0:
            value = self.restore
        elif address == 0x4A9B80:
            value = self.clocks[self.clock_index]
            self.clock_index += 1
        self.u.reg_write(UC_X86_REG_EAX, value)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def run_handler(self, seed, mode, state, phase, count, argument=0, rejected=-1, stage=None):
        self.case = seed, mode, state, phase, count, argument, rejected, stage
        self.count = count
        self.car = seed % 4
        self.saved_argument = (argument & 0xFFFFFF00) | self.car
        self.poll, self.finished, self.restore = seed & 1, (seed >> 1) & 1, (seed >> 2) & 1
        self.stage_end = seed % 3
        self.clocks = (0x7FFFFFFF + seed, (0xFFFFFFFF - seed) & 0xFFFFFFFF)
        self.clock_index = 0
        rnd = random.Random(seed)
        self.u.mem_write(self.base, self.initial_image)
        self.u.mem_write(STACK, bytes(0x10000))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        # Guards on all globals the real handler and queries can change.
        locations = ((0x537F08, 1), (0x53810C, 1), (0x53810D, 1),
                     (0x537FFA, 1), (0x537FFC, 4), (0x537EF6, 1),
                     (0x537DF0, 4), (0x52F2AC, 4), (0x52EA52, 1))
        for address, size in locations:
            self.u.mem_write(self.addr(address) - 16, rnd.randbytes(size + 32))
        self.u.mem_write(self.addr(0x52AFA0) - 16, rnd.randbytes(0x3900 + 32))
        self.u.mem_write(self.addr(0x537F3C) - 16, rnd.randbytes(64))
        self.put(PLAYERS, '<B', count)
        self.put(PLAYERS + 4, '<I', HEAP + 0x2000)
        for i in range(count):
            self.put(HEAP + 0x2000 + i * 8, '<B', 10 if i == rejected else 11)
        self.put(self.addr(0x537F08), '<B', 0)
        self.put(self.addr(0x53810C), '<B', bool(phase & 1))
        self.put(self.addr(0x53810D), '<B', bool(phase & 2))
        self.put(self.addr(0x537FFA), '<B', (seed >> 1) & 1)
        previous = 0x7FFFFFFD - seed
        self.put(self.addr(0x537FFC), '<I', previous)
        self.put(self.addr(0x537EF6), '<B', self.car)
        self.put(self.addr(0x537DF0), '<I', state)
        stage = seed % 3 if stage is None else stage
        selected = (seed % 3) | (stage << 5) | ((seed % 4) << 14)
        selected |= (seed % 4) << 12 | ((seed >> 1) % 4) << 10
        selected |= (seed & 1) << 25 | ((seed >> 1) & 1) << 26 | ((seed >> 2) & 1) << 28
        self.put(self.addr(0x52F2AC), '<I', selected)
        game = self.addr(0x52AFA0)
        match_car = (seed >> 1) & 1
        players_setting = self.car + 1 if match_car else self.car + 2
        settings = mode << 3 | (seed % 3) << 10 | players_setting << 13
        settings |= (seed & 1) << 17 | ((seed >> 1) & 1) << 19
        self.put(game + 0x14, '<I', settings)
        self.put(game + 0x18, '<I', ((seed >> 2) & 1) << 30)
        gated = seed & 1
        self.put(self.addr(0x52EA52), '<B', gated)
        timer = (1, 0x7FFFFFFF, 0xFFFFFFFE, 0x80000000)[seed % 4]
        self.put(game + 0x38F4, '<I', timer)
        self.put(self.addr(0x537F3C), '<8I', *(HEAP + 0x5000 + i * 0x100 for i in range(8)))
        spans = {a: self.read(self.addr(a) - 16, size + 32) for a, size in locations}
        game_before = self.read(game - 16, 0x3900 + 32)
        replay_before = self.read(self.addr(0x537F3C) - 16, 64)
        heap_before = self.read(HEAP, 0x10000)
        self.trace = []
        self.invoke(0x41E8D0, [PLAYERS, argument])
        assert self.read(HEAP, 0x10000) == heap_before, ('input/heap guard changed', self.case)
        assert self.read(self.addr(0x537F3C) - 16, 64) == replay_before
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 12
        early = argument & 255 != 0 or 0 <= rejected < count
        if early:
            assert self.trace == []
        else:
            prefix = []
            if not seed & 1:
                prefix.append((0x49D940, (1, 156, 180, 172)))
            if (seed >> 1) & 1:
                prefix.append((0x4A2F00, ()))
            if mode == 10:
                prefix.extend(((0x427660, ()), (0x40A230, (HEAP + 0x4000,))))
            prefix.extend(((0x4CF140, ()), (0x403500, ())))
            assert self.trace[:len(prefix)] == prefix, ('pre-dispatch actions', self.case)
        assert self.clock_index in (0, 2)
        expected_game = bytearray(game_before)
        if self.clock_index:
            assert phase & 2 and not gated
            struct.pack_into('<I', expected_game, 16 + 0x38F4,
                             (timer + self.clocks[0] - previous) & 0xFFFFFFFF)
        assert self.read(game - 16, len(expected_game)) == expected_game, ('timer/game guard mismatch', self.case)
        allowed = {0x537F08: bytes([not early])}
        if self.clock_index:
            allowed[0x537FFC] = struct.pack('<I', self.clocks[1])
        if any(a == 0x49C1C0 for a, _ in self.trace) and phase & 2:
            # All resume exits clear this byte; teardown may return earlier.
            if any(a in (0x469B50, 0x406450) for a, _ in self.trace):
                allowed[0x53810D] = b'\0'
        # These actual helpers update one phase byte / mode-5 progress field.
        # Preserve all other bytes with the overlapping-span guard model.
        events = [a for a, _ in self.trace]
        expected_phase = 0 if 0x41B360 in events else self.car
        expected_phase = (expected_phase + events.count(0x41B340)) & 255
        allowed[0x537EF6] = bytes([expected_phase])
        expected_progress = selected
        for _ in range(events.count(0x4074A0)):
            if mode == 5:
                expected_progress = (expected_progress & ~0x3000) | ((expected_progress + 0x1000) & 0x3000)
        allowed[0x52F2AC] = struct.pack('<I', expected_progress)
        if phase & 2 and any(a == 0x469B50 for a, _ in self.trace):
            allowed[0x53810D] = b'\0'
        for address, before in spans.items():
            start = self.addr(address) - 16
            expected = bytearray(before)
            for a, data in allowed.items():
                pointer = self.addr(a)
                lo, hi = max(start, pointer), min(start + len(expected), pointer + len(data))
                if lo < hi:
                    expected[lo - start:hi - start] = data[lo - pointer:hi - pointer]
            assert self.read(start, len(expected)) == expected, ('global guard mismatch', self.case, hex(address))
        state_after = tuple(self.read(self.addr(a), size) for a, size in locations)
        return self.trace, state_after, bytes(expected_game), heap_before


def cases():
    yield from itertools.product(range(8), (*range(13), 15), (*range(5), 9), range(4), (0, 1, 2, 8))
    # Both stage-dependent restore exits, including every even-stage transition.
    for mode, state, stage, count in itertools.product((0, 1), (0, 1, 4), range(12), (0, 2, 8)):
        yield 7, mode, state, 0, count, 0, -1, stage
    # Every rejecting slot and low-byte argument guard, plus accepted upper bits.
    for count in (0, 1, 2, 8):
        for reject in range(count):
            yield 0, 0, 0, 0, count, 0, reject
        for argument in (1, 255, 0x100, 0x12340000):
            yield 1, 2, 4, 1, count, argument
            if argument & 255 == 0:
                yield 1, 6, 9, 1, count, argument


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = RaceHandler(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = RaceHandler(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    count = 0
    for case in cases():
        a, b = original.run_handler(*case), rebuilt.run_handler(*case)
        if a != b:
            print('FAIL race handler', case)
            print('original', a[0])
            print('rebuilt ', b[0])
            return 1
        count += 1
    assert list(map(len, original.dispatch_indices.values())) == [11, 5, 13, 7, 4], original.dispatch_indices
    print(f'{count} race-handler cases: all five mode/state dispatch tables, real phase/game/rally/timer queries, guarded inputs/globals and complete controlled action traces')
    return 0


if __name__ == '__main__':
    sys.exit(main())
