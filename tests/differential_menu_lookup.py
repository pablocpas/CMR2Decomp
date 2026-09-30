#!/usr/bin/env python3
"""Compare native menu lookup code with distinct +4 and +8 fields and guards.

Usage: differential_menu_lookup.py report.json
Includes duplicate tags, signed counts/words, missing tags and wide queries.
An offset mutation restoring the old +4 lookup must be detected.
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


def extract(path, address):
    pe = pefile.PE(str(path))
    data = pe.get_data(address - pe.OPTIONAL_HEADER.ImageBase, 4096)
    for ins in MD.disasm(data, address):
        assert not ins.group(capstone.CS_GRP_CALL), 'Unexpected call'
        for operand in ins.operands:
            if operand.type == capstone.x86.X86_OP_MEM:
                assert operand.mem.disp < 0x400000, 'Unexpected global'
        if ins.group(capstone.CS_GRP_RET):
            assert ins.op_str == '8'
            return data[:ins.address + ins.size - address]
    raise RuntimeError('Missing return')


DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <string.h>
typedef int (__stdcall *FindFn)(void *, int);
struct Input { BYTE before[64], menu[0x1e0], after[64]; };
static struct Input initial, current;
static void *load(const BYTE *bytes, int size) {
    void *p=VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    if(!p) return 0;
    memcpy(p,bytes,size); FlushInstructionCache(GetCurrentProcess(),p,size); return p;
}
int main(void) {
    FindFn functions[3]; int caseId, f, i, count, query, expected, result;
    int differences[3]={0,0,0}; unsigned int seed=123;
    functions[0]=(FindFn)load(code_original,sizeof(code_original));
    functions[1]=(FindFn)load(code_rebuilt,sizeof(code_rebuilt));
    functions[2]=(FindFn)load(code_mutation,sizeof(code_mutation));
    if(!functions[0] || !functions[1] || !functions[2]) return 2;
    for(caseId=0;caseId<6000;caseId++) {
        short tag, previous=0, label;
        count=caseId%26-3;
        memset(&initial,0xa5,sizeof(initial)); initial.menu[6]=(BYTE)count;
        for(i=0;i<22;i++) {
            seed=seed*1664525u+1013904223u; tag=(short)(seed>>16);
            if(i%5==1) tag=previous;
            label=(short)(tag^0x55aa);
            memcpy(initial.menu+0x14+i*20+4,&label,2);
            memcpy(initial.menu+0x14+i*20+8,&tag,2); previous=tag;
        }
        if(caseId%3==0 && count>0) {
            memcpy(&tag,initial.menu+0x14+(caseId%count)*20+8,2); query=tag;
        } else if(caseId%3==1) query=0x10000+caseId;
        else { seed=seed*1664525u+1013904223u; query=(short)(seed>>16); }
        expected=-1;
        for(i=0;i<count;i++) {
            memcpy(&tag,initial.menu+0x14+i*20+8,2);
            if(tag==query) { expected=i; break; }
        }
        for(f=0;f<3;f++) {
            current=initial; result=functions[f](current.menu,query);
            if(result!=expected) differences[f]++;
            if(memcmp(&current,&initial,sizeof(initial))) return 3;
        }
    }
    printf("6000 menu lookups: original differences %d, rebuilt differences %d; input and guards intact; old-field mutation differences %d\n",differences[0],differences[1],differences[2]);
    return differences[0]!=0 || differences[1]!=0 || differences[2]==0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entry = next(e for e in report['data'] if int(e['address'], 16) == 0x4a0380)
    original = extract(ROOT / 'cmr2bin/CMR2.exe', 0x4a0380)
    rebuilt = extract(ROOT / 'build/CMR2.exe', int(entry['recomp'], 16))
    mutation, changed = bytearray(original), 0
    for ins in MD.disasm(original, 0x4a0380):
        if ins.mnemonic == 'add' and ins.op_str == 'ecx, 0x1c':
            assert ins.imm_size == 1
            mutation[ins.address - 0x4a0380 + ins.imm_offset] = 0x18
            changed += 1
    assert changed == 1
    source = '#include <windows.h>\n'
    for name, code in [('original', original), ('rebuilt', rebuilt), ('mutation', mutation)]:
        source += 'static BYTE code_%s[]={%s};\n' % (name, ','.join(map(str, code)))
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=str(TOOLS / 'wineprefix'))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-menu-lookup-') as tmp:
        (Path(tmp) / 'lookup.c').write_text(source + DRIVER)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo',
                        '/O2', '/MD', 'lookup.c', '/Felookup.exe'], cwd=tmp,
                       env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'lookup.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
