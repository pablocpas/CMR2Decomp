#!/usr/bin/env python3
"""Compare the native quick-race back callback and its early return.

Usage: differential_menu_back.py report.json entities.json
Checks eight unsigned option values, back bytes, call order and read-only input.
A mutation deleting the early return must produce extra calls.
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
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True


def extract(pe, address, callees):
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 256)
    instructions = {i.address: i for i in MD.disasm(data, address)}
    pending, visited, fixups, returns = [address], set(), [], []
    while pending:
        pos = pending.pop()
        if pos in visited:
            continue
        ins = instructions[pos]
        visited.add(pos)
        for op in ins.operands:
            if op.type == capstone.x86.X86_OP_MEM:
                assert op.mem.disp < 0x400000, 'Unexpected global'
        if ins.group(capstone.CS_GRP_CALL):
            assert ins.operands[0].type == capstone.x86.X86_OP_IMM
            fixups.append((pos - address + ins.imm_offset,
                           callees.index(ins.operands[0].imm)))
        if ins.group(capstone.CS_GRP_RET):
            assert ins.op_str == '8'
            returns.append(pos - address)
            continue
        if ins.group(capstone.CS_GRP_JUMP):
            target = ins.operands[0].imm
            assert address <= target < address + 256
            pending.append(target)
            if ins.mnemonic == 'jmp':
                continue
        pending.append(pos + ins.size)
    end = max(pos + instructions[pos].size for pos in visited)
    assert len(fixups) == 2 and len(returns) == 2
    return data[:end - address], sorted(fixups), sorted(returns)


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef void (__stdcall *Callback)(void *,char);
static int calls,entries[16][3];
static void __stdcall back(void){entries[calls][0]=0;entries[calls][1]=0;entries[calls++][2]=0;}
static void __stdcall update(int index,int value){
    if(calls>=16)exit(3);entries[calls][0]=1;entries[calls][1]=index;entries[calls++][2]=value;
}
static void *mocks[]={back,update};
static void *load(const BYTE *code,int size,const int (*fixups)[2]){
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int i,offset;if(!p)exit(2);memcpy(p,code,size);
    for(i=0;i<2;i++){
        offset=(BYTE *)mocks[fixups[i][1]]-(p+fixups[i][0]+4);
        memcpy(p+fixups[i][0],&offset,4);
    }
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
int main(void){
    Callback callbacks[3];BYTE initial[0x180],current[0x180];
    char backs[5]={0,1,-1,127,-128};int f,test,i,expectedCount,differences[3]={0,0,0};
    callbacks[0]=(Callback)load(code_original,sizeof(code_original),calls_original);
    callbacks[1]=(Callback)load(code_rebuilt,sizeof(code_rebuilt),calls_rebuilt);
    callbacks[2]=(Callback)load(code_mutation,sizeof(code_mutation),calls_original);
    for(test=0;test<6000;test++){
        memset(initial,0xa5,sizeof(initial));
        for(i=0;i<8;i++)initial[64+0x1f+i*20]=(BYTE)(test*17+i*31);
        expectedCount=backs[test%5] ? 1 : 8;
        for(f=0;f<3;f++){
            memcpy(current,initial,sizeof(current));calls=0;callbacks[f](current+64,backs[test%5]);
            if(memcmp(current,initial,sizeof(current)))return 3;
            if(calls!=expectedCount){differences[f]++;continue;}
            for(i=0;i<calls;i++){
                if(backs[test%5]){
                    if(entries[i][0]!=0)differences[f]++;
                }else if(entries[i][0]!=1 || entries[i][1]!=i || entries[i][2]!=initial[64+0x1f+i*20])differences[f]++;
            }
        }
    }
    printf("6000 menu back callbacks: original %d differences, rebuilt %d; calls, unsigned values and input guards checked; missing-return mutation %d\n",differences[0],differences[1],differences[2]);
    return differences[0]!=0 || differences[1]!=0 || differences[2]==0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    entry = next(e for e in report['data'] if int(e['address'], 16) == 0x4f2970)
    source = '#include <windows.h>\n'
    for n, name in enumerate(['original', 'rebuilt']):
        pe = pefile.PE(str(ROOT / ('build/CMR2.exe' if n else 'cmr2bin/CMR2.exe')))
        callees = [entities[hex(a)][0] if n else a for a in [0x4eaba0, 0x4eab40]]
        code, calls, returns = extract(pe, int(entry['recomp'], 16) if n else 0x4f2970, callees)
        source += 'static BYTE code_%s[]={%s};\n' % (name, ','.join(map(str, code)))
        source += 'static int calls_%s[][2]={%s};\n' % (name, ','.join('{%d,%d}' % c for c in calls))
        if n == 0:
            mutation = bytearray(code)
            assert mutation[returns[0]:returns[0]+3] == b'\xc2\x08\x00'
            mutation[returns[0]:returns[0]+3] = b'\x90\x90\x90'
            source += 'static BYTE code_mutation[]={%s};\n' % ','.join(map(str, mutation))
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=str(TOOLS / 'wineprefix'))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-menu-back-') as tmp:
        (Path(tmp) / 'back.c').write_text(source + DRIVER)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo',
                        '/O2', '/MD', 'back.c', '/Feback.exe'], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'back.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
