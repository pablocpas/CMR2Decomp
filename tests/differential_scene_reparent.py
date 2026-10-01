#!/usr/bin/env python3
"""Compare native scene reparenting, ancestor flags, failures and memory guards."""
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
TOOLS = Path(os.environ.get('CMR2_TOOLS', ROOT.parent / 'tools'))
DRIVER = r'''
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int (__stdcall *Fn)(void *,void *);
static BYTE arena[32768],initial[32768],original[32768],allowed[32768];
static int caseId,position,children,depth;
static BYTE *node(int id) {return arena+4096+id*512;}
static void pointer(int id,int at,int target) {
    *(BYTE **)(node(id)+at)=target<0?NULL:node(target);
}
static void permit(int id,int at) {
    memset(allowed+4096+id*512+at,1,4);
}
static void *load(const BYTE *bytes,int size) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    if(!p)exit(2);memcpy(p,bytes,size);FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    unsigned int rng=caseId+791;int i,mode=caseId%10,count=1+(caseId/10)%8,id,next;
    for(i=0;i<sizeof(arena);i++) {
        rng=rng*1664525u+1013904223u;arena[i]=(BYTE)(rng>>24);
    }
    for(i=0;i<32;i++)memset(node(i),0,12);
    pointer(1,8,0);pointer(2,8,1);
    depth=1+(caseId/80)%6;
    pointer(3,8,depth==1?0:4);
    for(i=4;i<3+depth;i++)pointer(i,8,i==2+depth?0:i+1);
    position=(caseId/480)%count;
    if(mode==3)position=-1;
    pointer(1,4,position==0?2:10);
    for(i=0;i<count;i++) {
        id=i==position?2:10+i;
        next=i+1==count?-1:(i+1==position?2:11+i);
        pointer(id,0,next);pointer(id,8,1);
    }
    children=(caseId/3840)%3;
    pointer(3,4,children?22:-1);
    for(i=0;i<children;i++){pointer(22+i,0,i+1==children?-1:23+i);pointer(22+i,8,3);}
    if(mode==2)pointer(3,8,2);
    memcpy(initial,arena,sizeof(arena));memset(allowed,0,sizeof(allowed));
    if(mode>=4) {
        permit(position?9+position:1,position?0:4);
        permit(2,0);permit(2,8);permit(3,4);permit(2,0x174);
        permit(3,0x174);permit(0,0x174);
        for(i=4;i<3+depth;i++)permit(i,0x174);
    }
}
int main(void) {
    Fn fn[2];int i,image,mode,expected,result,oldResult,dirty;
    BYTE *newParent,*expectedNext;
LOADS
    for(caseId=0;caseId<12000;caseId++) {
        prepare();mode=caseId%10;expected=mode>=4;
        newParent=mode==1?node(1):node(3);
        expectedNext=*(BYTE **)(newParent+4);
        for(image=0;image<2;image++) {
            memcpy(arena,initial,sizeof(arena));
            result=fn[image](mode==0?NULL:node(2),newParent);
            if(result!=expected){printf("return case=%d image=%d result=%d/%d\n",caseId,image,result,expected);return 1;}
            for(i=0;i<sizeof(arena);i++)if(!allowed[i]&&arena[i]!=initial[i]) {
                printf("memory guard case=%d image=%d at=%d\n",caseId,image,i);return 2;
            }
            if(expected) {
                if(*(BYTE **)(node(2)+8)!=newParent||*(BYTE **)(newParent+4)!=node(2)||*(BYTE **)node(2)!=expectedNext)return 3;
                dirty=2;
                do {
                    if(*(int *)(node(dirty)+0x174)!=1)return 4;
                    if(dirty==0)break;
                    dirty=(int)(*(BYTE **)(node(dirty)+8)-node(0))/512;
                } while(1);
            }
            if(!image){memcpy(original,arena,sizeof(arena));oldResult=result;}
            else if(result!=oldResult||memcmp(original,arena,sizeof(arena))) {
                printf("reparent mismatch case=%d\n",caseId);return 5;
            }
        }
    }
    printf("12000 native scene reparent cases: 0 differences; first/middle/last siblings, ancestor chains, failures, return values and all guards checked\n");return 0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    fn = next(f for f in report['data'] if int(f['address'], 16) == 0x4ac7a0)
    assert int(fn['recomp'], 16) == entity_address(entities, 0x4ac7a0)
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / 'build/CMR2.exe'
    source, loads = '', ''
    for image, path in enumerate([ROOT / 'cmr2bin/CMR2.exe', rebuilt]):
        pe = pefile.PE(str(path))
        code, calls = extract(pe, int(fn['recomp'], 16) if image else 0x4ac7a0, [], {}, 8)
        assert not calls
        source += 'static unsigned char code%d[]={%s};\n' % (image, ','.join(map(str, code)))
        loads += '    fn[%d]=(Fn)load(code%d,sizeof(code%d));\n' % (image, image, image)
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=os.environ.get('WINEPREFIX', str(TOOLS / 'wineprefix')))
    win = lambda p: 'Z:' + str(p).replace('/', '\\')
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-scene-reparent-') as tmp:
        (Path(tmp) / 'reparent.c').write_text(source + DRIVER.replace('LOADS', loads))
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo', '/O2', '/MD',
                        'reparent.c', '/Fereparent.exe'], cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'reparent.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
