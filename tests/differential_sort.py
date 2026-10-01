#!/usr/bin/env python3
"""Compare the original/rebuilt rally ordering routine on ties and edge counts.

Usage: python3 tests/differential_sort.py /tmp/reccmp.json
Executes both functions without mocks or global relocations.
"""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

import capstone
import pefile

ROOT = Path(__file__).resolve().parents[1]
TOOLS = Path(os.environ.get("CMR2_TOOLS", ROOT.parent / "tools"))


def extract(path, address):
    pe = pefile.PE(str(path))
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 4096)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    for ins in md.disasm(data, address):
        assert ins.mnemonic != "call", "Unexpected callee"
        for operand in ins.operands:
            if operand.type == capstone.x86.X86_OP_MEM:
                assert operand.mem.disp < 0x400000, "Unexpected global"
        if ins.mnemonic == "ret":
            assert ins.op_str == "0x14", "Expected five stdcall parameters"
            return data[:ins.address + ins.size - address]
    raise RuntimeError("No return in extraction range")


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <string.h>
typedef void (__stdcall *SortFn)(int *, char *, int, int, char);
struct Inputs { unsigned char before[64]; int times[16]; unsigned char middle[64];
                char order[16]; unsigned char after[64]; };
static struct Inputs initial, original, rebuilt;
static unsigned int seed;
static unsigned int next(void) { seed = seed * 1664525u + 1013904223u; return seed; }
static void *executable(unsigned char *bytes, unsigned int size) {
    void *p = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!p) return 0;
    memcpy(p, bytes, size);
    FlushInstructionCache(GetCurrentProcess(), p, size);
    return p;
}
int main(void) {
    int test, pos, j, count, mismatches = 0;
    char value, init;
    SortFn a = (SortFn)executable(code_original, sizeof(code_original));
    SortFn b = (SortFn)executable(code_rebuilt, sizeof(code_rebuilt));
    if (!a || !b) return 2;
    for (test = 0; test < 6000; test++) {
        seed = test + 123;
        count = test % 19 - 2; /* -2 .. 16 */
        init = (char)(test % 3 - 1);
        memset(&initial, 0xa5, sizeof(initial));
        for (pos = 0; pos < 16; pos++) {
            initial.times[pos] = (int)(next() % 65) - 32;
            initial.order[pos] = (char)pos;
        }
        for (pos = 15; pos > 0; pos--) {
            j = next() % (pos + 1);
            value = initial.order[pos];
            initial.order[pos] = initial.order[j]; initial.order[j] = value;
        }
        original = initial; rebuilt = initial;
        a(original.times, original.order, test % 5 - 1, count, init);
        b(rebuilt.times, rebuilt.order, test % 5 - 1, count, init);
        if (memcmp(&original, &rebuilt, sizeof(original))) {
            if (mismatches++ < 3) printf("sort mismatch at case %d\n", test);
        }
        if (memcmp(original.before, initial.before, 64) ||
            memcmp(original.middle, initial.middle, 64) ||
            memcmp(original.after, initial.after, 64) ||
            memcmp(rebuilt.before, initial.before, 64) ||
            memcmp(rebuilt.middle, initial.middle, 64) ||
            memcmp(rebuilt.after, initial.after, 64) ||
            memcmp(original.times, initial.times, sizeof(initial.times)) ||
            memcmp(rebuilt.times, initial.times, sizeof(initial.times))) return 3;
    }
    printf("6000 sorts: %d differences; times and guards intact\n", mismatches);
    return mismatches != 0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entry = next(e for e in report["data"] if int(e["address"], 16) == 0x40d520)
    original = extract(ROOT / "cmr2bin/CMR2.exe", 0x40d520)
    rebuilt = extract(ROOT / "build/CMR2.exe", int(entry["recomp"], 16))
    arrays = "".join("static unsigned char code_%s[] = {%s};\n" %
                     (name, ",".join(str(x) for x in code))
                     for name, code in (("original", original), ("rebuilt", rebuilt)))
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=os.environ.get("WINEPREFIX", str(TOOLS / "wineprefix")))
    def win(path):
        return subprocess.check_output(["winepath", "-w", str(path)], env=env, text=True).strip()
    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin") + ";" + win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-sort-") as tmp:
        (Path(tmp) / "sort.c").write_text(arrays + DRIVER)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "sort.c", "/Fesort.exe"], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "sort.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
