#!/usr/bin/env python3
"""Run original and rebuilt 0x48df50 machine code against the same car records.

Usage: python3 tests/differential_slip.py /tmp/cmr2-review-fixed.json
Requires a current MSVC6 build, reccmp JSON, capstone, pefile and Wine.
Only the function's absolute global references are redirected. Relative branches
and arithmetic execute unchanged. The test also mutates the rebuilt damping
constant and requires that the same cases detect the mutation.
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


ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT.parent / "tools"
GLOBAL_SLOT = 0x21000000


def extract(path, address):
    pe = pefile.PE(str(path))
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 4096)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    code = bytearray(data)
    globals_used = set()
    mutation = None
    for ins in md.disasm(data, address):
        offset = ins.address - address
        if ins.mnemonic == "call":
            raise RuntimeError("Unexpected external call: isolation needs a hook")
        for operand in ins.operands:
            if operand.type == capstone.x86.X86_OP_MEM:
                mem = operand.mem
                if not mem.base and not mem.index and mem.disp:
                    globals_used.add(mem.disp)
                    struct.pack_into("<I", code, offset + ins.disp_offset, GLOBAL_SLOT)
            elif operand.type == capstone.x86.X86_OP_IMM and operand.imm == 0xf851:
                assert ins.imm_size == 4
                mutation = offset + ins.imm_offset
        if ins.mnemonic == "ret":
            assert ins.op_str == "4", "Expected one stdcall argument"
            assert len(globals_used) == 1, globals_used
            assert mutation is not None, "Damping constant not found"
            return code[:offset + ins.size], mutation
    raise RuntimeError("Function did not terminate within extraction limit")


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <string.h>
typedef void (__stdcall *SlipFn)(void *);
struct GuardedCar { unsigned char before[64], car[0xc24], after[64]; };
static struct GuardedCar initial, original, rebuilt;
static unsigned int seed;
static unsigned int next(void) { seed = seed * 1664525u + 1013904223u; return seed; }
static void setInt(void *p, int offset, int value) { memcpy((char *)p + offset, &value, 4); }
static void *executable(unsigned char *bytes, unsigned int size) {
    void *p = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!p) return 0;
    memcpy(p, bytes, size);
    FlushInstructionCache(GetCurrentProcess(), p, size);
    return p;
}
int main(void) {
    int test, wheel, offset, mismatches = 0, detected = 0;
    static int pressure[] = { -65536, -1, 0, 1, 32768, 65535, 65536, 65537, 131072 };
    void **slot = (void **)VirtualAlloc((void *)0x21000000, 4096,
                                      MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    SlipFn a = (SlipFn)executable(code_original, sizeof(code_original));
    SlipFn b = (SlipFn)executable(code_rebuilt, sizeof(code_rebuilt));
    SlipFn mutant = (SlipFn)executable(code_mutant, sizeof(code_mutant));
    if (!slot || slot != (void **)0x21000000 || !a || !b || !mutant) return 2;
    for (test = 0; test < 6000; test++) {
        seed = test + 123;
        memset(&initial, 0xa5, sizeof(initial));
        for (offset = 0; offset + 4 <= sizeof(initial.car); offset += 4)
            setInt(initial.car, offset, (int)(next() % 262145) - 131072);
        setInt(initial.car, 0x778, pressure[test % 9]);
        for (wheel = 0; wheel < 4; wheel++) {
            setInt(initial.car, 0xbbc + wheel * 4, (test >> wheel) & 1);
            setInt(initial.car, 0x8fc + wheel * 4, (int)(next() % 65537) - 32768);
            setInt(initial.car, 0x270 + wheel * 12, (int)(next() % 262145) - 131072);
            setInt(initial.car, 0x278 + wheel * 12, (int)(next() % 262145) - 131072);
        }
        original = initial; rebuilt = initial;
        a(original.car);
        if (*slot != original.car) return 3;
        b(rebuilt.car);
        if (*slot != rebuilt.car) return 3;
        if (memcmp(&original, &rebuilt, sizeof(original))) {
            if (mismatches++ < 3) printf("mismatch at case %d\n", test);
        }
        if (memcmp(original.before, initial.before, 64) ||
            memcmp(original.after, initial.after, 64) ||
            memcmp(rebuilt.before, initial.before, 64) ||
            memcmp(rebuilt.after, initial.after, 64)) return 4;
        rebuilt = initial;
        mutant(rebuilt.car);
        if (memcmp(&original, &rebuilt, sizeof(original))) detected++;
    }
    printf("6000 cases: %d differences; mutation detected in %d cases\n", mismatches, detected);
    return mismatches || !detected;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entry = next(e for e in report["data"] if int(e["address"], 16) == 0x48df50)
    original, _ = extract(ROOT / "cmr2bin/CMR2.exe", 0x48df50)
    rebuilt, mutation = extract(ROOT / "build/CMR2.exe", int(entry["recomp"], 16))
    mutant = bytearray(rebuilt)
    struct.pack_into("<I", mutant, mutation, 0xf850)
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=str(TOOLS / "wineprefix"))

    def win(path):
        return subprocess.check_output(["winepath", "-w", str(path)], env=env, text=True).strip()

    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin") + ";" + win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-slip-") as tmp:
        source = Path(tmp) / "slip.c"
        arrays = "".join("static unsigned char code_%s[] = {%s};\n" %
                         (name, ",".join(str(x) for x in code))
                         for name, code in (("original", original), ("rebuilt", rebuilt), ("mutant", mutant)))
        source.write_text(arrays + DRIVER)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"),
                        "/nologo", "/O2", "/MD", "slip.c", "/Feslip.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "slip.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
