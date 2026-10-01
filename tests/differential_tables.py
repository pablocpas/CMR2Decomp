#!/usr/bin/env python3
"""Run real original/rebuilt table resets and keyboard queues with their own layouts.

Usage: python3 tests/differential_tables.py /tmp/reccmp.json [entities.json]
Keeps each executable's distances between globals; checks every buffer and guard.
Also compares event counters and verifies that the two wrong-address mutations
are detected, including a queue end placed before its buffer as in the old build.
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

from matching_entities import load_entities, entity_address

ROOT = Path(__file__).resolve().parents[1]
TOOLS = Path(os.environ.get("CMR2_TOOLS", ROOT.parent / "tools"))
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True
RESET = [(0x5335b8, 32), (0x5335d8, 24), (0x5335f0, 32),
         (0x533610, 8), (0x533618, 8), (0x533620, 8), (0x533628, 8)]
QUEUES = [(0x6ed3f4, 120), (0x6ed46c, 120)]
EVENTS = [(0x588ed8, 11 * 28)]
FUNCTIONS = [0x40cc60, 0x4b7ca0, 0x4b7cd0, 0x4b7d10, 0x4b7d60, 0x46ed40]


def extract(pe, address):
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 4096)
    instructions = {i.address: i for i in MD.disasm(data, address)}
    pending, visited = [address], set()
    while pending:
        pos = pending.pop()
        if pos in visited:
            continue
        ins = instructions[pos]
        visited.add(pos)
        assert not ins.group(capstone.CS_GRP_CALL), 'Unexpected call'
        if ins.group(capstone.CS_GRP_RET):
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
    return data[:end - address]


def relocate(data, address, low, high, destination):
    output = bytearray(data)
    for ins in MD.disasm(data, address):
        for operand in ins.operands:
            if operand.type == capstone.x86.X86_OP_MEM:
                value, offset, size = operand.mem.disp, ins.disp_offset, ins.disp_size
            elif operand.type == capstone.x86.X86_OP_IMM and not ins.group(capstone.CS_GRP_JUMP):
                value, offset, size = operand.imm, ins.imm_offset, ins.imm_size
            else:
                continue
            if low <= value < high:
                assert size == 4
                struct.pack_into('<I', output, ins.address - address + offset,
                                 destination + value - low)
    return output


def mutate(data, address, old, new):
    output, changes = bytearray(data), 0
    for ins in MD.disasm(data, address):
        for operand in ins.operands:
            if operand.type == capstone.x86.X86_OP_MEM:
                value, offset, size = operand.mem.disp, ins.disp_offset, ins.disp_size
            elif operand.type == capstone.x86.X86_OP_IMM and not ins.group(capstone.CS_GRP_JUMP):
                value, offset, size = operand.imm, ins.imm_offset, ins.imm_size
            else:
                continue
            if value == old:
                assert size == 4
                struct.pack_into('<I', output, ins.address - address + offset, new)
                changes += 1
    assert old != new, 'Mutation must change the target'
    assert changes, 'Mutation did not replace any address'
    return output


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *ResetFn)(void);
typedef void (__stdcall *AppendFn)(int);
typedef BYTE (__stdcall *PopByteFn)(int *);
typedef int (__stdcall *PopIntFn)(int *);
struct Region { BYTE *data, *before, *mask; int size, count; const unsigned int *offsets, *lengths; };
typedef BYTE (__stdcall *TickFn)(int);
static struct Region regions[6];
static void *exec_code(const BYTE *data, unsigned int size) {
    void *p = VirtualAlloc(0, size, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!p) exit(2);
    memcpy(p, data, size); FlushInstructionCache(GetCurrentProcess(), p, size); return p;
}
static unsigned int seed;
static unsigned int next(void) { seed = seed * 1664525u + 1013904223u; return seed; }
static void setup(int n, unsigned int base, int size, int count,
                  const unsigned int *offsets, const unsigned int *lengths) {
    int i; struct Region *r = &regions[n];
    r->data = (BYTE *)VirtualAlloc((void *)base, size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (r->data != (BYTE *)base) exit(2);
    r->before=(BYTE *)malloc(size); r->mask=(BYTE *)calloc(size,1);
    if (!r->before || !r->mask) exit(2);
    r->size=size;r->count=count;r->offsets=offsets;r->lengths=lengths;
    for (i=0;i<count;i++) memset(r->mask+offsets[i],1,lengths[i]);
}
static int equal_and_guards(int a, int b) {
    int i,j; struct Region *ra=&regions[a], *rb=&regions[b];
    for(i=0;i<ra->count;i++)
        if(memcmp(ra->data+ra->offsets[i],rb->data+rb->offsets[i],ra->lengths[i]))return 0;
    for(j=a;j<=b;j++) {
        struct Region *r=&regions[j];
        for(i=0;i<r->size;i++) if(!r->mask[i] && r->data[i]!=r->before[i])return 0;
    }
    return 1;
}
static int reset_cases(ResetFn a, ResetFn b, int count) {
    int test,i,k; unsigned int value;
    for(test=0;test<count;test++) {
        seed=test+312;memset(regions[0].data,0xa5,regions[0].size);
        memset(regions[1].data,0xa5,regions[1].size);
        for(i=0;i<7;i++)for(k=0;k<(int)regions[0].lengths[i];k++) {
            value=next();regions[0].data[regions[0].offsets[i]+k]=(BYTE)value;
            regions[1].data[regions[1].offsets[i]+k]=(BYTE)value;
        }
        for(i=0;i<2;i++)memcpy(regions[i].before,regions[i].data,regions[i].size);
        a();b();if(!equal_and_guards(0,1))return test+1;
    }
    return 0;
}
struct Output { BYTE before[16]; int value; BYTE after[16]; };
static int queue_cases(AppendFn aa, PopByteFn pa, AppendFn ab, PopIntFn pb,
                       AppendFn ba, PopByteFn qa, AppendFn bb, PopIntFn qb, int count) {
    int test,i,k,n,action,va,vb; struct Output oa,ob;
    for(test=0;test<count;test++) {
        seed=test+717;memset(regions[2].data,0xa5,regions[2].size);
        memset(regions[3].data,0xa5,regions[3].size);
        for(i=0;i<2;i++) {
            int *a=(int *)(regions[2].data+regions[2].offsets[i]);
            int *b=(int *)(regions[3].data+regions[3].offsets[i]);
            memset(a,0,120);memset(b,0,120);n=(test+i*7)%31;
            for(k=0;k<n;k++)a[k]=b[k]=(int)(next()|1);
        }
        for(i=2;i<4;i++)memcpy(regions[i].before,regions[i].data,regions[i].size);
        for(action=0;action<12;action++) {
            int value=(int)(next()|1);
            if(action%4==0) { aa(value);ba(value); }
            else if(action%4==2) { ab(value);bb(value); }
            else {
                memset(&oa,0xa5,sizeof(oa));ob=oa;
                if(action%4==1){va=pa(&oa.value);vb=qa(&ob.value);}
                else {va=pb(&oa.value);vb=qb(&ob.value);}
                if(va!=vb||memcmp(&oa,&ob,sizeof(oa)))return test+1;
            }
            if(!equal_and_guards(2,3))return test+1;
        }
    }
    return 0;
}
static int tick_cases(TickFn a, TickFn b) {
    static unsigned short values[]={0,1,254,255,256,32767,65535};
    int test,i,k,index,va,vb;BYTE *ea,*eb;
    for(test=0;test<6000;test++) {
        seed=test+195;index=test%11;
        memset(regions[4].data,0xa5,regions[4].size);
        memset(regions[5].data,0xa5,regions[5].size);
        ea=regions[4].data+regions[4].offsets[0];
        eb=regions[5].data+regions[5].offsets[0];
        for(k=0;k<308;k++)ea[k]=eb[k]=(BYTE)next();
        ea+=index*28;eb+=index*28;
        ea[24]=eb[24]=(BYTE)(test%3==0);
        *(unsigned short *)(ea+16)=*(unsigned short *)(eb+16)=
            test<49?values[test%7]:(unsigned short)next();
        *(unsigned short *)(ea+20)=*(unsigned short *)(eb+20)=
            test<49?values[test/7]:(unsigned short)next();
        for(i=4;i<6;i++)memcpy(regions[i].before,regions[i].data,regions[i].size);
        va=a(index);vb=b(index);
        if(va!=vb||!equal_and_guards(4,5))return test+1;
    }
    return 0;
}
int main(void) {
    ResetFn reset_a,reset_b,reset_bad;
    AppendFn aa,ab,ba,bb,bad; PopByteFn pa,qa; PopIntFn pb,qb;
    TickFn tick_a,tick_b;
    int result;
    setup(0,0x21000000,region_size_0,7,offsets_0,lengths_0);
    setup(1,0x21100000,region_size_1,7,offsets_1,lengths_1);
    setup(2,0x21200000,region_size_2,2,offsets_2,lengths_2);
    setup(3,0x21300000,region_size_3,2,offsets_3,lengths_3);
    setup(4,0x21400000,region_size_4,1,offsets_4,lengths_4);
    setup(5,0x21500000,region_size_5,1,offsets_5,lengths_5);
    tick_a=(TickFn)exec_code(tick_original,sizeof(tick_original));
    tick_b=(TickFn)exec_code(tick_rebuilt,sizeof(tick_rebuilt));
    reset_a=(ResetFn)exec_code(reset_original,sizeof(reset_original));
    reset_b=(ResetFn)exec_code(reset_rebuilt,sizeof(reset_rebuilt));
    reset_bad=(ResetFn)exec_code(reset_mutated,sizeof(reset_mutated));
    aa=(AppendFn)exec_code(append_a_original,sizeof(append_a_original));
    ba=(AppendFn)exec_code(append_a_rebuilt,sizeof(append_a_rebuilt));
    bad=(AppendFn)exec_code(append_a_mutated,sizeof(append_a_mutated));
    ab=(AppendFn)exec_code(append_b_original,sizeof(append_b_original));
    bb=(AppendFn)exec_code(append_b_rebuilt,sizeof(append_b_rebuilt));
    pa=(PopByteFn)exec_code(pop_a_original,sizeof(pop_a_original));
    qa=(PopByteFn)exec_code(pop_a_rebuilt,sizeof(pop_a_rebuilt));
    pb=(PopIntFn)exec_code(pop_b_original,sizeof(pop_b_original));
    qb=(PopIntFn)exec_code(pop_b_rebuilt,sizeof(pop_b_rebuilt));
    result=reset_cases(reset_a,reset_b,6000);
    if(result){printf("reset differs at case %d\n",result-1);return 1;}
    result=queue_cases(aa,pa,ab,pb,ba,qa,bb,qb,6000);
    if(result){printf("queues differ at case %d\n",result-1);return 1;}
    result=tick_cases(tick_a,tick_b);
    if(result){printf("event tick differs at case %d\n",result-1);return 1;}
    if(!reset_cases(reset_a,reset_bad,20)||!queue_cases(aa,pa,ab,pb,bad,qa,bb,qb,40)) {
        printf("wrong-address mutation was not detected\n");return 1;
    }
    printf("6000 resets, 6000 queue sequences and 6000 event ticks: no differences or guard writes; both wrong-address mutations detected\n");
    return 0;
}
'''


def main():
    report = {int(r['address'], 16): r for r in json.loads(Path(sys.argv[1]).read_text())['data']}
    entities = load_entities(sys.argv[2] if len(sys.argv) > 2 else None)
    # Championship fields are member views of the contiguous owner introduced
    # in the previous matching batch; resolve them only for native relocation.
    for address, _ in RESET:
        entities.setdefault(hex(address), [entity_address(entities, address, 0x5335b8),
                                           'ChampionshipTables member'])
    if '0x6ed46c' not in entities:
        queues = entities['0x6ed3f4'][0]
        entities['0x6ed46c'] = [queues + 120, 'g_inputQueues.keys']
    pes = [pefile.PE(str(ROOT / 'cmr2bin/CMR2.exe')),
           pefile.PE(str(ROOT / 'build/CMR2.exe'))]
    code, c = {}, ''
    for model, globals_ in enumerate([RESET, RESET, QUEUES, QUEUES, EVENTS, EVENTS]):
        side = model % 2
        addresses = [a if side == 0 else entities[hex(a)][0] for a, _ in globals_]
        low = min(addresses) & ~0xfff
        high = (max(a + n for a, (_, n) in zip(addresses, globals_)) + 0xfff) & ~0xfff
        destination = 0x21000000 + model * 0x100000
        c += 'static const int region_size_%d = %d;\n' % (model, high - low)
        c += 'static const unsigned int offsets_%d[] = {%s};\n' % (
            model, ','.join(str(a-low) for a in addresses))
        c += 'static const unsigned int lengths_%d[] = {%s};\n' % (
            model, ','.join(str(n) for _, n in globals_))
        targets = FUNCTIONS[:1] if model < 2 else FUNCTIONS[1:5] if model < 4 else FUNCTIONS[5:]
        for original in targets:
            address = original if side == 0 else entities[hex(original)][0]
            if side:
                assert address == int(report[original]['recomp'], 16), 'Stale entity map'
            raw = extract(pes[side], address)
            code[model, original] = relocate(raw, address, low, high, destination)
        if model == 1:
            # A one-DWORD error in the first column must affect a buffer or its
            # guard. The old gap-based mutation equals the correct address now
            # that these fields belong to one contiguous ChampionshipTables.
            code['reset_bad'] = mutate(code[model, 0x40cc60], entities['0x40cc60'][0],
                                      destination + addresses[0] - low,
                                      destination + addresses[0] - low + 4)
        if model == 3:
            code['queue_bad'] = mutate(code[model, 0x4b7ca0], entities['0x4b7ca0'][0],
                                      destination + addresses[0] - low + 120,
                                      destination + addresses[0] - low - 120)
    for model in range(6):
        for original, name in zip(FUNCTIONS, ['reset', 'append_a', 'pop_a', 'append_b', 'pop_b', 'tick']):
            if (model, original) not in code:
                continue
            suffix = 'original' if model % 2 == 0 else 'rebuilt'
            c += 'static const BYTE %s_%s[] = {%s};\n' % (
                name, suffix, ','.join(str(b) for b in code[model, original]))
    for key, name in [('reset_bad', 'reset_mutated'), ('queue_bad', 'append_a_mutated')]:
        c += 'static const BYTE %s[] = {%s};\n' % (name, ','.join(str(b) for b in code[key]))
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=os.environ.get('WINEPREFIX', str(TOOLS / 'wineprefix')))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-tables-') as tmp:
        (Path(tmp) / 'tables.c').write_text('#include <windows.h>\n' + c + DRIVER)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo',
                        '/O2', '/MD', 'tables.c', '/Fetables.exe'], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'tables.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
