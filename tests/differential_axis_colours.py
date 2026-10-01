#!/usr/bin/env python3
"""Check native axis-row drawing and colour reads after provider calls.

Usage: report.json entities.json [rebuilt.exe]
Axis lookup, highlight state and the final draw are simulated providers.
They mutate menu flags/cursor and colours so cached values are observable.
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
typedef void (__stdcall *RowFn)(short,short,void *,int);
static BYTE *arena,expected[65536],original[65536];
static int trace[8][5],expectedTrace[8][5],traceCount,expectedCount;
static int caseId,image,index;
static short x,y;
static unsigned int seed;
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *why,int at) {
    printf("%s case=%d image=%d offset=%d\n",why,caseId,image,at);exit(3);
}
static void record(int kind,int a,int b,DWORD c,int d) {
    int *p;if(traceCount>=8)exit(3);p=trace[traceCount++];
    p[0]=kind;p[1]=a;p[2]=b;p[3]=(int)c;p[4]=d;
}
static void lookupEffect(BYTE *memory) {
    int at=8192+0x1a+20*index;
    /* Change the visible bit after the original's initial decision too. */
    memory[at]=(memory[at]&~3)|(caseId%7?1:0)|(caseId%2?0:2);
    memory[8192+7]=(BYTE)(caseId%3?index:index+1);
    *(DWORD *)(memory+4096)=0x11000000+caseId*97;
}
static void highlightEffect(BYTE *memory) {
    *(DWORD *)(memory+4096)=0x22000000+caseId*113;
    *(DWORD *)(memory+4100)^=0x12345678;
}
static int highlighted(void) {return caseId%3==1?0:(caseId%3==2?-1:1);}
static void *__stdcall lookup(int value) {
    if(value!=index)fail("lookup argument",value);
    record(1,value,0,0,0);lookupEffect(arena);
    return caseId%5?arena+24576:NULL;
}
static int __stdcall highlight(void) {
    record(2,0,0,0,0);highlightEffect(arena);return highlighted();
}
static void __stdcall draw(short drawX,short drawY,DWORD colour,void *pAxis) {
    record(3,drawX,drawY,colour,pAxis?(BYTE *)pAxis-arena:-1);
}
static void *mocks[]={lookup,highlight,draw};
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,disp;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){
        disp=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);
    }
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i,at;seed=caseId+419;
    for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    index=caseId%22;x=(short)(caseId*271);y=(short)(~caseId*109);
    at=8192+0x1a+20*index;
    arena[at]=(arena[at]&~3)|(caseId%4?2:0)|(caseId%2?1:0);
    arena[8192+7]=(BYTE)(caseId%3?index:index+1);
    *(DWORD *)(arena+4096)=0x33000000+caseId;
    *(DWORD *)(arena+4100)=0x44000000+caseId*3;
    *(DWORD *)(arena+4104)=0x55000000+caseId*7;
    memcpy(expected,arena,sizeof(expected));traceCount=expectedCount=0;
    memset(trace,0,sizeof(trace));memset(expectedTrace,0,sizeof(expectedTrace));
}
static void expectedRecord(int kind,int a,int b,DWORD c,int d) {
    int *p=expectedTrace[expectedCount++];
    p[0]=kind;p[1]=a;p[2]=b;p[3]=(int)c;p[4]=d;
}
static void model(void) {
    int at=8192+0x1a+20*index,axis=-1,doDraw=1;DWORD colour;
    if((expected[at]&2)==0){
        colour=*(DWORD *)(expected+4104);
    } else {
        expectedRecord(1,index,0,0,0);lookupEffect(expected);
        axis=caseId%5?24576:-1;
        if((signed char)expected[8192+7]==index){
            expectedRecord(2,0,0,0,0);highlightEffect(expected);
            colour=highlighted()?0xff1414f0:*(DWORD *)(expected+4096);
        } else if(expected[at]&1){
            colour=*(DWORD *)(expected+4100);
        } else {
            doDraw=0;
        }
    }
    if(doDraw)expectedRecord(3,x,y,colour,axis);
}
int main(void) {
    RowFn fns[2];int i,differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x2c000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2c000000)exit(2);
'''
TAIL = r'''
    for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();model();fns[image](x,y,arena+8192,index);
        if(traceCount!=expectedCount || memcmp(trace,expectedTrace,traceCount*5*sizeof(int)))
            fail("axis call model",traceCount);
        for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail("axis memory model",i);
        if(!image)memcpy(original,arena,65536);
        else if(memcmp(original,arena,65536))differences++;
    }
    printf("6000 native axis colour cases: %d differences; visibility, selection, live colours, provider order, draw arguments and all guards checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    address = next(int(e["recomp"], 16) for e in report["data"]
                   if int(e["address"], 16) == 0x4ff060)
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "build/CMR2.exe"
    source, loads = "#include <windows.h>\n", ""
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / "cmr2bin/CMR2.exe"))

        def entity(a):
            return entity_address(entities, a) if image else a

        globals_ = [(entity(a), 4, 0x2c000000+at)
                    for a, at in [(0x524968, 4096), (0x52496c, 4100), (0x524970, 4104)]]
        callees = {entity(a): k for k, a in enumerate([0x4fbe60, 0x4fc610, 0x4ff0f0])}
        code, calls = extract(pe, address if image else 0x4ff060, globals_, callees, 16)
        source += "static BYTE code%d[]={%s};\n" % (image, ",".join(map(str, code)))
        source += "static int calls%d[][2]={%s};\n" % (image, ",".join("{%d,%d}" % c for c in calls))
        loads += "    fns[%d]=(RowFn)load(code%d,sizeof(code%d),calls%d,sizeof(calls%d)/sizeof(calls%d[0]));\n" % ((image,)*6)
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=str(TOOLS / "wineprefix"))

    def win(path):
        return "Z:" + str(path).replace("/", "\\")

    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin") + ";" + win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-axis-colours-") as tmp:
        (Path(tmp) / "axis.c").write_text(source+DRIVER+loads+TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "axis.c", "/Feaxis.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "axis.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
