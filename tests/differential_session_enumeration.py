#!/usr/bin/env python3
"""Execute both DirectPlay session enumerators from the original and rebuilt PE.

Checks all HRESULT results, descriptor/GUID bytes, COM arguments and call order,
busy/null provider paths and complete arena guards against an independent model.
Usage: differential_session_enumeration.py report.json entities.json
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
typedef int (__stdcall *PasswordFn)(int);
typedef int (__stdcall *EnumFn)(void);
struct Object {void **table;};
static struct Object directPlay;
static void *table[32];
static BYTE *arena,initial[4096],expected[4096],original[4096];
static int stage,caseId,image,trace[8],originalTrace[8],traceCount,originalCount;
static DWORD password;
static unsigned int seed;
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static DWORD resultCode(void) {
    static DWORD codes[]={0,0x8877015e,0x88770118,0x88770140,0x887700aa,
        0x8877005a,0x88770082,0x887700cb,0x80004005,0x80070057,1,0x7fffffff,
        0x80000000,0x887700a9,0x887700ab,0x8877015d,0x8877015f,0xffffffff};
    if(caseId%19==18)return caseId*2654435761u;
    return codes[caseId%19];
}
static void fail(const char *why,int where) {
    printf("%s stage=%d case=%d image=%d offset=%d\n",why,stage,caseId,image,where);exit(3);
}
static void record(int kind) {if(traceCount>=8)fail("trace capacity",0);trace[traceCount++]=kind;}
static void __stdcall clearSessions(void) {record(1);arena[768]=(BYTE)caseId;}
static struct Object *__stdcall getDirectPlay(void) {record(2);return caseId%11?&directPlay:NULL;}
static HRESULT __stdcall enumerate(struct Object *self,BYTE *desc,DWORD timeout,void *callback,void *context,DWORD flags) {
    int i;record(3);
    if(self!=&directPlay||desc!=arena+64||timeout||context||callback!=(void *)0x4a12b0||flags!=(stage?0x20:0x51))fail("EnumSessions arguments",0);
    if(*(DWORD *)desc!=0x50||*(BYTE **)(desc+0x30)!=arena+512||*(DWORD *)(desc+0x34)!=(stage?0:password))fail("descriptor fields",0);
    for(i=0;i<4;i++)if(*(DWORD *)(desc+0x18+i*4)!=*(DWORD *)(arena+160+i*4))fail("application GUID",i);
    return (HRESULT)resultCode();
}
static void *mocks[]={clearSessions,getDirectPlay};
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,disp;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){disp=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);}
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i;seed=caseId+3747;
    for(i=0;i<4096;i++)arena[i]=(BYTE)(next()>>24);
    password=next();*(DWORD *)(arena+32)=caseId%7?0:next()|1;
    /* Distinct words make a swapped GUID detectable. */
    for(i=0;i<4;i++)*(DWORD *)(arena+160+i*4)=0x11223300u+caseId+i*0x1020304u;
    memcpy(initial,arena,sizeof(initial));memcpy(expected,arena,sizeof(expected));traceCount=0;
}
static int model(void) {
    int n=0,answer=0;DWORD code;
    if(*(DWORD *)(initial+32)==0){
        expected[768]=(BYTE)caseId;
        memset(expected+64,0,0x50);*(DWORD *)(expected+64)=0x50;
        memcpy(expected+64+0x18,initial+160,16);
        *(BYTE **)(expected+64+0x30)=arena+512;
        *(DWORD *)(expected+64+0x34)=stage?0:password;
        if(traceCount<2||trace[n++]!=1||trace[n++]!=2)fail("provider call order",0);
        if(caseId%11){
            if(traceCount<3||trace[n++]!=3)fail("missing enumeration",0);
            code=resultCode();
            if(!code)answer=1;
            else if(code==0x8877015e)answer=-1;
            else if(code==0x88770118)answer=-2;
        }
    }
    if(traceCount!=n)fail("extra calls on busy/null paths",traceCount);
    return answer;
}
int main(void) {
    void *fns[2][2];int answer,originalAnswer=0,i,differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x2d000000,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2d000000)exit(2);
    directPlay.table=table;table[13]=enumerate;
'''
TAIL = r'''
    for(stage=0;stage<2;stage++)for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();
        if(stage==0)answer=((PasswordFn)fns[stage][image])((int)password);
        else answer=((EnumFn)fns[stage][image])();
        if(answer!=model())fail("enumeration status",answer);
        for(i=0;i<4096;i++)if(arena[i]!=expected[i])fail("independent memory model",i);
        if(!image){memcpy(original,arena,4096);originalAnswer=answer;originalCount=traceCount;memcpy(originalTrace,trace,traceCount*sizeof(int));}
        else if(answer!=originalAnswer||memcmp(original,arena,4096)||traceCount!=originalCount||memcmp(originalTrace,trace,traceCount*sizeof(int)))differences++;
    }
    printf("12000 native session enumeration cases: %d differences; HRESULTs, GUID/descriptor, busy/null paths, COM calls and guards checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    addresses = {int(e["address"], 16): int(e["recomp"], 16)
                 for e in report["data"] if e.get("recomp")}
    regions = [(0x5a1814, 4, 32), (0x5a0068, 0x50, 64),
               (0x511cd8, 16, 160), (0x5a00b8, 0x104, 512),
               (0x4a12b0, 1, 0x4a12b0 - 0x2d000000)]
    providers = [0x4a0c60, 0x4aad40]
    routines = [(0x4a12d0, 4), (0x4a13b0, 0)]
    source, loads = "#include <windows.h>\n", ""
    for image in range(2):
        pe = pefile.PE(str(ROOT / ("build/CMR2.exe" if image else "cmr2bin/CMR2.exe")))

        def entity(a):
            return entity_address(entities, a) if image else a

        globals_ = [(entity(a), size, 0x2d000000 + offset)
                    for a, size, offset in regions]
        callees = {entity(a): i for i, a in enumerate(providers)}
        for stage, (address, cleanup) in enumerate(routines):
            code, calls = extract(pe, addresses[address] if image else address,
                                  globals_, callees, cleanup)
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
    with tempfile.TemporaryDirectory(prefix="cmr2-session-enumeration-") as tmp:
        (Path(tmp) / "sessions.c").write_text(source + DRIVER + loads + TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "sessions.c", "/Fesessions.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "sessions.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
