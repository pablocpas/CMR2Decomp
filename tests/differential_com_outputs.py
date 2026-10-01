#!/usr/bin/env python3
"""Run native DirectPlay creation, sound shutdown and vertex buffer release.

COM methods and two providers are mocked; compare arguments/order, output
identifiers, slot resets, all 100 fill counts and surrounding memory.
Usage: differential_com_outputs.py report.json entities.json
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


def extract(pe, address, globals_, callees, cleanup):
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
            if ins.operands[0].type == capstone.x86.X86_OP_IMM:
                calls.append((pos - address + ins.imm_offset, callees[ins.operands[0].imm]))
            else:
                assert ins.operands[0].type == capstone.x86.X86_OP_MEM
                assert ins.operands[0].mem.base != 0
        if ins.group(capstone.CS_GRP_RET):
            assert (int(ins.op_str, 0) if ins.op_str else 0) == cleanup
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
            replacement = next((target + value - start for start, length, target in globals_
                                if start <= value < start + length), None)
            if replacement is not None:
                assert size == 4
                struct.pack_into('<I', code, pos - address + offset, replacement)
            elif op.type == capstone.x86.X86_OP_MEM:
                assert not 0x400000 <= value < 0x900000, hex(value)
    return code, sorted(calls)


DRIVER = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int (__stdcall *PlayerFn)(char *,char *,void *,int);
typedef void (__stdcall *SoundFn)(void *);
typedef void (__stdcall *VertexFn)(void);
struct Object { void **table; int id; };
struct Name { DWORD size,flags; char *shortName,*longName; };
struct Slot { BYTE prefix[0x14];int is3d,unknown;struct Object *buffer,*spatial,*loop,*other;int owned,looping; };
static struct Object objects[110],directPlay;
static void *bufferTable[20],*playerTable[7];
static BYTE manager[0x348],expectedManager[0x348];
static struct Slot slot,expectedSlot;
static BYTE *active;
static int caseId,trace[256][8],expectedTrace[256][8],traceCount,expectedCount;
static int *record(int kind,int id) {int *p;if(traceCount>=256)exit(3);p=trace[traceCount++];memset(p,0,8*sizeof(int));p[0]=kind;p[1]=id;return p;}
static ULONG __stdcall release(struct Object *p) {record(1,p->id);return (caseId+p->id)%3==0?0:1;}
static HRESULT __stdcall stop(struct Object *p) {record(2,p->id);return (caseId+p->id)%2?0:E_FAIL;}
static HRESULT __stdcall status(struct Object *p,DWORD *out) {record(3,p->id);*out=caseId%4;return 0;}
static HRESULT __stdcall position(struct Object *p,DWORD at) {record(4,p->id)[2]=at;return caseId%2?E_FAIL:0;}
static int __stdcall result(HRESULT hr) {record(5,0)[2]=hr;return hr==0;}
static struct Object *__stdcall getDirectPlay(void) {record(6,0);return caseId%7?&directPlay:NULL;}
static HRESULT __stdcall createPlayer(struct Object *self,DWORD *id,struct Name *name,void *event,void *data,DWORD size,DWORD flags) {
    int *r=record(7,0);
    if(self!=&directPlay || id!=(DWORD *)(active+80) || name!=(struct Name *)(active+96))exit(3);
    r[2]=name->size;r[3]=name->flags;r[4]=(int)name->shortName;r[5]=(int)name->longName;r[6]=(int)data;r[7]=size;
    if(event || flags)exit(3);*id=0x12340000+caseId;
    switch(caseId%5) {case 0:return 0;case 1:return E_FAIL;case 2:return 0x88770078;case 3:return 0x887700aa;default:return 1;}
}
static void *mocks[]={getDirectPlay,result};
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);int i,disp;
    if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){disp=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);}
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i;memset(active,0xa5,4096);memset(manager,0xa5,sizeof(manager));memset(&slot,0xa5,sizeof(slot));
    *(BYTE **)(active+64)=manager;*(int *)(active+68)=caseId;
    *(DWORD *)(active+80)=0x99990000+caseId;*(BYTE *)(active+84)=caseId%2;
    for(i=0;i<103;i++) {
        struct Object *p=(caseId+i)%4?&objects[i]:NULL;
        if(i<100){*(struct Object **)(manager+0x18+i*4)=p;*(int *)(manager+0x1a8+i*4)=i*12345+caseId+1;}
        else *(struct Object **)(manager+0x338+(i-100)*4)=p;
    }
    slot.is3d=(caseId>>2)&1;slot.owned=(caseId>>1)&1;slot.looping=caseId&1;
    slot.buffer=caseId%9?&objects[103]:NULL;slot.spatial=caseId%3?&objects[104]:NULL;
    slot.loop=&objects[105];slot.other=caseId%5?&objects[106]:NULL;
}
int main(void) {
    PlayerFn player[2];SoundFn sound[2];VertexFn vertex[2];BYTE *regions[2],expectedGlobals[4096];
    int f,k,stage,differences=0,answer,expectedAnswer=0;
    for(k=0;k<110;k++){objects[k].table=bufferTable;objects[k].id=k;}
    bufferTable[2]=release;bufferTable[9]=status;bufferTable[13]=position;bufferTable[18]=stop;
    directPlay.table=playerTable;playerTable[6]=createPlayer;
    if(sizeof(struct Name)!=16 || sizeof(struct Slot)!=0x34)exit(2);
    for(f=0;f<2;f++){
        regions[f]=(BYTE *)VirtualAlloc((void *)(0x26000000+f*0x10000),4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
        if(regions[f]!=(BYTE *)(0x26000000+f*0x10000))exit(2);
    }
    player[0]=(PlayerFn)load(code_p_original,sizeof(code_p_original),calls_p_original,sizeof(calls_p_original)/sizeof(calls_p_original[0]));
    player[1]=(PlayerFn)load(code_p_rebuilt,sizeof(code_p_rebuilt),calls_p_rebuilt,sizeof(calls_p_rebuilt)/sizeof(calls_p_rebuilt[0]));
    sound[0]=(SoundFn)load(code_s_original,sizeof(code_s_original),calls_s_original,sizeof(calls_s_original)/sizeof(calls_s_original[0]));
    sound[1]=(SoundFn)load(code_s_rebuilt,sizeof(code_s_rebuilt),calls_s_rebuilt,sizeof(calls_s_rebuilt)/sizeof(calls_s_rebuilt[0]));
    vertex[0]=(VertexFn)load(code_v_original,sizeof(code_v_original),calls_v_original,0);
    vertex[1]=(VertexFn)load(code_v_rebuilt,sizeof(code_v_rebuilt),calls_v_rebuilt,0);
    for(stage=0;stage<3;stage++)for(caseId=0;caseId<6000;caseId++)for(f=0;f<2;f++){
        active=regions[f];prepare();traceCount=0;answer=0;
        if(stage==0)answer=player[f]("short","long",manager,caseId*17);
        else if(stage==1)sound[f](&slot);
        else vertex[f]();
        for(k=0;k<4096;k++)if((k<64 || (k>=72 && k<80) || (k>=85 && k<96) || k>=112) && active[k]!=0xa5)exit(3);
        if(stage==2)for(k=0;k<100;k++)if(*(int *)(manager+0x1a8+k*4)!=0)exit(3);
        if(f==0){expectedAnswer=answer;expectedCount=traceCount;memcpy(expectedTrace,trace,sizeof(trace));memcpy(expectedManager,manager,sizeof(manager));expectedSlot=slot;memcpy(expectedGlobals,active,4096);}
        else if(answer!=expectedAnswer || traceCount!=expectedCount || memcmp(trace,expectedTrace,traceCount*8*sizeof(int)) || memcmp(manager,expectedManager,sizeof(manager)) || memcmp(&slot,&expectedSlot,sizeof(slot)) || memcmp(active,expectedGlobals,4096))differences++;
    }
    printf("18000 native COM cases: %d differences; player output/name, sound HRESULT order, 100 fill counts and guards checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    addresses = {int(e['address'], 16): int(e['recomp'], 16)
                 for e in report['data'] if e.get('recomp')}
    source = '#include <windows.h>\n'
    for image, name in enumerate(['original', 'rebuilt']):
        rebuilt = image == 1
        pe = pefile.PE(str(ROOT / ('build/CMR2.exe' if rebuilt else 'cmr2bin/CMR2.exe')))
        def entity(a):
            return entities[hex(a)][0] if rebuilt else a
        globals_ = [(entity(a), length, 0x26000000 + image * 0x10000 + offset)
                    for a, length, offset in [(0x520b78, 4, 64), (0x6dd890, 4, 68),
                                              (0x5a1ea0, 4, 80), (0x5a1fc0, 1, 84),
                                              (0x5a1fa8, 16, 96)]]
        callees = {entity(0x4aad40): 0, entity(0x4a3250): 1}
        for tag, a, cleanup in [('p', 0x4a1a10, 16), ('s', 0x4a26f0, 4), ('v', 0x4b1de0, 0)]:
            code, calls = extract(pe, addresses[a] if rebuilt else a, globals_, callees, cleanup)
            source += 'static BYTE code_%s_%s[]={%s};\n' % (tag, name, ','.join(map(str, code)))
            source += 'static int calls_%s_%s[][2]={%s};\n' % (tag, name, ','.join('{%d,%d}' % c for c in calls) or '{0,0}')
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=os.environ.get('WINEPREFIX', str(TOOLS / 'wineprefix')))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-com-outputs-') as tmp:
        (Path(tmp) / 'com.c').write_text(source + DRIVER)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo', '/O2', '/MD',
                        'com.c', '/Fecom.exe'], cwd=tmp, env=env, check=True)
        subprocess.run(['wine', 'com.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
