#!/usr/bin/env python3
"""Compare event drawing's early exits and stdcall stack cleanup.

Usage: differential_event_draw_abi.py entities.json [rebuilt.exe]
The original accepts an unused four-byte argument. No drawing leaves execute
in these fixtures; active drawing logic remains outside this harness's scope.
"""
import json
from pathlib import Path
import sys

import capstone
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, STACK
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX


class EventDraw(Lighting):
    def __init__(self, path, entities=None, mutant=False):
        super().__init__(path, entities)
        self.callbacks = {}
        self.u.mem_write(self.base, bytes(self.memory))
        if mutant:
            address = self.addr(0x46E780)
            instructions = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
            found = False
            for instruction in instructions.disasm(self.read(address, 1024), address):
                if instruction.mnemonic == "ret" and instruction.op_str == "4":
                    self.u.mem_write(instruction.address, bytes.fromhex("c20000"))
                    found = True
                    break
            assert found, "The corrected function must pop its unused argument"

    def run_exit(self, events, draws):
        self.put(self.addr(0x58931C), "<i", events)
        self.put(self.addr(0x589318), "<i", draws)
        self.u.mem_write(STACK, bytes([0xA5]) * 0x10000)
        self.invoke(0x46E780, [0])
        return (
            self.u.reg_read(UC_X86_REG_ESP) - (STACK + 0xFF00),
            self.read(self.addr(0x58931C), 4),
            self.read(self.addr(0x589318), 4),
        )


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    path = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "build/CMR2.exe"
    original = EventDraw(ROOT / "cmr2bin/CMR2.exe")
    rebuilt = EventDraw(path, entities)
    cases = [(-1, -1), (0, 0), (0, 1), (1, 0), (3, -1)]
    for case in cases:
        left, right = original.run_exit(*case), rebuilt.run_exit(*case)
        if left != right:
            print("FAIL event draw ABI", case, "original", left, "rebuilt", right)
            return 1
        assert left[0] == 8
    mutant = EventDraw(path, entities, mutant=True)
    assert mutant.run_exit(0, 0) != original.run_exit(0, 0)
    for address in (0x4057A8, 0x4057AB):
        for initial in (0, 1, 0xFFFFFFFF):
            for image in (original, rebuilt):
                image.u.reg_write(UC_X86_REG_EAX, initial)
                image.invoke(address, [])
                assert image.u.reg_read(UC_X86_REG_ESP) == STACK + 0xFF00 + 4
                assert image.u.reg_read(UC_X86_REG_EAX) == (
                    0 if address == 0x4057A8 else initial
                )
    print(
        f"{len(cases)} event draw exits and 6 release no-op cases: identical globals, returns and stack cleanup; old-signature mutation detected"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
