#!/usr/bin/env python3
"""Check camera clip updates with providers that replace the graphics object.

Usage: report.json entities.json [rebuilt.exe]. Both native implementations
must obey an independent call/memory model; the old cached pointer must fail.
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
typedef void (__stdcall *ClipFn)(BYTE,unsigned int,int);
static BYTE *arena,initial[65536],expected[65536],original[65536];
static int caseId,image,events,wantedEvents,trace[32][3],wanted[32][3];
static int counts[2],stateCount,nearValue,farValue,distanceValue,player,direction;
static unsigned int inputNode,seed;
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *why,int position) {
    printf("%s case=%d image=%d position=%d\n",why,caseId,image,position);exit(3);
}
static int graphicsOffset(int ordinal) {return 0x1000+((caseId+ordinal*3)%4)*0x800;}
static int changedFar(int ordinal) {return (caseId%7-3)*65536+ordinal*91;}
static void effect(BYTE *memory,int ordinal) {
    int offset=graphicsOffset(ordinal);
    *(BYTE **)(memory+256)=arena+offset;
    *(int *)(memory+offset+0x3c8)=changedFar(ordinal);
}
static void event(int kind,int a,int b) {
    if(events>=32)fail("event overflow",events);
    trace[events][0]=kind;trace[events][1]=a;trace[events][2]=b;
    events++;effect(arena,events);
}
static void expect(int kind,int a,int b) {
    wanted[wantedEvents][0]=kind;wanted[wantedEvents][1]=a;wanted[wantedEvents][2]=b;
    wantedEvents++;effect(expected,wantedEvents);
}
static BYTE __stdcall state(void) {event(0,0,0);return (BYTE)stateCount;}
static int __stdcall count(void) {int n=events?trace[events-1][0]:-1;event(1,0,0);return counts[n==1];}
static int __stdcall special(void) {event(2,0,0);return caseId%2;}
static BYTE __stdcall mode(void) {event(3,0,0);return (BYTE)(caseId%5);}
static BYTE *__stdcall node(unsigned int index) {event(4,index,0);return arena+0x8000;}
static int __stdcall distance(void) {event(5,0,0);return distanceValue;}
static void __stdcall setDistance(BYTE index,int value) {
    *(int *)(arena+0x9000+index*4)=value;event(6,index,value);
}
static void *mocks[]={state,count,special,mode,node,distance,setDistance};
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,disp;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){disp=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);}
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    static int edges[]={0,-1,1,65536,-65536,2147483647,(-2147483647-1)};
    int i;seed=caseId+901;for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    player=caseId%4;stateCount=(caseId/4)%5;direction=caseId%3-1;
    inputNode=(caseId%17==0)?0xffffffffu:caseId%13;
    counts[0]=caseId%15;counts[1]=(caseId+3)%15;
    nearValue=edges[(caseId/3)%7];farValue=edges[(caseId/21)%7];
    distanceValue=edges[(caseId/147)%7];
    *(BYTE **)(arena+256)=arena+0x1000;
    for(i=0;i<4;i++)*(int *)(arena+0x1000+i*0x800+0x3c8)=i*12345;
    *(int *)(arena+0x801c)=nearValue;*(int *)(arena+0x8020)=farValue;
    *(int *)(arena+0x8024)=nearValue;*(int *)(arena+0x8028)=farValue;
    if(caseId==1){player=0;stateCount=2;direction=1;nearValue=1;farValue=65536;distanceValue=65536;
        *(int *)(arena+0x8024)=nearValue;*(int *)(arena+0x8028)=farValue;}
    memcpy(initial,arena,65536);memcpy(expected,arena,65536);
    events=0;wantedEvents=0;
}
static void model(void) {
    unsigned int index=inputNode;int i,limit,value,offset;
    expect(0,0,0);if(player>=stateCount)return;
    if(direction<0){index++;expect(1,0,0);if(index>=(unsigned int)counts[0]){expect(1,0,0);index=counts[1]-1;}}
    expect(0,0,0);if(stateCount>1){expect(2,0,0);if(caseId%2==0)expect(3,0,0);}
    expect(4,index,0);
    for(i=0;i<2;i++){
        value=i?nearValue:farValue;if(value<=0)continue;
        offset=(BYTE *)*(BYTE **)(expected+256)-arena;
        *(int *)(expected+offset+(i?0x3c4:0x3c8))=value;
        expect(5,0,0);
        offset=(BYTE *)*(BYTE **)(expected+256)-arena;
        limit=*(int *)(expected+offset+0x3c8);
        value=distanceValue>limit?limit:distanceValue;
        *(int *)(expected+0x9000+player*4)=value;
        expect(6,player,value);
    }
}
int main(void) {
    ClipFn fns[2];int i,differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x30000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x30000000)exit(2);
'''
TAIL = r'''
    for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();model();fns[image]((BYTE)player,inputNode,direction);
        if(events!=wantedEvents||memcmp(trace,wanted,events*sizeof(trace[0])))fail("camera call trace",events);
        for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail("camera clip memory",i);
        if(!image)memcpy(original,arena,65536);else if(memcmp(original,arena,65536))differences++;
    }
    printf("6000 native camera clip cases: %d differences; live graphics replacements, signed clips, unsigned node clamp, call traces and full arena checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "build/CMR2.exe"
    address = next(int(e["recomp"], 16) for e in report["data"]
                   if int(e["address"], 16) == 0x459250)
    source, loads = "#include <windows.h>\n", ""
    providers = [0x4074f0, 0x421420, 0x41f3a0, 0x405da0, 0x421440, 0x423f30, 0x422f90]
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / "cmr2bin/CMR2.exe"))
        def entity(a):
            return entity_address(entities, a) if image else a
        code, calls = extract(pe, address if image else 0x459250,
                              [(entity(0x520b74), 4, 0x30000100)],
                              {entity(a): i for i, a in enumerate(providers)}, 12)
        source += "static BYTE code%d[]={%s};\n" % (image, ",".join(map(str, code)))
        source += "static int calls%d[][2]={%s};\n" % (image, ",".join("{%d,%d}" % c for c in calls))
        loads += "    fns[%d]=(ClipFn)load(code%d,sizeof(code%d),calls%d,%d);\n" % (image, image, image, image, len(calls))
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=os.environ.get("WINEPREFIX", str(TOOLS / "wineprefix")))
    def win(path):
        return "Z:"+str(path).replace("/", "\\")
    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin")+";"+win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-camera-clip-") as tmp:
        (Path(tmp) / "clip.c").write_text(source+DRIVER+loads+TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "clip.c", "/Feclip.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "clip.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
