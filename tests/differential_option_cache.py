#!/usr/bin/env python3
"""Compare native option cache copies, player swaps, rally penalties and blinking.

Only player-count, option-record and clock providers are mocked. Check complete
buffers, provider call order, four records, the ten swapped fields of four players and guards.
Usage: differential_option_cache.py report.json entities.json
"""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import pefile
from differential_com_outputs import extract
from matching_entities import entity_address

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT.parent / 'tools'
DRIVER = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *VoidFn)(void);
typedef void (__stdcall *SwapFn)(int);
typedef int (__stdcall *BlinkFn)(void);
static BYTE *active,records[4][360],recordExpected[4][360],memoryExpected[16384],initial[16384];
static int caseId,count,trace[128],traceExpected[128],traceCount,traceExpectedCount;
static unsigned int seed;
static unsigned int next(void) {seed=seed*1664525+1013904223;return seed;}
static unsigned int __stdcall players(void) {trace[traceCount++]=100;return count;}
static void *__stdcall options(int i) {if(i<0||i>=4)exit(3);trace[traceCount++]=i;return records[i]+16;}
static unsigned int __stdcall clockValue(void) {trace[traceCount++]=200;return caseId*7919u;}
static void *mocks[]={players,options,clockValue};
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);int i,disp;
    if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){disp=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);}
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i,j;seed=caseId+123;count=caseId%5;memset(active,0xa5,16384);
    for(i=0;i<0x550;i++)active[256+i]=(BYTE)next();
    for(i=0;i<4;i++)for(j=0;j<360;j++)records[i][j]=j<16||j>=344?0xa5:(BYTE)next();
    for(i=0;i<0x80;i++)active[2048+i]=(BYTE)next();
    *(double *)(active+4096)=-65536.0;
    for(i=0;i<0x140;i++)active[5000+i]=(BYTE)next();
    *(DWORD *)(active+6400)=next();*(DWORD *)(active+6404)=next();
    active[6408]=(BYTE)(caseId%5-1);
    *(DWORD *)(active+6420)=caseId%3;*(DWORD *)(active+6424)=next();
    memcpy(initial,active,16384);traceCount=0;
}
static void verify(int stage,int target,int answer) {
    int i,j,current=(signed char)initial[6408];
    static int fields[]={0x4c,0x1c,0x18,0x28,0x14,0x20,0x24,0x44,0x40,0x3c};
    BYTE model[16384];memcpy(model,initial,16384);
    if(stage==0) {
        for(i=0;i<count;i++){memcpy(model+256+48+i*328,records[i]+16,328);memset(model+256+i*12,0,12);}
    } else if(stage==1) {
        for(i=0;i<count;i++)if(memcmp(records[i]+16,initial+256+48+i*328,328)){printf("restore case=%d player=%d\n",caseId,i);exit(3);}
    } else if(stage==2 && current!=target) {
        if(current!=-1 && target!=-1)for(j=0;j<10;j++) {
            DWORD *a=(DWORD *)(model+5000+current*80+fields[j]),*b=(DWORD *)(model+5000+target*80+fields[j]),v=*a;*a=*b;*b=v;
        }
        *(DWORD *)(model+6400)=caseId*7919u;*(DWORD *)(model+6404)=0;model[6408]=(BYTE)target;
    } else if(stage==3) {
        for(i=0;i<16;i++)*(DWORD *)(model+2048+32+i*4)-=(int)(signed char)initial[2048+96+i]*65536;
    } else if(stage==4) {
        unsigned int expected=0;
        if(*(DWORD *)(initial+6420))expected=(~((caseId*7919u-*(DWORD *)(initial+6424))/10))&1;
        if(answer!=(int)expected){printf("blink case=%d got=%d expected=%u\n",caseId,answer,expected);exit(3);}
    }
    if(memcmp(active,model,16384)){for(i=0;i<16384;i++)if(active[i]!=model[i]){printf("model mismatch stage=%d case=%d byte=%d got=%u wanted=%u\n",stage,caseId,i,active[i],model[i]);break;}exit(3);}
    for(i=0;i<4;i++)for(j=0;j<360;j++)if((j<16||j>=344)&&records[i][j]!=0xa5)exit(3);
}
int main(void) {
    void *fns[5][2];BYTE *regions[2];int image,stage,differences=0,target,answer,expectedAnswer=0;
    for(image=0;image<2;image++){
        regions[image]=(BYTE *)VirtualAlloc((void *)(0x27000000+image*0x10000),16384,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
        if(regions[image]!=(BYTE *)(0x27000000+image*0x10000))exit(2);
    }
'''
DRIVER_TAIL = r'''
    for(stage=0;stage<5;stage++)for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        active=regions[image];prepare();target=(caseId/5)%5-1;answer=0;
        if(stage==2)((SwapFn)fns[stage][image])(target);
        else if(stage==4)answer=((BlinkFn)fns[stage][image])();
        else ((VoidFn)fns[stage][image])();
        verify(stage,target,answer);
        if(image==0){expectedAnswer=answer;traceExpectedCount=traceCount;memcpy(traceExpected,trace,traceCount*sizeof(int));memcpy(recordExpected,records,sizeof(records));memcpy(memoryExpected,active,16384);}
        else if(answer!=expectedAnswer||traceCount!=traceExpectedCount||memcmp(trace,traceExpected,traceCount*sizeof(int))||memcmp(records,recordExpected,sizeof(records))||memcmp(active,memoryExpected,16384))differences++;
    }
    printf("30000 native cases: %d differences; four option records, ten-field swaps for four players, signed penalties, blink parity, provider order and guards checked\n",differences);
    return differences!=0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    addresses = {int(e['address'], 16): int(e['recomp'], 16)
                 for e in report['data'] if e.get('recomp')}
    source, loads = '#include <windows.h>\n', ''
    # The menus configure four players. The legacy 16-record declaration
    # overlaps unrelated state beyond that domain; preserve separate controls
    # and test every valid old/new player pair, including the -1 sentinel.
    regions = [(0x82c040, 0x550, 256), (0x533538, 0x80, 2048), (0x5112e8, 8, 4096),
               (0x82c6c8, 0x140, 5000), (0x82c6c0, 4, 6400), (0x82cb44, 4, 6404),
               (0x82ca1c, 1, 6408), (0x82ac58, 4, 6420), (0x82ac5c, 4, 6424)]
    for image in range(2):
        pe = pefile.PE(str(ROOT / ('build/CMR2.exe' if image else 'cmr2bin/CMR2.exe')))
        def entity(a):
            return entity_address(entities, a) if image else a
        globals_ = [(entity(a), size, 0x27000000 + image * 0x10000 + offset)
                    for a, size, offset in regions]
        callees = {entity(a): i for i, a in enumerate([0x405d70, 0x407610, 0x4a9b80])}
        for stage, (address, cleanup) in enumerate([(0x502d50, 0), (0x502db0, 0),
                                                    (0x505a60, 4), (0x40d010, 0), (0x5004c0, 0)]):
            code, calls = extract(pe, addresses[address] if image else address, globals_, callees, cleanup)
            name = 's%d_i%d' % (stage, image)
            source += 'static BYTE code_%s[]={%s};\n' % (name, ','.join(map(str, code)))
            source += 'static int calls_%s[][2]={%s};\n' % (name, ','.join('{%d,%d}' % c for c in calls) or '{0,0}')
            loads += '    fns[%d][%d]=load(code_%s,sizeof(code_%s),calls_%s,%d);\n' % (stage, image, name, name, name, len(calls))
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=str(TOOLS / 'wineprefix'))
    def win(path):
        return subprocess.check_output(['winepath', '-w', str(path)], env=env, text=True).strip()
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-option-cache-') as tmp:
        (Path(tmp) / 'options.c').write_text(source + DRIVER + loads + DRIVER_TAIL)
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo', '/O2', '/MD',
                        'options.c', '/Feoptions.exe'], cwd=tmp, env=env, check=True,
                       stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'options.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
