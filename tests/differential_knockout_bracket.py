#!/usr/bin/env python3
"""Compare knockout seeding, random draws, AI calls, bitfields and memory guards."""
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

import capstone
import pefile
from matching_entities import entity_address

ROOT = Path(__file__).resolve().parents[1]
TOOLS = Path(os.environ.get('CMR2_TOOLS', ROOT.parent / 'tools'))
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True


def extract(pe, address, table_address, providers, cleanup=0):
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 8192)
    instructions = {i.address: i for i in MD.disasm(data, address)}
    pending, visited, calls, tables = [address], set(), [], {}
    while pending:
        pos = pending.pop()
        if pos in visited:
            continue
        ins = instructions[pos]
        visited.add(pos)
        if ins.group(capstone.CS_GRP_CALL):
            assert ins.operands[0].type == capstone.x86.X86_OP_IMM
            calls.append((pos - address + ins.imm_offset, providers[ins.operands[0].imm]))
        if ins.group(capstone.CS_GRP_RET):
            assert (int(ins.op_str, 0) if ins.op_str else 0) == cleanup
            continue
        if ins.group(capstone.CS_GRP_JUMP):
            op = ins.operands[0]
            if op.type == capstone.x86.X86_OP_MEM:
                assert ins.mnemonic == 'jmp' and op.mem.base == 0 and op.mem.scale == 4
                targets = struct.unpack('<4I', pe.get_data(op.mem.disp - pe.OPTIONAL_HEADER.ImageBase, 16))
                assert all(address <= target < address + 8192 for target in targets)
                tables[op.mem.disp] = targets
                pending.extend(targets)
                continue
            assert op.type == capstone.x86.X86_OP_IMM and address <= op.imm < address + 8192
            pending.append(op.imm)
            if ins.mnemonic == 'jmp':
                continue
        pending.append(pos + ins.size)
    end = max([pos + instructions[pos].size for pos in visited] + [p + 16 for p in tables])
    code = bytearray(data[:end - address])
    fixups = []
    for pos in sorted(visited):
        ins = instructions[pos]
        if ins.group(capstone.CS_GRP_CALL):
            continue
        for op in ins.operands:
            if op.type == capstone.x86.X86_OP_MEM:
                value, offset, size = op.mem.disp, ins.disp_offset, ins.disp_size
            elif op.type == capstone.x86.X86_OP_IMM:
                value, offset, size = op.imm, ins.imm_offset, ins.imm_size
            else:
                continue
            if value in tables:
                assert size == 4
                fixups.append((pos - address + offset, value - address))
            elif table_address <= value <= table_address + 280:
                assert size == 4
                struct.pack_into('<I', code, pos - address + offset, 0x2d001000 + value - table_address)
            elif op.type == capstone.x86.X86_OP_MEM:
                assert not pe.OPTIONAL_HEADER.ImageBase <= value < pe.OPTIONAL_HEADER.ImageBase + pe.OPTIONAL_HEADER.SizeOfImage, hex(value)
    for table, targets in tables.items():
        for i, target in enumerate(targets):
            fixups.append((table - address + 4 * i, target - address))
    return code, sorted(calls), sorted(set(fixups))


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *SeedFn)(void);
typedef unsigned int (__stdcall *RoundFn)(unsigned int *);
static BYTE *arena, initial[8192], original[8192];
static unsigned int rng, initialRng;
static int caseId, humans, withAI, trace[4096][3], oldTrace[4096][3], ntrace, oldCount;
static void record(int kind,int a,int b) {
    if(ntrace>=4096){printf("trace overflow\n");exit(3);}
    trace[ntrace][0]=kind;trace[ntrace][1]=a;trace[ntrace++][2]=b;
}
static BYTE __stdcall players(void) {record(0,humans,0);return (BYTE)humans;}
static BYTE __stdcall includeAI(void) {record(1,withAI,0);return (BYTE)withAI;}
static int __cdecl randomDraw(void) {
    int value;rng=rng*1103515245u+12345u;value=(rng>>16)&32767;
    record(2,value,0);return value;
}
static void __stdcall assignAI(BYTE driver,BYTE opponent) {record(3,driver,opponent);}
static unsigned int *__stdcall getState(void) {record(4,0,0);return (unsigned int *)(arena+4096);}
static void *mocks[]={players,includeAI,randomDraw,assignAI,getState};
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int nc,const int (*fixups)[2],int nf) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);int i,value;
    if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<nc;i++){value=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);memcpy(p+calls[i][0],&value,4);}
    for(i=0;i<nf;i++){value=(int)p+fixups[i][1];memcpy(p+fixups[i][0],&value,4);}
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void check(void) {
    int i,slot;unsigned int before,after,word;
    before=*(unsigned int *)(initial+4096);after=*(unsigned int *)(arena+4096);
    if((before&0xffff01c7u)!=(after&0xffff01c7u))exit(4);
    if((after&0xfc00u)||(after&0x200u)==0)exit(5);
    if(((after>>3)&7u)!=(unsigned int)(4-caseId%4))exit(9);
    for(slot=0;slot<15;slot++) {
        before=*(unsigned int *)(initial+4100+slot*12);
        word=*(unsigned int *)(arena+4100+slot*12);
        if((before&0xffffe000u)!=(word&0xffffe000u)||(word&0x1c00u))exit(6);
        if(*(int *)(arena+4104+slot*12)||*(int *)(arena+4108+slot*12))exit(7);
    }
    for(i=0;i<8192;i++)if((i<4096||i>=4096+184)&&arena[i]!=initial[i])exit(8);
}
int main(void) {
    SeedFn fn[2];RoundFn roundFn[2];int i,image,mode,capacity,slot,at;
    unsigned int expected,result;unsigned int *high;
    arena=(BYTE *)VirtualAlloc((void *)0x2d000000,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2d000000)return 2;
LOADS
    for(caseId=0;caseId<6000;caseId++) {
        mode=1+caseId%4;capacity=1<<mode;humans=(caseId/8)%(capacity+1);withAI=(caseId/4)%2;
        rng=caseId+991;
        for(i=0;i<8192;i++){rng=rng*1664525u+1013904223u;initial[i]=(BYTE)(rng>>24);}
        *(unsigned int *)(initial+4096)=(*(unsigned int *)(initial+4096)&~7u)|mode;
        initialRng=rng;
        for(image=0;image<2;image++) {
            memcpy(arena,initial,8192);rng=initialRng;ntrace=0;fn[image]();check();
            if(!image){memcpy(original,arena,8192);memcpy(oldTrace,trace,sizeof(trace));oldCount=ntrace;}
            else if(memcmp(original,arena,8192)||oldCount!=ntrace||memcmp(oldTrace,trace,ntrace*sizeof(trace[0]))) {
                printf("knockout mismatch case=%d mode=%d players=%d AI=%d calls=%d/%d\n",caseId,mode,humans,withAI,oldCount,ntrace);return 1;
            }
        }
    }
    printf("6000 knockout cases: 0 differences; all four modes, AI choices, random draws, call order, bitfields and guards checked\n");
    for(caseId=0;caseId<6000;caseId++) {
        mode=caseId%8;
        capacity=mode>=1&&mode<=4?16>>mode:1;slot=(caseId/8)%capacity;
        for(i=0;i<8192;i++){rng=rng*1664525u+1013904223u;initial[i]=(BYTE)(rng>>24);}
        *(unsigned int *)(initial+4096)=(*(unsigned int *)(initial+4096)&~0xf038u)|(mode<<3)|(slot<<12);
        high=mode>=1&&mode<=4&&(caseId/64)%2==0?NULL:(unsigned int *)(arena+4096+220);
        at=mode==1?88+12*slot:mode==2?40+12*slot:mode==3?16+12*slot:mode==4?4:220;
        expected=(*(unsigned int *)(initial+4096+at)>>(high?5:0))&31u;
        for(image=0;image<2;image++) {
            memcpy(arena,initial,8192);ntrace=0;result=roundFn[image](high);
            if(result!=expected||memcmp(arena,initial,8192)||ntrace!=1||trace[0][0]!=4) {
                printf("round selector mismatch case=%d mode=%d image=%d result=%u/%u\n",caseId,mode,image,result,expected);return 1;
            }
        }
    }
    printf("6000 current-round selector cases: 0 differences; both fields, four rounds, fallback and all guards checked\n");return 0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    fn = next(f for f in report['data'] if int(f['address'], 16) == 0x4069c0)
    assert int(fn['recomp'], 16) == entity_address(entities, 0x4069c0)
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / 'build/CMR2.exe'
    source, loads = '', ''
    for image in range(2):
        pe = pefile.PE(str(rebuilt if image else ROOT / 'cmr2bin/CMR2.exe'))
        entity = lambda a: entity_address(entities, a) if image else a
        code, calls, fixups = extract(pe, entity(0x4069c0), entity(0x52f2b4),
                                    {entity(a): i for i, a in enumerate((0x405d70, 0x405dd0, 0x4c6676, 0x4084c0))})
        source += 'static unsigned char code%d[]={%s};\n' % (image, ','.join(map(str, code)))
        for name, rows in [('calls', calls), ('fixups', fixups)]:
            assert rows
            source += 'static const int %s%d[][2]={%s};\n' % (name, image, ','.join('{%d,%d}' % row for row in rows))
        loads += '    fn[%d]=(SeedFn)load(code%d,sizeof(code%d),calls%d,%d,fixups%d,%d);\n' % (image, image, image, image, len(calls), image, len(fixups))
        code, calls, fixups = extract(pe, entity(0x4735a0), entity(0x52f2b4),
                                    {entity(0x407820): 4}, cleanup=4)
        source += 'static unsigned char round%d[]={%s};\n' % (image, ','.join(map(str, code)))
        for name, rows in [('roundCalls', calls), ('roundFixups', fixups)]:
            assert rows
            source += 'static const int %s%d[][2]={%s};\n' % (name, image, ','.join('{%d,%d}' % row for row in rows))
        loads += '    roundFn[%d]=(RoundFn)load(round%d,sizeof(round%d),roundCalls%d,%d,roundFixups%d,%d);\n' % (image, image, image, image, len(calls), image, len(fixups))
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=os.environ.get('WINEPREFIX', str(TOOLS / 'wineprefix')))
    win = lambda p: 'Z:' + str(p).replace('/', '\\')
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-knockout-') as tmp:
        (Path(tmp) / 'knockout.c').write_text(source + DRIVER.replace('LOADS', loads))
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo', '/O2', '/MD',
                        'knockout.c', '/Feknockout.exe'], cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'knockout.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
