#!/usr/bin/env python3
"""Compare native text rendering with BYTE characters and 16-bit coordinates.

Usage: differential_font_coordinates.py report.json entities.json
Runs the complete original/rebuilt DrawText bodies. Width, selection, random
and glyph drawing are mocked and their arguments/order compared. This checks
the text dispatcher, not rasterization or the still partial width calculator.
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

ROOT = Path(__file__).resolve().parents[1]
TOOLS = Path(os.environ.get("CMR2_TOOLS", ROOT.parent / "tools"))
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True


def extract(pe, address, globals_, callees):
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 4096)
    instructions = {i.address: i for i in MD.disasm(data, address)}
    pending, visited, calls = [address], set(), []
    while pending:
        pos = pending.pop()
        if pos in visited:
            continue
        ins = instructions[pos]
        visited.add(pos)
        if ins.group(capstone.CS_GRP_CALL):
            assert ins.operands[0].type == capstone.x86.X86_OP_IMM
            calls.append((pos - address + ins.imm_offset, callees[ins.operands[0].imm]))
        if ins.group(capstone.CS_GRP_RET):
            assert ins.op_str == '0x18'
            continue
        if ins.group(capstone.CS_GRP_JUMP):
            target = ins.operands[0].imm
            assert address <= target < address + 4096
            pending.append(target)
            if ins.mnemonic == 'jmp':
                continue
        pending.append(pos + ins.size)
    end = max(pos + instructions[pos].size for pos in visited)
    code = bytearray(data[:end - address])
    for pos in sorted(visited):
        ins = instructions[pos]
        if ins.group(capstone.CS_GRP_CALL) or ins.group(capstone.CS_GRP_JUMP):
            continue
        for op in ins.operands:
            if op.type == capstone.x86.X86_OP_MEM:
                value, offset, size = op.mem.disp, ins.disp_offset, ins.disp_size
            elif op.type == capstone.x86.X86_OP_IMM:
                value, offset, size = op.imm, ins.imm_offset, ins.imm_size
            else:
                continue
            replacement = next((target + value - start for start, target in globals_
                                if start <= value < start + 4), None)
            if replacement is not None:
                assert size == 4
                struct.pack_into('<I', code, pos - address + offset, replacement)
            elif op.type == capstone.x86.X86_OP_MEM:
                assert not 0x400000 <= value < 0x900000, hex(value)
    return code, sorted(calls)


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *DrawFn)(BYTE,char *,short,short,int *,unsigned int);
struct Kern { BYTE ch,pad;short offset; };
struct Glyph { BYTE prefix[4];short width,height;BYTE pad,count,middle[2];struct Kern *kern; };
struct Header { int unknown;unsigned int glyphCount;int height,space,spacing,gap; };
struct Font { struct Header *header;struct Glyph *glyphs;void *texture;short *map;void *data;BYTE loaded,owns,pad[2]; };
static struct Header headers[4];
static struct Glyph glyphs[4][256];
static struct Kern kerns[4][256][8];
static short maps[4][256];
static struct Font fonts[4];
static char text[96],before[96];
static int colour[2],trace[512][7],expected[512][7],traceCount,caseId;
static unsigned int seed;
static unsigned int next(void) { seed=seed*1664525u+1013904223u;return seed; }
static int *record(int kind) {
    int *p;if(traceCount>=512)exit(3);p=trace[traceCount++];memset(p,0,7*sizeof(int));p[0]=kind;return p;
}
static int __cdecl randomValue(void) { record(0);return caseId*12345; }
static int __stdcall width(BYTE index,BYTE *p) {
    int *r=record(1);unsigned int hash=0;BYTE *start=p;
    if((char *)p<text || (char *)p>=text+sizeof(text))exit(3);
    r[1]=index;r[2]=(char *)p-text;
    while(*p && *p!='\n' && *p!='^')hash=hash*33u+*p++;
    return (int)((hash+(unsigned int)(start-(BYTE *)text))%2001)-1000;
}
static void __stdcall selectFont(BYTE index,int *pColour) {
    int *r=record(2);r[1]=index;if(pColour!=colour)exit(3);r[2]=pColour[0];r[3]=pColour[1];
}
static void __stdcall drawChar(BYTE ch,short x,short y) {
    int *r=record(3);r[1]=ch;r[2]=x;r[3]=y;
}
static void *mocks[]={randomValue,width,selectFont,drawChar};
static DrawFn load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);int i,disp;
    if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++) {disp=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);}
    FlushInstructionCache(GetCurrentProcess(),p,size);return (DrawFn)p;
}
static void prepare(void) {
    int f,g,k,length;seed=caseId+17;
    memset(text,0xa5,sizeof(text));length=caseId%80;
    for(g=0;g<length;g++) {
        unsigned int v=next()%15;text[g]=v==0?' ':v==1?'\n':v==2?'^':(char)(next()%255+1);
    }
    text[length]=0;memcpy(before,text,sizeof(text));colour[0]=next();colour[1]=next();
    for(f=0;f<4;f++) {
        memset(&fonts[f],0,sizeof(fonts[f]));
        fonts[f].header=&headers[f];fonts[f].glyphs=glyphs[f];fonts[f].map=maps[f];fonts[f].loaded=(caseId+f)%5!=0;
        headers[f].glyphCount=256;headers[f].height=(int)(next()%1000)-500;
        headers[f].space=(int)(next()%200001)-100000;headers[f].spacing=(int)(next()%200001)-100000;
        headers[f].gap=(int)(next()%1000)-500;
        for(g=0;g<256;g++) {
            maps[f][g]=(caseId+g+f)%9==0?-1:g;
            memset(&glyphs[f][g],0,sizeof(struct Glyph));
            glyphs[f][g].width=(short)next();glyphs[f][g].height=(short)next();
            glyphs[f][g].middle[0]=(BYTE)next();glyphs[f][g].count=(BYTE)(next()%9);glyphs[f][g].kern=kerns[f][g];
            for(k=0;k<8;k++) {kerns[f][g][k].ch=(BYTE)(k*32+(g%31));kerns[f][g][k].offset=(short)next();}
        }
    }
}
int main(void) {
    DrawFn draw[2];BYTE *regions[2];int f,count,expectedCount,differences=0,characters=0;short x,y;unsigned int flags;BYTE index;
    if(sizeof(struct Font)!=24 || sizeof(struct Glyph)!=16)exit(2);
    for(f=0;f<2;f++) {
        regions[f]=(BYTE *)VirtualAlloc((void *)(0x25000000+f*0x10000),4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
        if(regions[f]!=(BYTE *)(0x25000000+f*0x10000))exit(2);
    }
    draw[0]=load(code_original,sizeof(code_original),calls_original,sizeof(calls_original)/sizeof(calls_original[0]));
    draw[1]=load(code_rebuilt,sizeof(code_rebuilt),calls_rebuilt,sizeof(calls_rebuilt)/sizeof(calls_rebuilt[0]));
    for(caseId=0;caseId<6000;caseId++) {
        prepare();index=caseId%4;x=(short)next();y=(short)next();flags=caseId%64;
        for(f=0;f<2;f++) {
            memset(regions[f],0xa5,4096);*(struct Font **)(regions[f]+64)=fonts;*(unsigned int *)(regions[f]+68)=3;
            traceCount=0;draw[f](index,text,x,y,colour,flags);
            if(memcmp(text,before,sizeof(text)))exit(3);
            for(count=0;count<4096;count++)if((count<64 || count>=72) && regions[f][count]!=0xa5)exit(3);
            if(*(struct Font **)(regions[f]+64)!=fonts || *(unsigned int *)(regions[f]+68)!=3)exit(3);
            if(f==0) {memcpy(expected,trace,sizeof(trace));expectedCount=traceCount;}
            else if(traceCount!=expectedCount || memcmp(expected,trace,expectedCount*7*sizeof(int)))differences++;
        }
        for(f=0;f<expectedCount;f++)if(expected[f][0]==3)characters++;
    }
    printf("6000 native font cases: %d differences, %d glyph draws; BYTE characters, signed coordinates, breaks, flags, kerning, missing glyphs and guards checked\n",differences,characters);
    return differences!=0 || characters==0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    addresses = {int(e['address'], 16): int(e['recomp'], 16)
                 for e in report['data'] if e.get('recomp')}
    source = '#include <windows.h>\n'
    for n, name in enumerate(['original', 'rebuilt']):
        rebuilt = n == 1
        pe = pefile.PE(str(ROOT / ('build/CMR2.exe' if rebuilt else 'cmr2bin/CMR2.exe')))
        address = addresses[0x40b880] if rebuilt else 0x40b880
        callees = {(v[0] if rebuilt else int(k, 16)): 0 for k, v in entities.items()
                   if v[1].endswith('::rand')}
        for i, a in enumerate([0x40b5b0, 0x40b580, 0x40b7c0], 1):
            callees[entities[hex(a)][0] if rebuilt else a] = i
        globals_ = [(entities[hex(a)][0] if rebuilt else a, 0x25000000 + n * 0x10000 + 64 + i * 4)
                    for i, a in enumerate([0x53223c, 0x532240])]
        code, calls = extract(pe, address, globals_, callees)
        source += 'static BYTE code_%s[]={%s};\n' % (name, ','.join(map(str, code)))
        source += 'static int calls_%s[][2]={%s};\n' % (name, ','.join('{%d,%d}' % c for c in calls))
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=os.environ.get('WINEPREFIX', str(TOOLS / 'wineprefix')))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-font-coordinates-') as tmp:
        (Path(tmp) / 'font.c').write_text(source + DRIVER)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo', '/O2', '/MD',
                        'font.c', '/Fefont.exe'], cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'font.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
