#!/usr/bin/env python3
"""Execute both billboard initializers; compare all indices, flags and callback.

Usage: differential_billboard_init.py report.json entities.json
Preserves each image's distances between globals and checks intervening guards.
A mutation omitting the enabled store must fail the same comparison.
"""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

import capstone
import pefile

from differential_tables import relocate

ROOT = Path(__file__).resolve().parents[1]
TOOLS = Path(os.environ.get("CMR2_TOOLS", ROOT.parent / "tools"))
GLOBALS = [0x6db200, 0x6dd780, 0x6dd784, 0x6dd788]
MD = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
MD.detail = True


def extract(path, address, register, callback):
    pe = pefile.PE(str(path))
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 4096)
    instructions = {i.address: i for i in MD.disasm(data, address)}
    pending, visited, calls, callbacks = [address], set(), [], []
    while pending:
        pos = pending.pop()
        if pos in visited:
            continue
        ins = instructions[pos]
        visited.add(pos)
        if ins.group(capstone.CS_GRP_CALL):
            assert ins.operands[0].imm == register, 'Unexpected callee'
            calls.append(pos - address + ins.imm_offset)
        elif any(o.type == capstone.x86.X86_OP_IMM and o.imm == callback for o in ins.operands):
            callbacks.append(pos - address + ins.imm_offset)
        if ins.group(capstone.CS_GRP_RET):
            assert not ins.op_str
            continue
        if ins.group(capstone.CS_GRP_JUMP):
            assert ins.operands[0].type == capstone.x86.X86_OP_IMM
            pending.append(ins.operands[0].imm)
            if ins.mnemonic == 'jmp':
                continue
        pending.append(pos + ins.size)
    end = max(pos + instructions[pos].size for pos in visited)
    assert len(calls) == len(callbacks) == 1
    return data[:end - address], calls[0], callbacks[0]


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *InitFn)(void);
struct Region { BYTE *all, *data, *mask; int size; const unsigned int *offsets; };
static struct Region regions[3];
static struct Region *active;
static int calls, callbackValid, callbackState[3];
static void __stdcall resetMock(void) {}
static void __stdcall registerMock(void *callback, void *argument) {
    int i; calls++; callbackValid=(callback==(void *)resetMock && argument==0);
    for(i=0;i<3;i++) memcpy(&callbackState[i],active->data+active->offsets[i+1],4);
}
static InitFn load(const BYTE *bytes, int size, int callOffset, int callbackOffset) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    int relative; void *callback=resetMock;
    if (!p) exit(2);
    memcpy(p,bytes,size); relative=(BYTE *)registerMock-(p+callOffset+4);
    memcpy(p+callOffset,&relative,4); memcpy(p+callbackOffset,&callback,4);
    FlushInstructionCache(GetCurrentProcess(),p,size); return (InitFn)p;
}
static int check(int f, int caseId) {
    struct Region *r=&regions[f]; WORD *indices=(WORD *)(r->data+r->offsets[0]);
    int quad, i, value, failed=0, wanted[3]={1,0,0}; BYTE pattern=(BYTE)(caseId*31+0xa5);
    for(quad=0;quad<800;quad++) {
        int first=quad*4;
        if(indices[quad*6]!=first || indices[quad*6+1]!=first+3 ||
           indices[quad*6+2]!=first+2 || indices[quad*6+3]!=first ||
           indices[quad*6+4]!=first+1 || indices[quad*6+5]!=first+3) failed=1;
    }
    for(i=0;i<3;i++) {
        memcpy(&value,r->data+r->offsets[i+1],4);
        if(value!=wanted[i] || callbackState[i]!=wanted[i]) failed=1;
    }
    if(calls!=1 || !callbackValid) failed=1;
    for(i=0;i<r->size+128;i++) if(!r->mask[i] && r->all[i]!=pattern) exit(3);
    return failed;
}
int main(void) {
    InitFn functions[3]; int f, caseId, i, initial, differences[3]={0,0,0};
    functions[0]=load(code_original,sizeof(code_original),call_original,callback_original);
    functions[1]=load(code_rebuilt,sizeof(code_rebuilt),call_rebuilt,callback_rebuilt);
    functions[2]=load(code_mutation,sizeof(code_mutation),call_original,callback_original);
    for(f=0;f<3;f++) {
        struct Region *r=&regions[f]; r->size=sizes[f]; r->offsets=offsets[f];
        r->all=(BYTE *)VirtualAlloc((void *)bases[f],r->size+128,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
        if(r->all!=(BYTE *)bases[f]) return 2;
        r->data=r->all+64; r->mask=(BYTE *)calloc(r->size+128,1);
        if(!r->mask) return 2;
        memset(r->mask+64+r->offsets[0],1,9600);
        for(i=1;i<4;i++) memset(r->mask+64+r->offsets[i],1,4);
    }
    for(caseId=0;caseId<6000;caseId++) for(f=0;f<3;f++) {
        active=&regions[f]; memset(active->all,(BYTE)(caseId*31+0xa5),active->size+128);
        initial=caseId%3-1; memcpy(active->data+active->offsets[1],&initial,4);
        calls=callbackValid=0; memset(callbackState,0,sizeof(callbackState));
        functions[f](); differences[f]+=check(f,caseId);
    }
    printf("6000 billboard initializations: original failures %d, rebuilt failures %d; guards intact; missing-enable mutation failures %d\n",differences[0],differences[1],differences[2]);
    return differences[0]!=0 || differences[1]!=0 || differences[2]==0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    entry = next(e for e in report['data'] if int(e['address'], 16) == 0x4b1150)
    rebuilt = int(entry['recomp'], 16)
    assert rebuilt == entities['0x4b1150'][0]
    original, call, callback = extract(ROOT / 'cmr2bin/CMR2.exe', 0x4b1150, 0x49c0a0, 0x4b1500)
    current, call_new, callback_new = extract(ROOT / 'build/CMR2.exe', rebuilt,
                                             entities['0x49c0a0'][0], entities['0x4b1500'][0])
    mutation, stores = bytearray(original), 0
    for ins in MD.disasm(original, 0x4b1150):
        if ins.mnemonic == 'mov' and ins.operands[0].type == capstone.x86.X86_OP_MEM and ins.operands[0].mem.disp == 0x6dd780:
            assert ins.operands[1].imm == 1
            offset = ins.address - 0x4b1150
            mutation[offset:offset + ins.size] = b'\x90' * ins.size
            stores += 1
    assert stores == 1
    models = [('original', original, 0x4b1150, GLOBALS, call, callback),
              ('rebuilt', current, rebuilt, [entities[hex(a)][0] for a in GLOBALS], call_new, callback_new),
              ('mutation', mutation, 0x4b1150, GLOBALS, call, callback)]
    source, sizes, offsets, bases = '', [], [], []
    for f, (name, code, address, variables, call_offset, callback_offset) in enumerate(models):
        low = min(variables)
        high = max(a + (9600 if i == 0 else 4) for i, a in enumerate(variables))
        base = 0x21000000 + f * 0x01000000
        code = relocate(code, address, low, high, base + 64)
        source += 'static BYTE code_%s[]={%s};\n' % (name, ','.join(map(str, code)))
        source += 'static int call_%s=%d, callback_%s=%d;\n' % (name, call_offset, name, callback_offset)
        sizes.append(high - low); offsets.append([a - low for a in variables]); bases.append(base)
    source = '#include <windows.h>\n' + source
    source += 'static int sizes[3]={%s};\n' % ','.join(map(str, sizes))
    source += 'static unsigned int offsets[3][4]={%s}, bases[3]={%s};\n' % (
        ','.join('{%s}' % ','.join(map(str, row)) for row in offsets), ','.join(map(str, bases)))
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=os.environ.get('WINEPREFIX', str(TOOLS / 'wineprefix')))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-billboard-init-') as tmp:
        (Path(tmp) / 'billboard.c').write_text(source + DRIVER)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo',
                        '/O2', '/MD', 'billboard.c', '/Febillboard.exe'], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'billboard.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
