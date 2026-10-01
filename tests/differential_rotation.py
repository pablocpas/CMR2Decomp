#!/usr/bin/env python3
"""Compare original/rebuilt SceneNode_Rotate machine code, including scratch data.

Usage: python3 tests/differential_rotation.py /tmp/current-reccmp.json [entities.json]
Requires MSVC6, Wine, capstone and pefile. No game calls are mocked: this
function has no callees. Only its global addresses are relocated.
"""

import json
import math
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
SIN = BASE
SQRT = BASE + 0x4000
SCRATCH = BASE + 0x8000
GLOBALS = {
    0x6e2ef4: SIN,
    0x6e0ef4: SQRT,
}
for index, address in enumerate((0x67f250, 0x67f248, 0x68336c, 0x67f254,
                                 0x67f258, 0x67f25c, 0x67f238, 0x67f244,
                                 0x67f24c)):
    GLOBALS[address] = SCRATCH + index * 4


def extract(path, address, replacements):
    pe = pefile.PE(str(path))
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 16384)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    code = bytearray(data)
    seen = set()
    for ins in md.disasm(data, address):
        offset = ins.address - address
        if ins.mnemonic == "call":
            raise RuntimeError("Unexpected callee: relocation requires a hook")
        for operand in ins.operands:
            if operand.type == capstone.x86.X86_OP_MEM:
                disp = operand.mem.disp
                if disp >= 0x400000:
                    if disp not in replacements:
                        raise RuntimeError("Unmapped global: %#x" % disp)
                    struct.pack_into("<I", code, offset + ins.disp_offset,
                                     replacements[disp])
                    seen.add(replacements[disp])
        if ins.mnemonic == "ret":
            assert ins.op_str == "0xc", "Expected three stdcall parameters"
            assert seen == set(GLOBALS.values()), seen
            return code[:offset + ins.size]
    raise RuntimeError("No return in the extraction range")


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <string.h>
typedef void (__stdcall *RotateFn)(void *, void *, void *);
struct GuardedNode { unsigned char before[64], node[0x18c], after[64]; };
static struct GuardedNode initial[3], original[3], rebuilt[3];
static unsigned int seed;
static unsigned int next(void) { seed = seed * 1664525u + 1013904223u; return seed; }
static void putInt(void *p, int offset, int value) { memcpy((char *)p + offset, &value, 4); }
static void *executable(unsigned char *bytes, unsigned int size) {
    void *p = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!p) return 0;
    memcpy(p, bytes, size);
    FlushInstructionCache(GetCurrentProcess(), p, size);
    return p;
}
static void parents(struct GuardedNode *nodes, int count) {
    int n;
    for (n = 0; n < 3; n++)
        putInt(nodes[n].node, 8, n + 1 < count ? (int)nodes[n+1].node : 0);
}
static void basis(unsigned char *node, int mode) {
    int axis, component;
    static int permutations[3][3] = {{0,1,2}, {1,2,0}, {2,0,1}};
    for (axis = 0; axis < 3; axis++)
        for (component = 0; component < 3; component++) {
            int value = component == permutations[mode % 3][axis] ? 65536 : 0;
            if (mode >= 3 && axis != 2) value = -value;
            putInt(node, 0x98 + axis * 16 + component * 4, value);
        }
}
int main(void) {
    int test, node, offset, component, count, mismatches = 0;
    int translation[3], scratchA[9];
    short angles[3], angleCopy[3];
    unsigned char translationCopy[12];
    static short limits[] = {0,1,-1,0x100,0x400,0x800,0xfff,0x1000,32767,-32768};
    unsigned char *globals = (unsigned char *)VirtualAlloc((void *)0x21000000,
        0x9000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    RotateFn a = (RotateFn)executable(code_original, sizeof(code_original));
    RotateFn b = (RotateFn)executable(code_rebuilt, sizeof(code_rebuilt));
    if (globals != (unsigned char *)0x21000000 || !a || !b) return 2;
    memcpy(globals, sin_values, sizeof(sin_values));
    memcpy(globals + 0x4000, sqrt_values, sizeof(sqrt_values));
    for (test = 0; test < 6000; test++) {
        seed = test + 123;
        count = 1 + test % 3;
        memset(initial, 0xa5, sizeof(initial));
        for (node = 0; node < 3; node++) {
            for (offset = 0; offset + 4 <= sizeof(initial[node].node); offset += 4)
                putInt(initial[node].node, offset, (int)(next() % 131073) - 65536);
            basis(initial[node].node, test % 6);
        }
        for (component = 0; component < 3; component++) {
            translation[component] = (int)(next() % 131073) - 65536;
            angles[component] = test < 1000 ? limits[(test / (component == 0 ? 1 : component == 1 ? 10 : 100)) % 10] : (short)next();
        }
        memcpy(original, initial, sizeof(initial));
        memcpy(rebuilt, initial, sizeof(initial));
        parents(original, count); parents(rebuilt, count);
        memcpy(angleCopy, angles, sizeof(angles));
        memcpy(translationCopy, translation, sizeof(translation));
        memset(globals + 0x8000, 0, sizeof(scratchA));
        a(original[0].node, translation, angles);
        memcpy(scratchA, globals + 0x8000, sizeof(scratchA));
        memset(globals + 0x8000, 0, sizeof(scratchA));
        b(rebuilt[0].node, translation, angles);
        /* Check links before normalizing the addresses of the parent objects. */
        for (node = 0; node < 3; node++) {
            int linkA, linkB;
            memcpy(&linkA, original[node].node + 8, 4);
            memcpy(&linkB, rebuilt[node].node + 8, 4);
            if (linkA != (node + 1 < count ? (int)original[node+1].node : 0) ||
                linkB != (node + 1 < count ? (int)rebuilt[node+1].node : 0)) return 5;
        }
        parents(original, 0); parents(rebuilt, 0);
        if (memcmp(original, rebuilt, sizeof(original)) ||
            memcmp(scratchA, globals + 0x8000, sizeof(scratchA))) {
            if (mismatches++ < 3) printf("rotation mismatch at case %d\n", test);
        }
        if (memcmp(translationCopy, translation, sizeof(translation)) ||
            memcmp(angleCopy, angles, sizeof(angles))) return 3;
        for (node = 0; node < 3; node++) {
            if (memcmp(original[node].before, initial[node].before, 64) ||
                memcmp(original[node].after, initial[node].after, 64) ||
                memcmp(rebuilt[node].before, initial[node].before, 64) ||
                memcmp(rebuilt[node].after, initial[node].after, 64)) return 4;
        }
    }
    printf("6000 rotations: %d differences; node guards and input buffers intact\n", mismatches);
    return mismatches != 0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entry = next(e for e in report["data"] if int(e["address"], 16) == 0x4ac820)
    # The entity map includes scratch globals as well as the table bases.
    entities = load_entities(sys.argv[2] if len(sys.argv) > 2 else None)
    assert entities["0x4ac820"][0] == int(entry["recomp"], 16), "Stale entity map"
    replacements = {int(entities[hex(addr)][0]): target for addr, target in GLOBALS.items()}
    original = extract(ROOT / "cmr2bin/CMR2.exe", 0x4ac820, GLOBALS)
    rebuilt = extract(ROOT / "build/CMR2.exe", int(entry["recomp"], 16), replacements)
    pe = pefile.PE(str(ROOT / "cmr2bin/CMR2.exe"))
    def double(address):
        return struct.unpack("<d", pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 8))[0]
    scale = double(0x5112e0)  # 65536.0, used by the original initializer
    assert scale == 65536.0, scale
    sin_values = [int(round(math.sin(i * double(0x511d08)) * scale)) for i in range(4096)]
    sqrt_values = [int(round(math.sqrt((8 + 16 * i) * double(0x511310)) * scale)) & 0xffff
                   for i in range(4096)]
    arrays = "".join("static unsigned char code_%s[] = {%s};\n" %
                     (name, ",".join(str(x) for x in code))
                     for name, code in (("original", original), ("rebuilt", rebuilt)))
    arrays += "static int sin_values[] = {%s};\n" % ",".join(map(str, sin_values))
    arrays += "static unsigned short sqrt_values[] = {%s};\n" % ",".join(map(str, sqrt_values))
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=os.environ.get("WINEPREFIX", str(TOOLS / "wineprefix")))
    def win(path):
        return subprocess.check_output(["winepath", "-w", str(path)], env=env, text=True).strip()
    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin") + ";" + win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-rotation-") as tmp:
        (Path(tmp) / "rotation.c").write_text(arrays + DRIVER)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "rotation.c", "/Ferotation.exe"], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "rotation.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
