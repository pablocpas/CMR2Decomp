#!/usr/bin/env python3
"""Run native network result assembly and its comparator against the original.

Usage: differential_network_results.py report.json entities.json
Checks the complete player/result region, call order and guards. Game state and
local timing callees are mocked; qsort uses each image's native comparator.
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
CALLEES = [0x407e60, 0x4591e0, 0x4a1a00, 0x41b390, 0x4483c0, 0x4c667c]


def extract(pe, address, region, callees, destination, expected_ret):
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
            calls.append((pos - address + ins.imm_offset,
                          callees.index(ins.operands[0].imm)))
        if ins.group(capstone.CS_GRP_RET):
            assert ins.op_str == expected_ret
            continue
        if ins.group(capstone.CS_GRP_JUMP):
            assert ins.operands[0].type == capstone.x86.X86_OP_IMM
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
        for operand in ins.operands:
            if operand.type == capstone.x86.X86_OP_MEM:
                value, offset, size = operand.mem.disp, ins.disp_offset, ins.disp_size
            elif operand.type == capstone.x86.X86_OP_IMM:
                value, offset, size = operand.imm, ins.imm_offset, ins.imm_size
            else:
                continue
            if region <= value < region + 0x684:
                assert size == 4
                struct.pack_into('<I', code, pos - address + offset,
                                 destination + value - region)
            elif operand.type == capstone.x86.X86_OP_MEM:
                assert not 0x400000 <= value < 0x900000, 'Unmapped global'
    return code, sorted(calls)


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *BuildFn)(int,int,int);
typedef int (__cdecl *CompareFn)(const void *,const void *);
static BYTE *regions[2],input[0x704],expected[0x704];
static CompareFn comparators[2];
static int image,flag,logCount,logExpectedCount,logEntries[64][3],logExpected[64][3];
static unsigned int seed,playerId,timeValue,kind,session[2];
static unsigned int next(void){seed=seed*1664525u+1013904223u;return seed;}
static void record(int tag,int a,int b){
    if(logCount>=64)exit(3);
    logEntries[logCount][0]=tag;logEntries[logCount][1]=a;logEntries[logCount++][2]=b;
}
static BYTE __stdcall getFlag(void){record(0,flag,0);return (BYTE)flag;}
static void __stdcall adjust(int id,int *a,int *b){
    int offsetA=(BYTE *)a-regions[image],offsetB=(BYTE *)b-regions[image];
    if(offsetA<0x440+64 || offsetA>=0x520+64 || offsetB!=offsetA-8)exit(3);
    record(1,id,offsetA);*(unsigned int *)a^=(unsigned int)id*17u;
    *(unsigned int *)b+=(unsigned int)id*31u;
}
static int __stdcall getId(void){record(2,playerId,0);return playerId;}
static void *__stdcall getSession(void){record(3,0,0);return session;}
static int __stdcall getTime(int index){if(index)exit(3);record(4,index,timeValue);return timeValue;}
static void __cdecl sortResults(void *base,size_t count,size_t size,CompareFn compare){
    unsigned int expectedAddress=image ? COMP_rebuilt : COMP_original;
    if(base!=regions[image]+64+0x440 || count!=8 || size!=28 || (unsigned int)compare!=expectedAddress)exit(3);
    record(5,count,size);qsort(base,count,size,comparators[image]);
}
static void *mocks[]={getFlag,adjust,getId,getSession,getTime,sortResults};
static void *load(const BYTE *bytes,int size,const int (*fixups)[2],int count){
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,offset;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<count;i++){
        offset=(BYTE *)mocks[fixups[i][1]]-(p+fixups[i][0]+4);
        memcpy(p+fixups[i][0],&offset,4);
    }
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
int main(void){
    BuildFn build[2];int test,i,f,differences=0;unsigned int *words;
    regions[0]=(BYTE *)VirtualAlloc((void *)BASE_original,0x704,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    regions[1]=(BYTE *)VirtualAlloc((void *)BASE_rebuilt,0x704,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(regions[0]!=(BYTE *)BASE_original || regions[1]!=(BYTE *)BASE_rebuilt)return 2;
    comparators[0]=(CompareFn)load(comp_original,sizeof(comp_original),0,0);
    comparators[1]=(CompareFn)load(comp_rebuilt,sizeof(comp_rebuilt),0,0);
    build[0]=(BuildFn)load(code_original,sizeof(code_original),calls_original,8);
    build[1]=(BuildFn)load(code_rebuilt,sizeof(code_rebuilt),calls_rebuilt,8);
    session[1]=(unsigned int)&kind;
    for(test=0;test<6000;test++){
        seed=test+123;memset(input,0xa5,sizeof(input));words=(unsigned int *)(input+64);
        for(i=0;i<0x684/4;i++)words[i]=next();
        kind=next();playerId=next();timeValue=next();flag=test%4;
        for(f=0;f<2;f++){
            image=f;logCount=0;memcpy(regions[f],input,sizeof(input));
            build[f](test*331u,test*173u,~test);
            if(memcmp(regions[f],input,64) || memcmp(regions[f]+64+0x684,input+64+0x684,64))return 3;
            if(f==0){
                memcpy(expected,regions[f],sizeof(expected));logExpectedCount=logCount;
                memcpy(logExpected,logEntries,sizeof(logExpected));
            }else if(memcmp(expected,regions[f],sizeof(expected)) || logCount!=logExpectedCount ||
                     memcmp(logExpected,logEntries,logCount*sizeof(logEntries[0])))differences++;
        }
    }
    printf("6000 network result assemblies: %d differences; native sorting, complete region, call order and guards checked\n",differences);
    return differences!=0;
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
        region = entities['0x531778'][0] if rebuilt else 0x531778
        base = 0x25000000 + n * 0x1000000
        pe = pefile.PE(str(ROOT / ('build/CMR2.exe' if rebuilt else 'cmr2bin/CMR2.exe')))
        callees = [entities[hex(a)][0] for a in CALLEES] if rebuilt else CALLEES
        source += '#define BASE_%s %du\n#define COMP_%s %du\n' % (name, base, name, addresses[0x40a490] if rebuilt else 0x40a490)
        for f, addr in enumerate([0x40a580, 0x40a490]):
            code, calls = extract(pe, addresses[addr] if rebuilt else addr,
                                  region, callees, base + 64, '0xc' if f == 0 else '')
            assert len(calls) == (8 if f == 0 else 0)
            label = 'code' if f == 0 else 'comp'
            source += 'static BYTE %s_%s[]={%s};\n' % (label, name, ','.join(map(str, code)))
            if f == 0:
                source += 'static int calls_%s[][2]={%s};\n' % (name, ','.join('{%d,%d}' % c for c in calls))
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=os.environ.get('WINEPREFIX', str(TOOLS / 'wineprefix')))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-network-results-') as tmp:
        (Path(tmp) / 'results.c').write_text(source + DRIVER)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo',
                        '/O2', '/MD', 'results.c', '/Feresults.exe'], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'results.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
