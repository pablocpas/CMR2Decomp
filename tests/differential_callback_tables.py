#!/usr/bin/env python3
"""Execute callback promotion against the game's actual transition tables.

Fixtures use each image's own tables, rather than replacement heap tables.
Check complete table bytes, promotion results, guarded slots and table-read
bounds, including absent rules and wildcard/current-state transitions.
Usage: differential_callback_tables.py entities.json [rebuilt.exe]
"""
import itertools
import json
from pathlib import Path
import sys

from unicorn import UC_HOOK_MEM_READ
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ESP
from differential_menu_list import ROOT, HEAP, STACK
from differential_stage_lighting import Lighting

TABLES = ((0x519120, 26, 14), (0x526F18, 10, 7), (0x523C18, 16, 10))


class Rules(Lighting):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        self.table_bounds = None
        self.u.hook_add(UC_HOOK_MEM_READ, self.read_guard)

    def read_guard(self, u, access, address, size, value, data):
        if self.table_bounds and self.base <= address < self.base + self.size:
            low, high = self.table_bounds
            if not low <= address or address + size > high:
                self.bad_reads.append((address, size))

    def run_rule(self, table, words, state, entry_value, value, previous_level, level):
        self.reset()
        self.put(HEAP, '<B3x3I', 1, HEAP + 0x1000, 0, self.addr(table))
        self.put(HEAP + 0x1000, '<2I',
                 state | entry_value << 8 | previous_level << 24, 123)
        self.table_bounds = self.addr(table), self.addr(table) + words * 4
        self.bad_reads = []
        self.invoke(0x49C1C0, [HEAP, 0, value, level])
        self.table_bounds = None
        assert self.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 20
        if self.bad_reads:
            raise AssertionError(f'transition lookup read beyond table: {self.bad_reads[:3]}')
        return self.u.reg_read(UC_X86_REG_EAX), self.read(HEAP + 0x10, 0x2000 - 0x10)


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = Rules(ROOT / 'cmr2bin/CMR2.exe')
    rebuilt = Rules(Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / 'build/CMR2.exe', entities)
    total = 0
    for table, words, states in TABLES:
        before = original.memory[table - original.base:table - original.base + words * 4]
        address = rebuilt.addr(table)
        after = rebuilt.memory[address - rebuilt.base:address - rebuilt.base + words * 4]
        if before != after:
            print('FAIL callback table', hex(table), 'incomplete rules or missing terminator')
            return 1
        cases = itertools.product((*range(states), 0xFF), (0, 1, 2, 3, 4, 0xFF),
                                  (*range(9), 0xFF), (0, 3), (1, 2))
        for case in cases:
            a = original.run_rule(table, words, *case)
            b = rebuilt.run_rule(table, words, *case)
            if a != b:
                print('FAIL callback transition', hex(table), case)
                return 1
            total += 1
    print(f'{total} callback transitions: complete actual tables, bounded rule reads, guarded slots and promotion results identical')
    return 0


if __name__ == '__main__':
    sys.exit(main())
