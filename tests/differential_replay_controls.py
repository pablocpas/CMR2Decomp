#!/usr/bin/env python3
"""Compare replay control-frame dispatch and phase events with the original.

Usage: differential_replay_controls.py entities.json [rebuilt.exe]
Car lookup, packet decoding and buffer reset are controlled providers. Event
selection, packet/lane advancement and all replay-state writes run in both images.
"""
import itertools
import json
from pathlib import Path
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting


class Controls(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in ((0x42B5F0, 1), (0x46C4B0, 4), (0x46D2A0, 1)):
            self.callbacks[self.addr(address)] = (nargs * 4,
                lambda a=address, n=nargs: self.provider(a, n))

    def provider(self, address, nargs):
        args = self.args(nargs)
        self.trace.append((address, args))
        result = HEAP + 0x1000 if address == 0x42B5F0 else 0
        if address == 0x46C4B0:
            self.put(args[3], '<B', self.counter)
            result = self.advance
        self.u.reg_write(UC_X86_REG_EAX, result)
        self.u.reg_write(UC_X86_REG_ECX, 0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5A5A5A5A)

    def run_frame(self, kind, lane, count, frame, counter, advance, end):
        self.reset()
        self.counter, self.advance = counter, advance
        car, replay, inputs, states, frames, samples = (
            HEAP + n for n in (0x1000, 0x3000, 0x4000, 0x7000, 0x8000, 0x9000))
        self.put(car + 0xB43, '<B', 1)
        self.put(replay + 4, '<2I', 1, 1)
        self.put(replay + 0x1C, '<I', kind)
        self.put(replay + 0x20, '<B', 3)
        self.put(replay + 0x24, '<I', inputs)
        self.put(replay + 0x30, '<I', states)
        self.put(replay + 0x3C, '<I', frames)
        self.put(replay + 0xFE, '<2h', 32, lane + 1)
        self.put(replay + 0x104, '<I', samples)
        self.put(replay + 0x108, '<2hB', lane, frame, 0xEE)
        self.put(samples + lane * 2, '<h', frame + advance if end else 32)
        table = inputs + lane * 0x114C if kind == 0 else states + lane * 0x5C
        events = table + (0x110C if kind == 0 else 0x1C)
        self.u.mem_write(events, bytes([0xCD]) * 60)
        self.put(table + (0x1148 if kind == 0 else 0x58), '<B', count)
        for k in range(count):
            self.put(events + k * 6, '<hhBB', k * 3, k % 4, 7 + k, 0)
        self.invoke(0x46CFA0, [replay])
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 8
        return self.read(HEAP, 0x10000), self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Controls(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Controls(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    total = 0
    cases = itertools.product((0, 1), (0, 1), (0, 1, 2, 3, 10),
                              (0, 1, 2, 3, 4, 6, 9, 27, 30), (0, 1, 3, 63), (0, 1), (0, 1))
    for case in cases:
        a, b = original.run_frame(*case), rebuilt.run_frame(*case)
        if a != b:
            print('FAIL replay controls', case,
                  'phase original/rebuilt:', a[0][0x310C], b[0][0x310C])
            return 1
        total += 1
    print(f'{total} replay control ticks: identical phase events, state and provider traces')
    return 0


if __name__ == '__main__':
    sys.exit(main())
