#!/usr/bin/env python3
"""Run native replay initialization and dispatch with each image's global layout.

Usage: differential_replay_slots.py report.json entities.json
Checks all sixteen slot pointers, buffer resets, call order and guarded memory.
Restricting either dispatcher to eight slots must be detected. Replay callees
are mocked; this test does not simulate replay decoding or car physics.
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
GLOBALS = [(0x588d40, 64), (0x588e80, 32), (0x588ea0, 32),
           (0x588d3c, 4), (0x588d14, 4)]
FUNCTIONS = [0x46c540, 0x46d270, 0x46d5e0]
CALLEES = [0x406320, 0x405d80, 0x46cfa0, 0x46d610]
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
            assert not ins.op_str
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
    low = min(a for a, size in globals_)
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
            if low <= value <= high:
                assert size == 4
                if mutation and value == globals_[0][0] + 64:
                    value -= 32
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
typedef void (__stdcall *Function)(void);
struct Region { BYTE *data, *before, *mask; int size; const unsigned int *offsets; };
static struct Region regions[3];
static BYTE objects[16][64];
static int caseId, slot, count, trace[64][2];
static void record(int kind, int value) {
    if(count>=64)exit(2);trace[count][0]=kind;trace[count++][1]=value;
}
static int __stdcall flag(void) {
    slot++;record(0,slot);return (caseId+slot)%3;
}
static int __stdcall mode(void) {
    int value=((caseId+slot)/3)%4==0 ? 6 : (caseId+slot)%12;
    record(1,value);return value;
}
static void update(int kind, void *p) {
    int index;
    if(!p)index=-1;
    else {
        index=((BYTE *)p-(BYTE *)objects)/64;
        if(index<0 || index>=16 || p!=objects[index])exit(3);
    }
    record(kind,index);
}
static void __stdcall play(void *p) { update(2,p); }
static void __stdcall capture(void *p) { update(3,p); }
static void *mocks[]={flag,mode,play,capture};
static Function load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,offset;if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++) {
        offset=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);
        memcpy(p+calls[i][0],&offset,4);
    }
    FlushInstructionCache(GetCurrentProcess(),p,size);return (Function)p;
}
static void setup(int n,unsigned int base,int size,const unsigned int *offsets) {
    static int lengths[]={64,32,32,4,4};int i;struct Region *r=&regions[n];
    r->data=(BYTE *)VirtualAlloc((void *)base,size,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(r->data!=(BYTE *)base)exit(2);r->size=size;r->offsets=offsets;
    r->before=(BYTE *)malloc(size);r->mask=(BYTE *)calloc(size,1);
    if(!r->before || !r->mask)exit(2);
    for(i=0;i<5;i++)memset(r->mask+offsets[i],1,lengths[i]);
}
static void prepare(int n,Function init) {
    struct Region *r=&regions[n];void ***slots=(void ***)(r->data+r->offsets[0]);
    void **first=(void **)(r->data+r->offsets[1]);
    void **second=(void **)(r->data+r->offsets[2]);int i;
    memset(r->data,0xa5,r->size);init();
    for(i=0;i<8;i++) {
        if(first[i] || second[i] || slots[i]!=first+i || slots[i+8]!=second+i)exit(3);
    }
    if(*(int *)(r->data+r->offsets[3]) || *(int *)(r->data+r->offsets[4]))exit(3);
    for(i=0;i<r->size;i++)if(!r->mask[i] && r->data[i]!=0xa5)exit(3);
    for(i=0;i<16;i++)*slots[i]=(caseId+i)%7==0 ? 0 : objects[(caseId+i)%16];
    memcpy(r->before,r->data,r->size);
}
int main(void) {
    Function init[3],dispatch[3][2];int f,k,expected[64][2],expectedCount;
    int differences[2]={0,0},mutations[2]={0,0};BYTE objectCopy[sizeof(objects)];
    setup(0,BASE_original,SIZE_original,offsets_original);
    setup(1,BASE_rebuilt,SIZE_rebuilt,offsets_rebuilt);
    setup(2,BASE_mutation,SIZE_mutation,offsets_mutation);
    init[0]=load(code_original_0,sizeof(code_original_0),calls_original_0,0);
    init[1]=load(code_rebuilt_0,sizeof(code_rebuilt_0),calls_rebuilt_0,0);
    dispatch[0][0]=load(code_original_1,sizeof(code_original_1),calls_original_1,3);
    dispatch[0][1]=load(code_original_2,sizeof(code_original_2),calls_original_2,3);
    dispatch[1][0]=load(code_rebuilt_1,sizeof(code_rebuilt_1),calls_rebuilt_1,3);
    dispatch[1][1]=load(code_rebuilt_2,sizeof(code_rebuilt_2),calls_rebuilt_2,3);
    dispatch[2][0]=load(code_mutation_1,sizeof(code_mutation_1),calls_mutation_1,3);
    dispatch[2][1]=load(code_mutation_2,sizeof(code_mutation_2),calls_mutation_2,3);
    /* Mutation has its own relocated initializer and guarded region. */
    init[2]=load(code_mutation_0,sizeof(code_mutation_0),calls_mutation_0,0);
    memset(objects,0xa5,sizeof(objects));memcpy(objectCopy,objects,sizeof(objects));
    for(caseId=0;caseId<6000;caseId++) {
        for(f=0;f<3;f++)prepare(f,init[f]);
        for(k=0;k<2;k++)for(f=0;f<3;f++) {
            slot=-1;count=0;memset(trace,0,sizeof(trace));dispatch[f][k]();
            if(memcmp(regions[f].data,regions[f].before,regions[f].size) ||
               memcmp(objects,objectCopy,sizeof(objects)))return 3;
            if(!f) {
                if(slot!=15)return 3;
                expectedCount=count;memcpy(expected,trace,sizeof(trace));
            } else if(expectedCount!=count || memcmp(expected,trace,sizeof(trace))) {
                if(f==1)differences[k]++;else mutations[k]++;
            }
        }
    }
    printf("6000 replay initializations and dispatches: play differences %d, record differences %d; all 16 slots and guards checked; eight-slot mutation differences %d/%d\n",differences[0],differences[1],mutations[0],mutations[1]);
    return differences[0]!=0 || differences[1]!=0 || !mutations[0] || !mutations[1];
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    addresses = {int(e['address'], 16): int(e['recomp'], 16)
                 for e in report['data'] if e.get('recomp')}
    rebuilt_globals = [(entity_address(entities, a, 0x588cd4 if a == 0x588d14 else None), size)
                            for a, size in GLOBALS]
    source = '#include <windows.h>\n'
    for i, name in enumerate(['original', 'rebuilt', 'mutation']):
        rebuilt = name == 'rebuilt'
        globals_ = rebuilt_globals if rebuilt else GLOBALS
        low = min(a for a, size in globals_)
        high = max(a + size for a, size in globals_)
        base = 0x21000000 + i * 0x1000000
        destination = base + 64
        pe = pefile.PE(str(ROOT / ('build/CMR2.exe' if rebuilt else 'cmr2bin/CMR2.exe')))
        callees = [entities[hex(a)][0] for a in CALLEES] if rebuilt else CALLEES
        source += '#define BASE_%s %du\n#define SIZE_%s %d\n' % (name, base, name, high-low+128)
        source += 'static unsigned int offsets_%s[]={%s};\n' % (name, ','.join(str(a-low+64) for a, _ in globals_))
        for f, original in enumerate(FUNCTIONS):
            address = addresses[original] if rebuilt else original
            code, calls = extract(pe, address, globals_, destination, callees,
                                  mutation=name == 'mutation' and f > 0)
            assert len(calls) == (3 if f else 0)
            source += 'static BYTE code_%s_%d[]={%s};\n' % (name, f, ','.join(map(str, code)))
            source += 'static int calls_%s_%d[][2]={%s};\n' % (name, f, ','.join('{%d,%d}' % tuple(c) for c in calls) or '{0,0}')
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=str(TOOLS / 'wineprefix'))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-replay-slots-') as tmp:
        (Path(tmp) / 'slots.c').write_text(source + DRIVER)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo',
                        '/O2', '/MD', 'slots.c', '/Feslots.exe'], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'slots.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
