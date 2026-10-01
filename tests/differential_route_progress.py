#!/usr/bin/env python3
"""Check signed route progress, clamps and provider side effects with native code.

Usage: report.json entities.json [rebuilt.exe]. The previous unsigned comparison
must fail for negative progress; the provider also changes globals after capture.
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
typedef int (__stdcall *ProgressFn)(BYTE *);
static BYTE *arena,initial[65536],expected[65536],original[65536];
static int caseId,image,trace,originalTrace,slot,value,initialNodes,multi;
static int changedNodes,changedDivisor,count,expectedCalls;
static unsigned int seed;
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *reason,int where) {
    printf("%s case=%d image=%d position=%d\n",reason,caseId,image,where);exit(3);
}
static DWORD __stdcall provider(void) {
    trace++;
    *(int *)(arena+256)=changedNodes;
    *(int *)(arena+264)=changedDivisor*65536;
    *(int *)(arena+4096+slot*24+8)=7777+caseId;
    arena[0x8000+0xb1a]=(BYTE)((slot+1)%8);
    return 0xab120000u|(DWORD)count;
}
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,disp;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){
        if(calls[i][1])exit(2);
        disp=(BYTE *)provider-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);
    }
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    static int edges[]={-1,0,1,9999,20000,-20000,32767,-32767};
    int i;seed=caseId+769;
    for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    slot=caseId%8;value=caseId%3?edges[caseId%8]:(int)(next()%65535)-32767;
    initialNodes=caseId%7?caseId%127+1:0;multi=caseId%2;
    count=caseId%256;changedNodes=caseId%401-200;
    changedDivisor=caseId%601-300;if(!changedDivisor)changedDivisor=1;
    if(caseId==1){value=-1;changedNodes=123;changedDivisor=300;}
    *(int *)(arena+256)=initialNodes;*(int *)(arena+260)=multi;
    *(int *)(arena+264)=(caseId%301+1)*65536;
    *(int *)(arena+4096+slot*24+8)=value;arena[0x8000+0xb1a]=(BYTE)slot;
    memcpy(initial,arena,65536);memcpy(expected,arena,65536);trace=0;
}
static int model(void) {
    int nodes=initialNodes,denominator=caseId%301+1,result,threshold;
    expectedCalls=0;if(!nodes)return 0;
    if(multi){
        expectedCalls=1;nodes=changedNodes;denominator=changedDivisor;
        *(int *)(expected+256)=nodes;*(int *)(expected+264)=denominator*65536;
        *(int *)(expected+4096+slot*24+8)=7777+caseId;
        expected[0x8000+0xb1a]=(BYTE)((slot+1)%8);
        threshold=count*nodes;
    }else threshold=nodes;
    if(value>=threshold)return 65536;
    result=(value*65536)/denominator;
    if(result<0)return 0;if(result>65536)return 65536;return result;
}
int main(void) {
    ProgressFn fns[2];int i,result,want,differences=0,originalResult;
    arena=(BYTE *)VirtualAlloc((void *)0x2f000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2f000000)exit(2);
'''
TAIL = r'''
    for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();want=model();result=fns[image](arena+0x8000);
        if(result!=want)fail("signed route progress result",result);
        if(trace!=expectedCalls)fail("provider calls",trace);
        for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail("route progress memory",i);
        if(!image){memcpy(original,arena,65536);originalResult=result;originalTrace=trace;}
        else if(result!=originalResult||trace!=originalTrace||memcmp(original,arena,65536))differences++;
    }
    printf("6000 native route progress cases: %d differences; signed thresholds, clamps, provider capture and full arena checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "build/CMR2.exe"
    address = next(int(e["recomp"], 16) for e in report["data"]
                   if int(e["address"], 16) == 0x421470)
    source, loads = "#include <windows.h>\n", ""
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / "cmr2bin/CMR2.exe"))

        def entity(a):
            return entity_address(entities, a) if image else a

        regions = [(0x538a84, 4, 256), (0x538a94, 4, 260),
                   (0x538c94, 4, 264), (0x538aa8, 192, 4096)]
        globals_ = [(entity(a), size, 0x2f000000+offset) for a, size, offset in regions]
        code, calls = extract(pe, address if image else 0x421470,
                              globals_, {entity(0x406990): 0}, 4)
        source += "static BYTE code%d[]={%s};\n" % (image, ",".join(map(str, code)))
        source += "static int calls%d[][2]={%s};\n" % (
            image, ",".join("{%d,%d}" % c for c in calls))
        loads += "    fns[%d]=(ProgressFn)load(code%d,sizeof(code%d),calls%d,%d);\n" % (
            image, image, image, image, len(calls))
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=os.environ.get("WINEPREFIX", str(TOOLS / "wineprefix")))

    def win(path):
        return "Z:"+str(path).replace("/", "\\")

    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin")+";"+win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-route-progress-") as tmp:
        (Path(tmp) / "progress.c").write_text(source+DRIVER+loads+TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "progress.c", "/Feprogress.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "progress.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
