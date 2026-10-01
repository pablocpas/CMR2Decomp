#!/usr/bin/env python3
"""Check both player flags and the full race reset against original machine code.

Usage: differential_race_state_reset.py report.json entities.json [rebuilt.exe]
For the previous split globals, preserve their actual separation in the
rebuilt image. Relocating both scalars into adjacent slots would hide the bug.
"""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

import pefile
from differential_com_outputs import extract
from matching_entities import entity_address

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT.parent / "tools"
DRIVER = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *ResetFn)(void);
static BYTE *arena,initial[65536],expected[65536],original[65536];
static unsigned int seed;
static int caseId,image;
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *why,int offset) {
    printf("%s case=%d image=%d offset=%d\n",why,caseId,image,offset);exit(3);
}
static void *load(const BYTE *bytes,int size) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    if(!p)exit(2);memcpy(p,bytes,size);FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i;seed=caseId+283;
    for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    *(int *)(arena+4096+0x1bc)=caseId+1;
    *(int *)(arena+4096+0x1c0)=-caseId-1;
    memcpy(initial,arena,sizeof(initial));memcpy(expected,arena,sizeof(expected));
}
static void model(void) {
    int i,base;
    /* Offsets describe the original region beginning at 0x53708c. */
    for(i=0;i<2;i++){
        *(int *)(expected+4096+4*i)=-1;
        *(int *)(expected+4096+0x10c+4*i)=9999;
        *(int *)(expected+4096+0x1bc+4*i)=0;
    }
    *(int *)(expected+4096+0x114)=0;
    memset(expected+4096+0x118,0,40);
    for(i=0;i<10;i++){
        base=4096+0x144+12*i;
        *(int *)(expected+base)=0;
        *(int *)(expected+base+4)=0;
        *(DWORD *)(expected+base+8)&=0xfffffc00;
    }
    for(i=0;i<5;i++){
        base=4096+0x14+12*i;
        *(int *)(expected+base)=-1;
        *(int *)(expected+base+4)=-1;
        expected[base+8]&=0xfc;
    }
    for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail("race reset memory model",i);
}
int main(void) {
    ResetFn fns[2];int differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x2f000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2f000000)exit(2);
'''
TAIL = r'''
    for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();fns[image]();model();
        if(!image)memcpy(original,arena,65536);
        else if(memcmp(original,arena,65536))differences++;
    }
    printf("6000 native race reset cases: %d differences; both player flags, ten call records, five slots and all guards checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    address = next(int(e["recomp"], 16) for e in report["data"]
                   if int(e["address"], 16) == 0x416670)
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "build/CMR2.exe"
    source, loads = "#include <windows.h>\n", ""
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / "cmr2bin/CMR2.exe"))

        def entity(a):
            return entity_address(entities, a) if image else a

        # Include cursor end addresses used only for comparisons. The call
        # record flag cursor ends eight bytes beyond the record array.
        regions = [(0x53708c, 8), (0x537198, 8), (0x5371a0, 4),
                   (0x5371a4, 40), (0x5370a0, 241), (0x5371d0, 129)]
        globals_ = []
        flags = entity(0x537248)
        if image and "0x53724c" in entities:
            second = entity(0x53724c)
            delta = second - flags
            print("previous scalar separation: %d bytes" % delta, flush=True)
            assert delta > 8 and delta < 32768
            globals_ += [(flags, 4, 0x2f0011bc),
                         (second, 4, 0x2f0011bc+delta)]
        else:
            globals_.append((flags, 8, 0x2f0011bc))
        globals_ += [(entity(a), size, 0x2f001000+a-0x53708c)
                     for a, size in regions]
        code, calls = extract(pe, address if image else 0x416670, globals_, {}, 0)
        assert not calls
        source += "static BYTE code%d[]={%s};\n" % (image, ",".join(map(str, code)))
        loads += "    fns[%d]=(ResetFn)load(code%d,sizeof(code%d));\n" % (image, image, image)
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=str(TOOLS / "wineprefix"))

    def win(path):
        return "Z:" + str(path).replace("/", "\\")

    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin") + ";" + win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-race-state-reset-") as tmp:
        (Path(tmp) / "reset.c").write_text(source+DRIVER+loads+TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "reset.c", "/Fereset.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "reset.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
