#!/usr/bin/env python3
"""Compare real car-effect dispatch code, mocking callees and recording arguments.

Usage: differential_effect_calls.py report.json entities.json [old-code.json]
Use --capture report.json entities.json output.json before rebuilding to save
the old implementation as a regression mutation. Does not simulate particle physics.
"""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

import capstone
import pefile

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT.parent / 'tools'
CALLEES = [0x42b5f0, 0x405cd0, 0x463270, 0x45d1e0, 0x422f50,
           0x45d540, 0x45b580, 0x45c820, 0x49b3e0, 0x45af90]
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True


def extract(path, address, targets):
    pe = pefile.PE(str(path))
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 4096)
    instructions = {i.address: i for i in MD.disasm(data, address)}
    pending, visited, calls = [address], set(), []
    while pending:
        pos = pending.pop()
        if pos in visited:
            continue
        ins = instructions[pos]
        visited.add(pos)
        for op in ins.operands:
            if op.type == capstone.x86.X86_OP_MEM:
                assert not (0x400000 <= op.mem.disp < 0x900000), 'Unexpected global'
        if ins.group(capstone.CS_GRP_CALL):
            assert ins.operands[0].type == capstone.x86.X86_OP_IMM
            calls.append([pos - address + 1, targets.index(ins.operands[0].imm)])
        if ins.group(capstone.CS_GRP_RET):
            assert ins.op_str == '4'
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
    return {'code': list(data[:end - address]), 'calls': sorted(calls)}


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *EffectsFn)(int);
static unsigned char car[0xc80], initialCar[0xc80], colour[4];
static int mode, type, hasColour, count, trace[16][4];
static void record(int kind, int index, int value, int extra) {
    if (count >= 16) exit(2);
    trace[count][0]=kind; trace[count][1]=index;
    trace[count][2]=value; trace[count++][3]=extra;
}
static void *__stdcall getCar(int index) { record(0,index,0,0); return car+32; }
static int __stdcall getMode(void) { record(1,0,0,0); return mode; }
static void *__stdcall getColour(int index, int slot) {
    record(2,index,slot,0); return hasColour ? colour : 0;
}
static void __stdcall draw(int index, unsigned char *pColour) {
    int value; memcpy(&value,pColour,4); record(3,index,value,0);
}
static int __stdcall getType(int index) { record(4,index,0,0); return type; }
static void __stdcall dust(int i) { record(5,i,0,0); }
static void __stdcall particles(int i) { record(6,i,0,0); }
static void __stdcall splash(int i) { record(7,i,0,0); }
static void __stdcall spray(int i) { record(8,i,0,0); }
static void __stdcall exhaust(int i) { record(9,i,0,0); }
static void *mocks[] = {getCar,getMode,getColour,draw,getType,dust,
                       particles,splash,spray,exhaust};
static EffectsFn load(const unsigned char *bytes, int size, const int (*calls)[2], int n) {
    unsigned char *p=(unsigned char *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i, offset;
    if (!p) exit(2);
    memcpy(p,bytes,size);
    for (i=0;i<n;i++) {
        offset=(unsigned char *)mocks[calls[i][1]]-(p+calls[i][0]+4);
        memcpy(p+calls[i][0],&offset,4);
    }
    FlushInstructionCache(GetCurrentProcess(),p,size); return (EffectsFn)p;
}
int main(void) {
    EffectsFn functions[3]; int caseId, f, i, expected[16][4], expectedCount;
    int differences[3]={0,0,0}; unsigned int seed=123;
    functions[0]=load(code_original,sizeof(code_original),calls_original,sizeof(calls_original)/sizeof(calls_original[0]));
    functions[1]=load(code_rebuilt,sizeof(code_rebuilt),calls_rebuilt,sizeof(calls_rebuilt)/sizeof(calls_rebuilt[0]));
    functions[2]=load(code_mutation,sizeof(code_mutation),calls_mutation,sizeof(calls_mutation)/sizeof(calls_mutation[0]));
    for (caseId=0;caseId<6000;caseId++) {
        int index=caseId%12, disabled=(caseId/12)%3;
        mode=(caseId/36)%4; type=(caseId/144)%5; hasColour=(caseId/720)%2;
        memset(initialCar,0xa5,sizeof(initialCar));
        memcpy(initialCar+32+0xc0c,&disabled,4);
        for(i=0;i<4;i++) { seed=seed*1664525u+1013904223u; colour[i]=(unsigned char)(seed>>24); }
        for(f=0;f<3;f++) {
            memcpy(car,initialCar,sizeof(car)); memset(trace,0,sizeof(trace)); count=0;
            functions[f](index);
            if (memcmp(car,initialCar,sizeof(car))) return 3;
            if (!f) { memcpy(expected,trace,sizeof(trace)); expectedCount=count; }
            else if(count!=expectedCount || memcmp(trace,expected,sizeof(trace))) differences[f]++;
        }
    }
    printf("6000 effect dispatches: %d differences; car data and guards intact; mutation differences: %d\n",differences[1],differences[2]);
    return differences[1]!=0 || (EXPECT_MUTATION && differences[2]==0);
}
'''


def main():
    capture = sys.argv[1] == '--capture'
    args = sys.argv[2:] if capture else sys.argv[1:]
    report = json.loads(Path(args[0]).read_text())
    entities = json.loads(Path(args[1]).read_text())
    entry = next(e for e in report['data'] if int(e['address'], 16) == 0x45af00)
    address = int(entry['recomp'], 16)
    assert address == entities['0x45af00'][0], 'Report and entity cache differ'
    rebuilt = extract(ROOT / 'build/CMR2.exe', address,
                      [entities[hex(a)][0] for a in CALLEES])
    if capture:
        Path(args[2]).write_text(json.dumps(rebuilt))
        return
    original = extract(ROOT / 'cmr2bin/CMR2.exe', 0x45af00, CALLEES)
    mutation = json.loads(Path(args[2]).read_text()) if len(args) > 2 else rebuilt
    arrays = '#define EXPECT_MUTATION %d\n' % (len(args) > 2)
    for name, model in [('original', original), ('rebuilt', rebuilt), ('mutation', mutation)]:
        arrays += 'static unsigned char code_%s[]={%s};\n' % (name, ','.join(map(str, model['code'])))
        arrays += 'static int calls_%s[][2]={%s};\n' % (name, ','.join('{%d,%d}' % tuple(c) for c in model['calls']))
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=str(TOOLS / 'wineprefix'))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-effect-calls-') as tmp:
        (Path(tmp) / 'effects.c').write_text(arrays + DRIVER)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo',
                        '/O2', '/MD', 'effects.c', '/Feeffects.exe'], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'effects.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
