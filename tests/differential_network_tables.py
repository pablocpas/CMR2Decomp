#!/usr/bin/env python3
"""Compare native network table getters and updates, including stage/split zero.

Usage: differential_network_tables.py report.json entities.json
Preserves each image's layout, checks both complete table regions and guards,
and detects mutations that read the wrong neighbouring table. Callees providing
the game mode and record name are mocked; this does not test network transport.
"""

import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

from matching_entities import entity_address

import capstone
import pefile

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT.parent / 'tools'
GLOBALS = [(0x531778, 0x684), (0x531f80, 36)]
FUNCTIONS = [0x40a410, 0x40a420, 0x40a330]
CALLEES = [0x405d80, 0x408400]
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True


def extract(pe, address, globals_, destination, callees, mutation=False):
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
            calls.append([pos - address + ins.imm_offset,
                          callees.index(ins.operands[0].imm)])
        if ins.group(capstone.CS_GRP_RET):
            assert ins.op_str in ['4', '8']
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
    output = bytearray(data[:end - address])
    low = min(a for a, _ in globals_)
    high = max(a + size for a, size in globals_)
    changes = 0
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
            if low <= value < high:
                assert size == 4
                if mutation and value == globals_[1][0]:
                    value += 4
                    changes += 1
                elif mutation and value == globals_[0][0] + 0x51c:
                    value += 32
                    changes += 1
                struct.pack_into('<I', output, pos - address + offset,
                                 destination + value - low)
            elif op.type == capstone.x86.X86_OP_MEM:
                assert not 0x400000 <= value < 0x900000, 'Unmapped global'
    if mutation:
        assert changes == 1
    return output, sorted(calls)


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned int (__stdcall *GetFn)(int);
typedef void (__stdcall *SetFn)(unsigned int,int);
struct Region { BYTE *data,*before,*mask;int size;const unsigned int *offsets; };
static struct Region regions[3];
static int mode,calls,recordCalls;
static char name[160];
static unsigned int seed;
static unsigned int next(void) { seed=seed*1664525u+1013904223u;return seed; }
static int __stdcall getMode(void) { calls++;return mode; }
static char *__stdcall getRecord(int index) {
    if(index!=0 || calls!=1)exit(3);recordCalls++;return name;
}
static void *mocks[]={getMode,getRecord};
static void *load(const BYTE *bytes,int size,const int (*fixups)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,offset;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++) {
        offset=(BYTE *)mocks[fixups[i][1]]-(p+fixups[i][0]+4);
        memcpy(p+fixups[i][0],&offset,4);
    }
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void setup(int n,unsigned int base,int size,const unsigned int *offsets) {
    struct Region *r=&regions[n];r->size=size;r->offsets=offsets;
    r->data=(BYTE *)VirtualAlloc((void *)base,size,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(r->data!=(BYTE *)base)exit(2);
    r->before=(BYTE *)malloc(size);r->mask=(BYTE *)calloc(size,1);
    if(!r->before || !r->mask)exit(2);
    memset(r->mask+offsets[0],1,0x684);memset(r->mask+offsets[1],1,36);
}
static void prepare(int n,int test) {
    struct Region *r=&regions[n];int i;unsigned int *p;
    memset(r->data,0xa5,r->size);seed=test+123;
    p=(unsigned int *)(r->data+r->offsets[0]);
    for(i=0;i<0x684/4;i++)p[i]=next();
    /* Exercise empty/current/high-bit times and every table cell. */
    for(i=0;i<11;i++)p[0x51c/4+i]=(test+i)%7==0 ? 0 : next();
    p=(unsigned int *)(r->data+r->offsets[1]);
    for(i=0;i<9;i++)p[i]=next();
    memcpy(r->before,r->data,r->size);
}
static void guards(int n) {
    struct Region *r=&regions[n];int i;
    for(i=0;i<r->size;i++)if(!r->mask[i] && r->data[i]!=r->before[i])exit(3);
}
int main(void) {
    GetFn split[3],stage[3];SetFn update[2];
    int test,f,i,index,splitIndex,expectedCalls,expectedRecordCalls;
    int differences=0,mutationSplit=0,mutationStage=0;
    unsigned int expected,value,a,b;
    setup(0,BASE_original,SIZE_original,offsets_original);
    setup(1,BASE_rebuilt,SIZE_rebuilt,offsets_rebuilt);
    setup(2,BASE_mutation,SIZE_mutation,offsets_mutation);
    split[0]=(GetFn)load(code_original_0,sizeof(code_original_0),calls_original_0,0);
    split[1]=(GetFn)load(code_rebuilt_0,sizeof(code_rebuilt_0),calls_rebuilt_0,0);
    split[2]=(GetFn)load(code_mutation_0,sizeof(code_mutation_0),calls_mutation_0,0);
    stage[0]=(GetFn)load(code_original_1,sizeof(code_original_1),calls_original_1,1);
    stage[1]=(GetFn)load(code_rebuilt_1,sizeof(code_rebuilt_1),calls_rebuilt_1,1);
    stage[2]=(GetFn)load(code_mutation_1,sizeof(code_mutation_1),calls_mutation_1,1);
    update[0]=(SetFn)load(code_original_2,sizeof(code_original_2),calls_original_2,2);
    update[1]=(SetFn)load(code_rebuilt_2,sizeof(code_rebuilt_2),calls_rebuilt_2,2);
    for(test=0;test<6000;test++) {
        index=test%11;splitIndex=test%9;mode=(test/11)%14;
        for(i=0;i<test%159;i++)name[i]=(char)('a'+(i+test)%26);name[i]=0;
        for(f=0;f<3;f++)prepare(f,test);
        expected=*(unsigned int *)(regions[0].data+offsets_original[1]+4*splitIndex);
        for(f=0;f<3;f++) {
            a=split[f](splitIndex);
            if(a!=expected) { if(f<2)differences++;else mutationSplit++; }
            calls=recordCalls=0;a=stage[f](index);
            expected=*(unsigned int *)(regions[f].data+regions[f].offsets[0]+
                       (mode==12 ? 0x520 : 0x51c+index*4));
            if(calls!=1 || recordCalls || a!=expected) {
                if(f<2)differences++;else mutationStage++;
            }
            if(memcmp(regions[f].data,regions[f].before,regions[f].size))return 3;
            /* Recompute the split oracle before the next image. */
            expected=*(unsigned int *)(regions[0].data+offsets_original[1]+4*splitIndex);
        }
        seed=test+591;value=test%5==0 ? 0 : next();
        calls=recordCalls=0;update[0](value,index);
        expectedCalls=calls;expectedRecordCalls=recordCalls;
        calls=recordCalls=0;update[1](value,index);
        if(calls!=expectedCalls || recordCalls!=expectedRecordCalls)differences++;
        for(i=0;i<2;i++) {
            int size=i ? 36 : 0x684;
            if(memcmp(regions[0].data+offsets_original[i],
                      regions[1].data+offsets_rebuilt[i],size))differences++;
        }
        guards(0);guards(1);guards(2);
    }
    printf("6000 network table cases: %d differences; full table regions and guards intact; wrong-cell mutations split %d, stage %d\n",differences,mutationSplit,mutationStage);
    return differences!=0 || !mutationSplit || !mutationStage;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    addresses = {int(e['address'], 16): int(e['recomp'], 16)
                 for e in report['data'] if e.get('recomp')}
    source = '#include <windows.h>\n'
    for n, name in enumerate(['original', 'rebuilt', 'mutation']):
        rebuilt = name == 'rebuilt'
        globals_ = [(entity_address(entities, a, 0x531e00 if a == 0x531f80 else None), size)
                            for a, size in GLOBALS] if rebuilt else GLOBALS
        low = min(a for a, _ in globals_)
        high = max(a + size for a, size in globals_)
        base = 0x21000000 + n * 0x1000000
        pe = pefile.PE(str(ROOT / ('build/CMR2.exe' if rebuilt else 'cmr2bin/CMR2.exe')))
        callees = [entities[hex(a)][0] for a in CALLEES] if rebuilt else CALLEES
        source += '#define BASE_%s %du\n#define SIZE_%s %d\n' % (name, base, name, high-low+128)
        source += 'static unsigned int offsets_%s[]={%s};\n' % (name, ','.join(str(a-low+64) for a, _ in globals_))
        for f, original in enumerate(FUNCTIONS):
            if name == 'mutation' and f == 2:
                continue
            address = addresses[original] if rebuilt else original
            code, calls = extract(pe, address, globals_, base+64, callees,
                                  mutation=name == 'mutation')
            assert len(calls) == f
            source += 'static BYTE code_%s_%d[]={%s};\n' % (name, f, ','.join(map(str, code)))
            source += 'static int calls_%s_%d[][2]={%s};\n' % (name, f, ','.join('{%d,%d}' % tuple(c) for c in calls) or '{0,0}')
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=str(TOOLS / 'wineprefix'))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-network-tables-') as tmp:
        (Path(tmp) / 'tables.c').write_text(source + DRIVER)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo',
                        '/O2', '/MD', 'tables.c', '/Fetables.exe'], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'tables.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
