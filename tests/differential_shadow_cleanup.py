#!/usr/bin/env python3
"""Check shadow ownership cleanup and counters against native original code.

Usage: report.json entities.json [rebuilt.exe]
FreeGenericFileBuffer is simulated: record each release without destroying
the allocation, so all pointer clears and surrounding bytes can be checked.
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
typedef void (__stdcall *CleanupFn)(void);
static BYTE *arena,expected[65536],original[65536];
static int trace[1024],expectedTrace[1024],traceCount,expectedCount;
static int caseId,image;
static unsigned int seed;
static int bufferOffsets[]={0x38,0x40,0x3c,0x48,0x44,0x4c};
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *why,int at) {
    printf("%s case=%d image=%d offset=%d\n",why,caseId,image,at);exit(3);
}
static void __stdcall release(void *p) {
    int at=(BYTE *)p-arena;
    if(at<8192 || at>=65536 || traceCount>=1024)fail("invalid release",at);
    trace[traceCount++]=at;
}
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,disp;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){
        if(calls[i][1]!=0)exit(2);
        disp=(BYTE *)release-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);
    }
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void pointer(int at,int target) {*(BYTE **)(arena+at)=target?arena+target:NULL;}
static void prepare(void) {
    int i,j,k,caster,parts,mesh,count;
    seed=caseId+137;
    for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    memset(arena+4096,0,120);memset(arena+4224,0,40);
    arena[4272]=(BYTE)(caseId+1);arena[4273]=(BYTE)(caseId+7);
    *(int *)(arena+4276)=caseId*97+1;arena[4280]=(BYTE)(caseId+13);
    for(i=0;i<30;i++){
        caster=8192+i*32;parts=10240+i*0x160;
        count=(caseId+i)%5;
        if(caseId==0 || (next()&3)==0)continue;
        pointer(4096+4*i,caster);
        pointer(caster+4,(next()&3)?parts:0);arena[caster+8]=(BYTE)count;
        for(j=0;j<count;j++)for(k=0;k<6;k++)
            pointer(parts+0x58*j+bufferOffsets[k],(next()&3)?49152+i*192+j*40+k*4:0);
    }
    for(i=0;i<10;i++){
        mesh=24576+i*0x120;
        if(caseId==0 || (next()&3)==0)continue;
        pointer(4224+4*i,mesh);
        pointer(mesh+0x24,(next()&3)?57344+i*32:0);
        pointer(mesh+0xc,(next()&3)?57348+i*32:0);
    }
    memcpy(expected,arena,sizeof(expected));traceCount=expectedCount=0;
}
static int target(int at) {return *(BYTE **)(expected+at)-arena;}
static void owned(int at) {
    if(*(void **)(expected+at)!=NULL){
        if(expectedCount>=1024)exit(2);
        expectedTrace[expectedCount++]=target(at);
        *(void **)(expected+at)=NULL;
    }
}
static void model(void) {
    int i,j,k,caster,parts,mesh,count;
    for(i=0;i<30;i++)if(*(void **)(expected+4096+4*i)!=NULL){
        caster=target(4096+4*i);
        if(*(void **)(expected+caster+4)!=NULL){
            parts=target(caster+4);count=expected[caster+8];
            for(j=0;j<count;j++)for(k=0;k<6;k++)owned(parts+0x58*j+bufferOffsets[k]);
            owned(caster+4);
        }
        owned(4096+4*i);
    }
    for(i=0;i<10;i++)if(*(void **)(expected+4224+4*i)!=NULL){
        mesh=target(4224+4*i);owned(mesh+0x24);owned(mesh+0xc);owned(4224+4*i);
    }
    expected[4272]=expected[4273]=expected[4280]=0;
    *(int *)(expected+4276)=0;
}
int main(void) {
    CleanupFn fns[2];int i,differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x2e000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2e000000)exit(2);
'''
TAIL = r'''
    for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();model();fns[image]();
        if(traceCount!=expectedCount || memcmp(trace,expectedTrace,traceCount*sizeof(int)))
            fail("release order model",traceCount);
        for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail("shadow memory model",i);
        if(!image)memcpy(original,arena,65536);
        else if(memcmp(original,arena,65536))differences++;
    }
    printf("6000 native shadow cleanup cases: %d differences; nested releases, pointer clears, four counters and all guards checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    address = next(int(e["recomp"], 16) for e in report["data"]
                   if int(e["address"], 16) == 0x4b5380)
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "build/CMR2.exe"
    source, loads = "#include <windows.h>\n", ""
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / "cmr2bin/CMR2.exe"))

        def entity(a):
            return entity_address(entities, a) if image else a

        # Include the one-past table cursors used for loop comparisons.
        regions = [(0x6e0124, 121, 4096), (0x6e01c4, 41, 4224),
                   (0x6deab8, 1, 4272), (0x6e0b99, 1, 4273),
                   (0x6e0b94, 4, 4276), (0x6e0b98, 1, 4280)]
        globals_ = [(entity(a), length, 0x2e000000+at) for a, length, at in regions]
        code, calls = extract(pe, address if image else 0x4b5380, globals_,
                              {entity(0x4aade0): 0}, 0)
        source += "static BYTE code%d[]={%s};\n" % (image, ",".join(map(str, code)))
        source += "static int calls%d[][2]={%s};\n" % (image, ",".join("{%d,%d}" % c for c in calls))
        loads += "    fns[%d]=(CleanupFn)load(code%d,sizeof(code%d),calls%d,sizeof(calls%d)/sizeof(calls%d[0]));\n" % ((image,)*6)
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=str(TOOLS / "wineprefix"))

    def win(path):
        return "Z:" + str(path).replace("/", "\\")

    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin") + ";" + win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-shadow-cleanup-") as tmp:
        (Path(tmp) / "cleanup.c").write_text(source+DRIVER+loads+TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "cleanup.c", "/Fecleanup.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "cleanup.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
