#!/usr/bin/env python3
"""Compare the eight stage sound resets, full arena and callback ordering.

Usage: differential_stage_sound_reset.py report.json entities.json [rebuilt.exe]
The optional executable supports a negative check against the previous build.
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
TOOLS = Path(os.environ.get("CMR2_TOOLS", ROOT.parent / "tools"))
DRIVER = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *ResetFn)(void);
static BYTE *arena,initial[65536],expected[65536],original[65536];
static unsigned int seed;
static int caseId,image,traceCount,originalCount;
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *why,int offset) {
    printf("%s case=%d image=%d offset=%d\n",why,caseId,image,offset);exit(3);
}
static void __stdcall registerCallback(void *callback,void *context) {
    if((DWORD)callback!=0x418fe0 || context || traceCount++)fail("callback arguments/order",0);
    /* The callback runs after the records have been reset and before flag=1. */
    if(*(int *)(arena+4096+0x2e8)!=25)fail("callback before reset",0x2e8);
    arena[4096+0x864]=7;
    arena[4096+0x7ac]=0x5a;
}
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,disp;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){
        if(calls[i][1])fail("unknown provider",calls[i][1]);
        disp=(BYTE *)registerCallback-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);
    }
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i;seed=caseId+147;
    for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    arena[4096+0x864]=caseId%3?(BYTE)(1+caseId%255):0;
    memcpy(initial,arena,sizeof(initial));memcpy(expected,arena,sizeof(expected));traceCount=0;
}
static void model(void) {
    int car,base,i;
    *(int *)(expected+4096+0xf8)=0;
    memset(expected+4096+0x48,0,0x20);
    memset(expected+4096+0x94,0,0x20);
    memset(expected+4096+0x220,0,0x20);
    memset(expected+4096,0,0x20);
    for(car=0;car<8;car++){
        base=4096+0x240+car*0xb4;
        for(i=0;i<10;i++){
            *(int *)(expected+base+0x1c+4*i)=-1;
            *(int *)(expected+base+0x44+4*i)=-1;
        }
        memset(expected+base,0xff,8);
        *(short *)(expected+base+0x18)=-1;
        *(short *)(expected+base+0x1a)=-1;
        *(int *)(expected+base+0xa0)=0;
        *(int *)(expected+base+0xa4)=0;
        *(int *)(expected+base+0xa8)=25;
    }
    if(!initial[4096+0x864]){
        if(traceCount!=1)fail("missing callback",0);
        expected[4096+0x7ac]=0x5a;
        expected[4096+0x864]=1;
    } else if(traceCount)fail("unexpected callback",0);
    for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail("memory model",i);
}
int main(void) {
    ResetFn fns[2];int differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x2e000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2e000000)exit(2);
'''
TAIL = r'''
    for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();fns[image]();model();
        if(!image){memcpy(original,arena,65536);originalCount=traceCount;}
        else if(memcmp(original,arena,65536)||traceCount!=originalCount)differences++;
    }
    printf("6000 native stage sound reset cases: %d differences; eight records, state words, pattern, guards and callback order checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    address = next(int(e["recomp"], 16) for e in report["data"]
                   if int(e["address"], 16) == 0x418f20)
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "build/CMR2.exe"
    source, loads = "#include <windows.h>\n", ""
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / "cmr2bin/CMR2.exe"))

        def entity(a):
            return entity_address(entities, a) if image else a

        # The 0x537568..0x537dcc tables are separate globals (see StageUI.h);
        # each one is placed at its original offset in the arena.
        tables = [(0x537568, 0x20), (0x537588, 0x20), (0x5375a8, 8), (0x5375b0, 0x20),
                  (0x5375d0, 4), (0x5375d4, 0x1f), (0x5375f4, 8), (0x5375fc, 0x20),
                  (0x53761c, 4), (0x537620, 0x20), (0x537640, 0x20), (0x537660, 4),
                  (0x537664, 4), (0x537668, 0x20), (0x537788, 0x20), (0x5377a8, 0x5a0),
                  (0x537d48, 0x80), (0x537dc8, 4)]
        # The record loop ends on g_carSoundStates + 8: keep that one-past-the-end
        # address on the records (first match wins), wherever the rebuilt
        # linker put the next global.
        regions = [(0x5377a8, 0x5a1, 4096 + 0x240)]
        regions += [(a, size, 4096 + a - 0x537568) for a, size in tables]
        regions += [(0x537dcc, 1, 4096+0x864), (0x537dd0, 0x20, 4096+0x868)]
        globals_ = [(entity(a), size, 0x2e000000+offset) for a, size, offset in regions]
        globals_.append((entity(0x418fe0), 1, 0x418fe0))
        code, calls = extract(pe, address if image else 0x418f20,
                              globals_, {entity(0x49c0a0): 0}, 0)
        source += "static BYTE code%d[]={%s};\n" % (image, ",".join(map(str, code)))
        source += "static int calls%d[][2]={%s};\n" % (
            image, ",".join("{%d,%d}" % c for c in calls) or "{0,0}")
        loads += "    fns[%d]=(ResetFn)load(code%d,sizeof(code%d),calls%d,%d);\n" % (
            image, image, image, image, len(calls))
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=os.environ.get("WINEPREFIX", str(TOOLS / "wineprefix")))

    def win(path):
        return "Z:" + str(path).replace("/", "\\")

    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin") + ";" + win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-stage-sound-reset-") as tmp:
        (Path(tmp) / "reset.c").write_text(source+DRIVER+loads+TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "reset.c", "/Fereset.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "reset.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
