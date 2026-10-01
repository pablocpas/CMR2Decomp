#!/usr/bin/env python3
"""Execute both replay resets with full memory models and changing providers.

Usage: differential_replay_reset.py report.json entities.json
Checks all 0x290 bytes, the two independent timestamp arrays, surrounding
guards, reverse call order and a buffer pointer changed during iteration.
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
typedef void (__stdcall *ResetFn)(BYTE *);
typedef void (__stdcall *AllFn)(void);
static BYTE *arena,initial[65536],expected[65536],original[65536];
static int stage,caseId,image,trace[16],originalTrace[16],traceCount,originalCount;
static unsigned int seed;
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *why,int where) {
    printf("%s stage=%d case=%d image=%d offset=%d\n",why,stage,caseId,image,where);exit(3);
}
static void __stdcall resetRecord(BYTE *p) {
    int offset=(int)(p-arena);
    if(traceCount>=16)fail("trace capacity",offset);
    trace[traceCount++]=offset;
    p[0]=(BYTE)(caseId+traceCount);
    /* The loop must retain its count and reload the base pointer each time. */
    *(int *)(arena+32)=-7;
    if(caseId%2)*(BYTE **)(arena+36)=arena+12288;
}
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,disp;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){
        if(calls[i][1]!=0)fail("unknown provider",calls[i][1]);
        disp=(BYTE *)resetRecord-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);
    }
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i;seed=caseId+615;
    for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    *(int *)(arena+32)=caseId%13-3;
    *(BYTE **)(arena+36)=arena+4096;
    memcpy(initial,arena,sizeof(initial));memcpy(expected,arena,sizeof(expected));traceCount=0;
}
static void model(void) {
    int i,n=0,count,base=4096,offset;
    if(stage==0){
        expected[4096+0x20a]=expected[4096+0x20b]=0;
        for(i=0;i<20;i++)expected[4096+0x112+i*0xd]=0xff;
        memset(expected+4096+0x24c,0,0x22);
        memset(expected+4096+0x270,0,28);
        memcpy(expected+4096,expected+4096+0x106,0x106);
        memcpy(expected+4096+0x20c,expected+4096+0x24c,0x40);
        if(traceCount)fail("unexpected provider calls",traceCount);
    } else {
        count=*(int *)(initial+32);
        for(i=count-1;i>=0;i--){
            offset=base+i*0x290;
            if(n>=traceCount||trace[n]!=offset)fail("reverse reset order/base reload",offset);
            n++;expected[offset]=(BYTE)(caseId+n);*(int *)(expected+32)=-7;
            if(caseId%2){base=12288;*(BYTE **)(expected+36)=arena+base;}
        }
        if(traceCount!=n)fail("reset call count",traceCount);
        memset(expected+64,0,8);memset(expected+80,0,8);
    }
    for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail("independent memory model",i);
}
int main(void) {
    void *fns[2][2];int differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x2c000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2c000000)exit(2);
'''
TAIL = r'''
    for(stage=0;stage<2;stage++)for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();
        if(stage==0)((ResetFn)fns[stage][image])(arena+4096);
        else ((AllFn)fns[stage][image])();
        model();
        if(!image){memcpy(original,arena,65536);originalCount=traceCount;memcpy(originalTrace,trace,traceCount*sizeof(int));}
        else if(memcmp(original,arena,65536)||traceCount!=originalCount||memcmp(originalTrace,trace,traceCount*sizeof(int)))differences++;
    }
    printf("12000 native replay reset cases: %d differences; record bytes, two timestamp arrays, guards and changing providers checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    addresses = {int(e["address"], 16): int(e["recomp"], 16)
                 for e in report["data"] if e.get("recomp")}
    regions = [(0x588a90, 4, 32), (0x588b98, 4, 36),
               (0x588a80, 8, 64), (0x588a88, 8, 80)]
    routines = [(0x466920, 4), (0x4668d0, 0)]
    source, loads = "#include <windows.h>\n", ""
    for image in range(2):
        pe = pefile.PE(str(ROOT / ("build/CMR2.exe" if image else "cmr2bin/CMR2.exe")))

        def entity(a):
            return entity_address(entities, a) if image else a

        globals_ = [(entity(a), size, 0x2c000000 + offset)
                    for a, size, offset in regions]
        for stage, (address, cleanup) in enumerate(routines):
            code, calls = extract(pe, addresses[address] if image else address,
                                  globals_, {entity(0x466920): 0}, cleanup)
            name = "s%d_i%d" % (stage, image)
            source += "static BYTE code_%s[]={%s};\n" % (name, ",".join(map(str, code)))
            source += "static int calls_%s[][2]={%s};\n" % (
                name, ",".join("{%d,%d}" % c for c in calls) or "{0,0}")
            loads += "    fns[%d][%d]=load(code_%s,sizeof(code_%s),calls_%s,%d);\n" % (
                stage, image, name, name, name, len(calls))
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=os.environ.get("WINEPREFIX", str(TOOLS / "wineprefix")))

    def win(path):
        return "Z:" + str(path).replace("/", "\\")

    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin") + ";" + win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-replay-reset-") as tmp:
        (Path(tmp) / "reset.c").write_text(source + DRIVER + loads + TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "reset.c", "/Fereset.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "reset.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
