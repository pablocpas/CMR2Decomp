#!/usr/bin/env python3
"""Check the HUD font threshold with native x87 code and an integer oracle.

Usage: report.json entities.json [rebuilt.exe]. Exercises exact threshold
neighbours where spilling scaleY to float can change the selected font.
"""
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

import capstone
import pefile
from differential_com_outputs import extract, MD
from matching_entities import entity_address

ROOT = Path(__file__).resolve().parents[1]
TOOLS = Path(os.environ.get("CMR2_TOOLS", ROOT.parent / "tools"))
DRIVER = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *FontFn)(int,int,int);
static BYTE *arena,expected[65536],original[65536];
static int caseId,image,width,height,players;
static unsigned int seed;
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *reason,int where) {
    printf("%s case=%d image=%d position=%d width=%d height=%d players=%d\n",
           reason,caseId,image,where,width,height,players);exit(3);
}
static void *load(const BYTE *bytes,int size) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    if(!p)exit(2);memcpy(p,bytes,size);FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i,row;
    seed=caseId+9347;for(i=0;i<65536;i++)arena[i]=(BYTE)(next()>>24);
    players=caseId%128+1;row=players*192+560;
    switch(caseId%6){
    case 0:width=26*row;height=25*players*row-1;break;
    case 1:width=26*row;height=25*players*row;break;
    case 2:width=26*row;height=25*players*row+1;break;
    case 3:width=25*row-1;height=26*players*row;break;
    case 4:width=25*row;height=26*players*row;break;
    default:width=(int)(next()%(60*row+1));height=(int)(next()%(60*players*row+1));
    }
    if(caseId==1){players=100;row=19760;width=25*row;height=25*players*row-1;}
    *(float *)(arena+512)=0.0f;*(float *)(arena+516)=1.0f;
    *(float *)(arena+520)=40.0f;*(float *)(arena+524)=1000.0f;*(float *)(arena+528)=30.0f;
    memcpy(expected,arena,65536);
    /* Positive inputs and players<=128 keep scaleX's float rounding away
       from 25's boundary. The unspilled scaleY threshold is exact here. */
    *(int *)(expected+256)=width<25*row||height<25*players*row?0x29:0x28;
}
int main(void) {
    FontFn fns[2];int i,differences=0;
    arena=(BYTE *)VirtualAlloc((void *)0x2f000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2f000000)exit(2);
'''
TAIL = r'''
    for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();fns[image](width,height,players);
        for(i=0;i<65536;i++)if(arena[i]!=expected[i])fail("font threshold memory",i);
        if(!image)memcpy(original,arena,65536);
        else if(memcmp(original,arena,65536))differences++;
    }
    printf("6000 native network font cases: %d differences; integer threshold oracle, x87 boundary neighbours and full arena checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / "build/CMR2.exe"
    address = next(int(e["recomp"], 16) for e in report["data"]
                   if int(e["address"], 16) == 0x427580)
    source, loads = "#include <windows.h>\n", ""
    values = {struct.pack("<f", value): offset for value, offset in
              [(0.0,512),(1.0,516),(40.0,520),(1000.0,524),(30.0,528)]}
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / "cmr2bin/CMR2.exe"))
        start = address if image else 0x427580
        output = entity_address(entities, 0x5393d4) if image else 0x5393d4
        globals_ = [(output, 4, 0x2f000100)]
        # The previous PE uses anonymous literals. Rebase each actual float
        # operand by its value without assuming new owners existed there.
        for ins in MD.disasm(pe.get_data(start-pe.OPTIONAL_HEADER.ImageBase, 4096), start):
            if ins.group(capstone.CS_GRP_RET):
                break
            if not ins.mnemonic.startswith("f"):
                continue
            for op in ins.operands:
                if op.type == capstone.x86.X86_OP_MEM and not op.mem.base and not op.mem.index:
                    data = pe.get_data(op.mem.disp-pe.OPTIONAL_HEADER.ImageBase, 4)
                    assert op.size == 4 and data in values, (ins.mnemonic, hex(op.mem.disp), data)
                    globals_.append((op.mem.disp, 4, 0x2f000000+values[data]))
        code, calls = extract(pe, start, globals_, {}, 12)
        assert not calls
        source += "static BYTE code%d[]={%s};\n" % (image, ",".join(map(str, code)))
        loads += "    fns[%d]=(FontFn)load(code%d,sizeof(code%d));\n" % (image,image,image)
    env = dict(os.environ, WINEDEBUG="-all", WINEPREFIX=os.environ.get("WINEPREFIX", str(TOOLS / "wineprefix")))

    def win(path):
        return "Z:"+str(path).replace("/", "\\")

    env["INCLUDE"] = win(TOOLS / "msvc600/VC98/Include")
    env["LIB"] = win(TOOLS / "msvc600/VC98/Lib")
    env["WINEPATH"] = win(TOOLS / "msvc600/VC98/Bin")+";"+win(TOOLS / "msvc600/Common/MSDev98/Bin")
    with tempfile.TemporaryDirectory(prefix="cmr2-network-font-") as tmp:
        (Path(tmp) / "font.c").write_text(source+DRIVER+loads+TAIL)
        subprocess.run(["wine", str(TOOLS / "msvc600/VC98/Bin/CL.EXE"), "/nologo",
                        "/O2", "/MD", "font.c", "/Fefont.exe"],
                       cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["wine", "font.exe"], cwd=tmp, env=env, check=True)


if __name__ == "__main__":
    main()
