#!/usr/bin/env python3
"""Check native name entry: deletion, confirmation, length limits and guards.

Usage: report.json entities.json [rebuilt.exe]. Providers are mocked; all
64 KiB and their call arguments are checked against an independent model.
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
typedef void (__stdcall *NameFn)(BYTE *,int);
static BYTE *arena,expected[65536],original[65536];
static int caseId,image,length,row,column,events,wantedEvents;
static int trace[16][3],wanted[16][3];
static unsigned int seed;
static const char *rows[]={"abcdefghij","klmnopqrst","uvwxyz. <_"};
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *why,int position) {
    printf("%s case=%d image=%d position=%d\n",why,caseId,image,position);exit(3);
}
static void event(int id,int a,int b) {
    if(events>=16)fail("trace overflow",events);
    trace[events][0]=id;trace[events][1]=a;trace[events][2]=b;events++;
}
static void expect(int id,int a,int b) {
    wanted[wantedEvents][0]=id;wanted[wantedEvents][1]=a;wanted[wantedEvents][2]=b;wantedEvents++;
}
static int __stdcall recordIndex(void) {event(0,0,0);return caseId%4;}
static char *__stdcall recordName(int index) {event(1,index,0);return (char *)arena+0xa000;}
static int __stdcall action(void) {event(2,0,0);return 0x12340000+caseId;}
static void __stdcall setAction(int nextAction) {event(3,nextAction,0);}
static void __stdcall sound(int id) {event(4,id,0);}
static void __stdcall setName(int index,char *text) {
    event(5,index,text-(char *)arena);strcpy((char *)arena+0xa000,text);
}
static void *mocks[]={recordIndex,recordName,action,setAction,sound,setName};
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,disp;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){disp=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);}
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i;seed=caseId+773;for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    length=caseId%4;row=(caseId/4)%3;column=(caseId/12)%10;
    if(caseId==1){length=1;row=2;column=8;}
    for(i=0;i<length;i++)arena[0xa000+i]='A'+(caseId+i)%26;
    arena[0xa000+length]=0;arena[0x8007]=(BYTE)row;
    for(i=0;i<3;i++){
        *(short *)(arena+0x801c+i*20)=(short)i;
        arena[0x801f+i*20]=(BYTE)column;
        memcpy(arena+0x2000+i*12,rows[i],11);arena[0x2000+i*12+11]=0;
    }
    memcpy(expected,arena,65536);events=0;wantedEvents=0;
}
static void model(void) {
    char character=rows[row][column];int resultLength=length;
    expect(0,0,0);expect(1,caseId%4,0);
    memcpy(expected+0x4000,expected+0xa000,length+1);
    if(row==2&&character=='<'){
        if(length){resultLength--;expected[0x4000+resultLength]=0;expect(4,2,0);}
    }else if(row==2&&character=='_'){
        if(length){expect(2,0,0);expect(3,0x12340000+caseId,0);}return;
    }else if(length<3){
        expected[0x4000+length]=(BYTE)character;resultLength++;
        expected[0x4000+resultLength]=0;
        if(resultLength==3){expected[0x8007]=2;expected[0x8047]=9;}
        expect(4,1,0);
    }else expect(4,3,0);
    expect(0,0,0);expect(5,caseId%4,0x4000);
    memcpy(expected+0xa000,expected+0x4000,resultLength+1);
}
int main(void) {
    NameFn fns[2];int i,differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x31000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x31000000)exit(2);
'''
TAIL = r'''
    for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();model();fns[image](arena+0x8000,0);
        if(events!=wantedEvents||memcmp(trace,wanted,events*sizeof(trace[0])))fail("name entry trace",events);
        for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail("name entry memory",i);
        if(!image)memcpy(original,arena,65536);else if(memcmp(original,arena,65536))differences++;
    }
    printf("6000 native name entry cases: %d differences; lengths 0..3, deletion/confirmation, all columns, cursor, call traces and full arena checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "build/CMR2.exe"
    address = next(int(e["recomp"], 16) for e in report["data"]
                   if int(e["address"], 16) == 0x4f1040)
    source, loads = "#include <windows.h>\n", ""
    providers = [0x4f2be0, 0x408400, 0x4f2c00, 0x4a0ad0, 0x4a0bc0, 0x4eae90]
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / "cmr2bin/CMR2.exe"))
        def entity(a):
            return entity_address(entities, a) if image else a
        # Relocate base-minus-two operands as well; these are addressing
        # biases, and the memory model still protects the preceding bytes.
        regions = [(entity(0x663b60)-4, 264, 0x31003ffc)]
        regions += [(entity(0x525374+i*12), 12, 0x31002000+i*12) for i in range(3)]
        code, calls = extract(pe, address if image else 0x4f1040,
                              regions, {entity(a): i for i, a in enumerate(providers)}, 8)
        source += "static BYTE code%d[]={%s};\n" % (image, ",".join(map(str, code)))
        source += "static int calls%d[][2]={%s};\n" % (image, ",".join("{%d,%d}" % c for c in calls))
        loads += "    fns[%d]=(NameFn)load(code%d,sizeof(code%d),calls%d,%d);\n" % (image, image, image, image, len(calls))
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=os.environ.get("WINEPREFIX", str(TOOLS / "wineprefix")))
    def win(path):
        return "Z:"+str(path).replace("/", "\\")
    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin")+";"+win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-name-entry-") as tmp:
        (Path(tmp) / "name.c").write_text(source+DRIVER+loads+TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "name.c", "/Fename.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "name.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
