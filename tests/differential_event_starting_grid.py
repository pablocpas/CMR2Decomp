#!/usr/bin/env python3
"""Compare all four event starting-grid modes and their provider call order.

Relocate the complete candidate, including its switch table, so no candidate
case falls back to original code. Cover local/network grids, cached/random
starts and all early exits, with guarded state and the calling convention.
"""
import argparse
import math
from pathlib import Path
import random
import re
import struct

from differential_car_forces import F, SCRATCH
from differential_car_body_matrix import SAVED
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                              UC_X86_REG_EIP, UC_X86_REG_ESP, UC_X86_REG_EFLAGS)

ADDRESS = 0x4584d0


def grid_body(obj):
    full, bodies = [], []
    strip, dis = F.strip_pad, F.dis
    def capture_full(code):
        full.append(bytes(code))
        return strip(code)
    def capture_body(code, address):
        bodies.append(bytes(code))
        return dis(code, address)
    F.strip_pad, F.dis = capture_full, capture_body
    coff = F.COFF(str(obj))
    try:
        names, sizes = F.load_meta()
        _, _, _, unknown = F.compare(ADDRESS, str(obj), str(ROOT / 'CMR2Decomp/StageTiming.cpp'),
                                     names[ADDRESS], sizes[ADDRESS])
        assert not unknown, unknown
    finally:
        F.strip_pad, F.dis = strip, dis
    code = bytearray(strip(full[0]))
    _, symbol = F.find_func(coff, 'StageTiming_PlaceEventStartingGrid')
    start, end = F.func_extent(coff, symbol)
    section = coff.secs[symbol['sec'] - 1]
    for offset, _, kind in section['rels']:
        if kind == 6 and start <= offset < end:
            at = offset - start
            value, = struct.unpack_from('<I', code, at)
            if ADDRESS <= value < ADDRESS + len(code):
                struct.pack_into('<I', code, at, SCRATCH + value - ADDRESS)
    calls = []
    for ins in F.md.disasm(bodies[-1], ADDRESS):
        if ins.bytes[0] == 0xe8:
            target = ins.address + 5 + struct.unpack('<i', ins.bytes[1:])[0]
            at = ins.address - ADDRESS
            struct.pack_into('<i', code, at + 1, target - (SCRATCH + at + 5))
            calls.append((at, target))
    return bytes(code), calls


class Grid(Drawing):
    def __init__(self, code=None):
        super().__init__(ROOT / 'cmr2bin/CMR2.exe')
        self.code = code
        self.u.mem_map(SCRATCH, 0x10000)
        if code is not None:
            self.u.mem_write(SCRATCH, code)
        names, _ = F.load_meta()
        addresses = {name: address for address, name in names.items()}
        def bind(name, count, action):
            self.callbacks[addresses[name]] = (4*count, action)
        self.callbacks = {}
        bind('RallyData_GetFlag31', 0, lambda: self.query('flag31', 0, self.mode == 2))
        bind('CGameInfo::GetGameModeOptionBit19', 0, lambda: self.query('bit19', 0, self.mode == 3))
        bind('RallyDataStageIndex', 0, lambda: self.query('stage', 0, 10))
        bind('CGameInfo::GetConfiguredGameMode', 0, lambda: self.query('game-mode', 0, self.game_mode))
        bind('RallyData_GetFlag24', 0, lambda: self.query('flag24', 0, self.mode not in (1, 6)))
        bind('RallyData_GetSelectionFlag27', 0, lambda: self.query('flag27', 0, self.mode != 6))
        bind('RallyData_GetRouteAvailabilityState', 0, lambda: self.query('route-count', 0, 42))
        bind('RallyData_GetRouteNodeGroundPosition', 2, self.position)
        bind('StageObject_Atan2Degrees', 2, lambda: self.query('atan', 2, (self.seed * 37) % 360))
        bind('RallyDataState', 0, lambda: self.query('players', 0, self.locals))
        bind('RallyTiming_GetChampionshipDriverPosition', 1, self.championship)
        bind('CMain::GetFrameTime', 0, lambda: self.query('clock', 0, 1000 + self.seed))
        bind('NetPlayers_GetPlayerIDCount', 0, lambda: self.query('network-count', 0, self.net_count))
        bind('NetPlayers_GetSortedPlayerID', 1, self.sorted_id)
        bind('Network_GetLocalPlayerID', 0, lambda: self.query('local-id', 0, 1000))
        bind('NetPlayers_FindPlayerIndexByID', 1, self.find_id)
        bind('NetPlayers_GetPlayerField8', 1, self.car_id)
        self.callbacks[0x4c6676] = (0, lambda: self.query('rand', 0, (0, 16383, 16384, 32767)[self.seed % 4]))
        self.callbacks[0x4c6682] = (0, lambda: self.query('srand', 1, 0))
        self.executed = set()

    def hook(self, u, address, size, data):
        if self.code is not None:
            assert not ADDRESS <= address < 0x4589d4, 'candidate entered original grid case'
            if SCRATCH <= address < SCRATCH + len(self.code):
                self.executed.add(address)
        super().hook(u, address, size, data)

    def query(self, kind, count, value):
        self.trace.append((kind, *self.args(count)))
        self.u.reg_write(UC_X86_REG_EAX, int(value) & 0xffffffff)
        self.u.reg_write(UC_X86_REG_ECX, 0x12345678)
        self.u.reg_write(UC_X86_REG_EDX, 0x87654321)

    def position(self):
        index, dest = self.args(2)
        values = (self.p0, self.p1)[index == 0]
        self.put(dest, '<3i', *values)
        self.trace.append(('position', index, values))
        self.u.reg_write(UC_X86_REG_EAX, dest)
        self.u.reg_write(UC_X86_REG_ECX, 0x12345678)
        self.u.reg_write(UC_X86_REG_EDX, 0x87654321)

    def championship(self):
        i, = self.args(1)
        self.query('championship', 1, self.count - 1 - i)

    def sorted_id(self):
        i, = self.args(1)
        self.query('sorted-id', 1, self.ids[i])

    def find_id(self):
        id_, = self.args(1)
        self.query('find-id', 1, id_ - 1000)

    def car_id(self):
        i, = self.args(1)
        self.query('car-id', 1, i + 1)

    def run(self, seed):
        self.seed, self.mode = seed, seed % 7
        rnd = random.Random(seed)
        self.locals = 1 + (seed // 7) % 2
        self.count = 2 if self.locals == 2 or self.mode in (1, 6) else rnd.choice((2, 3, 4, 7))
        if self.mode == 4:
            self.count = 1
        self.game_mode = 10 if self.mode == 5 else (5 if seed % 3 == 0 else 0)
        self.net_count = 1 + (seed // 7) % 7
        self.ids = list(range(1000, 1007))
        rnd.shuffle(self.ids)
        self.p0, self.p1 = tuple(tuple(rnd.randrange(-500000, 500001) for _ in range(3)) for _ in range(2))
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, rnd.randbytes(0x10000))
        self.u.mem_write(STACK, rnd.randbytes(0x10000))
        self.u.mem_write(0x542c60, rnd.randbytes(0x160))
        self.put(0x542c68, '<i', self.count)
        self.put(0x542cd4, '<i', seed % 2)
        self.put(0x542c80, '<i', 77)
        self.put(0x6e2ef4, '<4096i', *(round(math.sin(i*math.tau/4096)*65536) for i in range(4096)))
        for register in (UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, *SAVED):
            self.u.reg_write(register, rnd.randrange(0x100000000))
        saved = tuple(self.u.reg_read(r) for r in SAVED)
        self.u.reg_write(UC_X86_REG_EFLAGS, 2)
        sp = STACK + 0xff00
        self.put(sp, '<2I', STOP, seed % 2 | rnd.randrange(0x1000000) << 8)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(ADDRESS if self.code is None else SCRATCH, STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP, 'grid did not return'
        assert self.u.reg_read(UC_X86_REG_ESP) == sp + 8, 'stack was not restored'
        assert tuple(self.u.reg_read(r) for r in SAVED) == saved, 'callee-saved registers changed'
        return self.read(HEAP, 0x10000), self.read(0x542c60, 0x160), self.trace


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', type=Path)
    parser.add_argument('--cases', type=int, default=700)
    args = parser.parse_args()
    if args.cases < 1:
        parser.error('--cases must be positive')
    code, calls = grid_body(args.object)
    original, rebuilt = Grid(), Grid(code)
    for seed in range(args.cases):
        if original.run(seed) != rebuilt.run(seed):
            print(f'FAIL event starting grid seed {seed}: {original.trace} / {rebuilt.trace}')
            return 1
    # Omit the second position query in network mode while preserving its ABI.
    table = next(ins for ins in F.md.disasm(code, SCRATCH) if ins.mnemonic == 'jmp' and 'ptr [' in ins.op_str)
    table_address = int(re.search(r'0x[0-9a-f]+', table.op_str)[0], 16)
    case2, case3 = struct.unpack_from('<2I', code, table_address - SCRATCH + 8)
    at = [at for at, target in calls if target == 0x421530 and case2 <= SCRATCH + at < case3][1]
    damaged = bytearray(code)
    damaged[at:at + 5] = bytes.fromhex('83 c4 08 90 90')
    negative = Grid(bytes(damaged))
    assert original.run(2) != negative.run(2), 'negative control passed'
    assert any(SCRATCH < a < case2 for a in rebuilt.executed), 'local case did not execute'
    assert any(case2 <= a < case3 for a in rebuilt.executed), 'network case did not execute'
    assert any(a >= case3 for a in rebuilt.executed), 'stage-10 case did not execute'
    print(f'PASS event starting grid: {args.cases} cases, all modes, provider order, guarded state and ABI identical; switch cases executed and negative control detected')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
