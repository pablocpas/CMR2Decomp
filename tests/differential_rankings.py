#!/usr/bin/env python3
"""Execute original/rebuilt split-ranking removal and rebuild on shuffled tables.

Usage: python3 tests/differential_rankings.py /tmp/reccmp.json [entities.json]
The only callee (split count) returns the count chosen by each case. All table
reads, moves and writes execute the actual machine code, with guards around data.
"""

import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

import capstone
import pefile

from matching_entities import load_entities

ROOT = Path(__file__).resolve().parents[1]
TOOLS = Path(os.environ.get("CMR2_TOOLS", ROOT.parent / "tools"))
BASE = 0x21000000
HOOK = BASE + 0x1000


def extract(path, address, destination, ranges, split_count, ret_bytes):
    pe = pefile.PE(str(path))
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 4096)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    code = bytearray(data)
    calls = 0
    for ins in md.disasm(data, address):
        offset = ins.address - address
        if ins.mnemonic == "call":
            assert ins.operands[0].imm == split_count, ins.op_str
            struct.pack_into("<i", code, offset + ins.imm_offset,
                             HOOK - (destination + offset + ins.size))
            calls += 1
        for operand in ins.operands:
            if operand.type == capstone.x86.X86_OP_MEM:
                disp = operand.mem.disp
                patch_offset = ins.disp_offset
                require_mapping = disp >= 0x400000
            elif operand.type == capstone.x86.X86_OP_IMM and not (
                    ins.mnemonic == "call" or ins.group(capstone.CS_GRP_JUMP)):
                disp = operand.imm
                patch_offset = ins.imm_offset
                require_mapping = False
            else:
                continue
            for start, size, target in ranges:
                if start <= disp < start + size:
                    assert (ins.imm_size if operand.type == capstone.x86.X86_OP_IMM else ins.disp_size) == 4
                    struct.pack_into("<I", code, offset + patch_offset,
                                     target + disp - start)
                    break
            else:
                if require_mapping:
                    raise RuntimeError("Unmapped table address: %#x" % disp)
        if ins.mnemonic == "ret":
            assert ins.op_str == ret_bytes, "Unexpected stdcall parameter count"
            assert calls == 1, calls
            return code[:offset + ins.size]
    raise RuntimeError("No return in extraction range")


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <string.h>
typedef void (__stdcall *RemoveFn)(int, int);
typedef void (__stdcall *RebuildFn)(void);
static unsigned int seed;
static unsigned int next(void) { seed = seed * 1664525u + 1013904223u; return seed; }
static unsigned char initial[0x300], result[0x300];
int main(void) {
    int mode, test, split, pos, j, count, driver, slot, splits, activeCount, mismatches = 0;
    int *splitCount;
    unsigned char *memory = (unsigned char *)VirtualAlloc((void *)0x21000000,
        0x6000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    RemoveFn a = (RemoveFn)0x21002000, b = (RemoveFn)0x21003000;
    RebuildFn ra = (RebuildFn)0x21004000, rb = (RebuildFn)0x21005000;
    static unsigned char hook[] = {0xa1,0x40,0x02,0x00,0x21,0xc3};
    if (memory != (unsigned char *)0x21000000) return 2;
    memcpy(memory + 0x1000, hook, sizeof(hook));
    memcpy(memory + 0x2000, code_original, sizeof(code_original));
    memcpy(memory + 0x3000, code_rebuilt, sizeof(code_rebuilt));
    memcpy(memory + 0x4000, code_rebuild_original, sizeof(code_rebuild_original));
    memcpy(memory + 0x5000, code_rebuild_rebuilt, sizeof(code_rebuild_rebuilt));
    FlushInstructionCache(GetCurrentProcess(), memory + 0x1000, 0x5000);
    splitCount = (int *)(memory + 0x240);
    for (mode = 0; mode < 2; mode++) {
    for (test = 0; test < 6000; test++) {
        seed = test + 123;
        splits = test % 10; /* Includes the zero-split path. */
        activeCount = test % 17;
        driver = test % 16;
        slot = test % 8;
        memset(initial, 0xa5, sizeof(initial));
        for (split = 0; split < 10; split++) {
            unsigned char *indices = initial + 0x40 + split * 16;
            unsigned char *positions = indices + 0xa0;
            unsigned char *times = indices + 0x140;
            count = mode ? activeCount : 1 + next() % 16;
            for (pos = 0; pos < 16; pos++) indices[pos] = times[pos] = pos;
            for (pos = 15; pos > 0; pos--) {
                unsigned char temp;
                j = next() % (pos + 1);
                temp = indices[pos]; indices[pos] = indices[j]; indices[j] = temp;
                if (!mode || pos < activeCount) {
                    j = next() % (pos + 1);
                    temp = times[pos]; times[pos] = times[j]; times[j] = temp;
                }
            }
            for (pos = 0; pos < 16; pos++) positions[indices[pos]] = pos;
            /* The removed driver must belong to every active ranking. */
            pos = positions[driver];
            if (!mode && pos >= count) {
                j = next() % count;
                indices[pos] = indices[j]; indices[j] = driver;
                positions[indices[pos]] = pos; positions[driver] = j;
            }
            initial[0x40 + 0x1e0 + split] = count;
        }
        memcpy(initial + 0x240, &splits, 4);
        memcpy(initial + 0x248, &activeCount, 4);
        memcpy(memory, initial, sizeof(initial));
        if (mode) ra(); else a(slot, driver);
        memcpy(result, memory, sizeof(result));
        memcpy(memory, initial, sizeof(initial));
        if (mode) rb(); else b(slot, driver);
        if (memcmp(result, memory, sizeof(result))) {
            if (mismatches++ < 3) printf("ranking mismatch in mode %d at case %d\n", mode, test);
        }
        /* The ranking block and eight removed-driver slots are the only outputs. */
        for (pos = 0; pos < sizeof(initial); pos++) {
            if ((pos >= 0x40 && pos < 0x40 + 0x1ec) || (!mode && pos >= 0x230 && pos < 0x238)) continue;
            if (result[pos] != initial[pos] || memory[pos] != initial[pos]) return 3;
        }
        if (*splitCount != splits) return 4;
    }
    }
    printf("6000 ranking removals and 6000 rebuilds: %d differences; data guards intact\n", mismatches);
    return mismatches != 0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = load_entities(sys.argv[2] if len(sys.argv) > 2 else None)
    entry = next(e for e in report["data"] if int(e["address"], 16) == 0x455bc0)
    assert entities["0x455bc0"][0] == int(entry["recomp"], 16), "Stale entity map"
    original_ranges = [(0x541fac, 0x1ec, BASE + 0x40),
                       (0x541f90, 8, BASE + 0x230),
                       (0x541f98, 4, BASE + 0x248)]
    rebuilt_ranges = [(entities[hex(start)][0], size, target)
                      for start, size, target in original_ranges]
    original = extract(ROOT / "cmr2bin/CMR2.exe", 0x455bc0, BASE + 0x2000,
                       original_ranges, 0x4583a0, "8")
    rebuilt = extract(ROOT / "build/CMR2.exe", int(entry["recomp"], 16), BASE + 0x3000,
                      rebuilt_ranges, entities["0x4583a0"][0], "8")
    rebuild_entry = next(e for e in report["data"] if int(e["address"], 16) == 0x456250)
    assert entities["0x456250"][0] == int(rebuild_entry["recomp"], 16), "Stale entity map"
    rebuild_original = extract(ROOT / "cmr2bin/CMR2.exe", 0x456250, BASE + 0x4000,
                               original_ranges, 0x4583c0, "")
    rebuild_rebuilt = extract(ROOT / "build/CMR2.exe", int(rebuild_entry["recomp"], 16),
                              BASE + 0x5000, rebuilt_ranges, entities["0x4583c0"][0], "")
    arrays = "".join("static unsigned char code_%s[] = {%s};\n" %
                     (name, ",".join(str(x) for x in code))
                     for name, code in (("original", original), ("rebuilt", rebuilt),
                                        ("rebuild_original", rebuild_original),
                                        ("rebuild_rebuilt", rebuild_rebuilt)))
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=os.environ.get("WINEPREFIX", str(TOOLS / "wineprefix")))
    def win(path):
        return subprocess.check_output(["winepath", "-w", str(path)], env=env, text=True).strip()
    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin") + ";" + win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-rankings-") as tmp:
        (Path(tmp) / "rankings.c").write_text(arrays + DRIVER)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "rankings.c", "/Ferankings.exe"], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "rankings.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
