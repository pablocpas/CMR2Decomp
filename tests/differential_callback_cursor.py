#!/usr/bin/env python3
"""Check callback transitions, cursor width and surrounding native memory.

Usage: report.json entities.json [rebuilt.exe]
No providers are simulated: the function has no external calls.
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
typedef void (__stdcall *TransitionFn)(void *);
static BYTE *arena,expected[65536],original[65536];
static unsigned int seed;
static int caseId,image;
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(int at) {
    printf("callback transition model case=%d image=%d offset=%d\n",caseId,image,at);exit(3);
}
static void *load(const BYTE *bytes,int size) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    if(!p)exit(2);memcpy(p,bytes,size);FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i;seed=caseId+311;
    for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    /* DWORD cursor at4096, published record at4104, group at8192. */
    arena[8192]=(BYTE)(caseId%256);
    *(BYTE **)(arena+8196)=arena+16384;
    *(BYTE **)(arena+4104)=arena+24576;
    memcpy(expected,arena,sizeof(expected));
}
static void model(void) {
    int i,at,count=expected[8192];DWORD flags;
    expected[4096]=0;
    for(i=0;i<count;i++){
        at=16384+8*i;flags=*(DWORD *)(expected+at);
        if(flags&0x03000000){
            *(DWORD *)(expected+at)=(flags&0xfc00ff00)|((flags>>16)&0xff);
            *(DWORD *)(expected+at+4)=0;
        } else {
            /* The original INC wraps the unsigned word. */
            *(DWORD *)(expected+at+4)+=1;
        }
        *(BYTE **)(expected+4104)=arena+at;
        expected[4096]=(BYTE)(i+1);
    }
}
int main(void) {
    TransitionFn fns[2];int i,differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x2d000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2d000000)exit(2);
'''
TAIL = r'''
    for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();model();fns[image](arena+8192);
        for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail(i);
        if(!image)memcpy(original,arena,65536);
        else if(memcmp(original,arena,65536))differences++;
    }
    printf("6000 native callback cursor cases: %d differences; counts0..255, transitions, preserved upper bytes, published record and all guards checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    address = next(int(e["recomp"], 16) for e in report["data"]
                   if int(e["address"], 16) == 0x49c370)
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "build/CMR2.exe"
    source, loads = "#include <windows.h>\n", ""
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / "cmr2bin/CMR2.exe"))

        def entity(a):
            return entity_address(entities, a) if image else a

        globals_ = [(entity(0x593ba8), 4, 0x2d001000),
                    (entity(0x593ba4), 4, 0x2d001008)]
        code, calls = extract(pe, address if image else 0x49c370, globals_, {}, 4)
        assert not calls
        source += "static BYTE code%d[]={%s};\n" % (image, ",".join(map(str, code)))
        loads += "    fns[%d]=(TransitionFn)load(code%d,sizeof(code%d));\n" % ((image,)*3)
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=os.environ.get("WINEPREFIX", str(TOOLS / "wineprefix")))

    def win(path):
        return "Z:" + str(path).replace("/", "\\")

    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin") + ";" + win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-callback-cursor-") as tmp:
        (Path(tmp) / "cursor.c").write_text(source+DRIVER+loads+TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "cursor.c", "/Fecursor.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "cursor.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
