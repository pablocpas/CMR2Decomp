#!/usr/bin/env python3
"""Compare native record reset with the original and an independent table model."""
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
typedef void (__stdcall *Fn)(void *);
static BYTE *arena,initial[16384],expected[16384],original[16384];
static unsigned int rng;
static int caseId,textLength;
static void *load(const BYTE *bytes,int size) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    if(!p)exit(2);memcpy(p,bytes,size);FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void label(int at) {memcpy(expected+4096+at,initial+256,textLength+1);}
static void word(int at,unsigned int keep,unsigned int bits) {
    unsigned int *p=(unsigned int *)(expected+4096+at);*p=(*p&keep)|bits;
}
static void model(void) {
    int i,j,row,at;
    memcpy(expected,initial,sizeof(initial));
    for(i=0;i<15;i++) {
        at=i*12;label(at);word(at+4,0xfffc0000,0x3c000);
        memset(expected+4096+at+8,0,3);
    }
    for(i=0;i<120;i++) {
        at=0xb4+i*12;label(at);word(at+4,0xffff8000,0x780);
        *(int *)(expected+4096+at+8)=360000;
    }
    for(i=0;i<88;i++) {
        at=0x654+i*8;label(at);word(at+4,0xff800000,(i%11==10?24000u:48000u)<<7);
        for(j=0;j<10;j++) {
            row=0x914+i*20+j*2;
            *(short *)(expected+4096+row)=(short)(j*(i%11==10?12000:6000));
        }
    }
    for(i=0;i<45;i++) {
        at=0xff4+i*12;label(at);word(at+4,0xffff0000,0x280);
    }
    for(i=0;i<9;i++) {
        at=0x1210+i*8;label(at);word(at+4,0xff800000,24000u<<7);
        for(j=0;j<6;j++)*(short *)(expected+4096+0x1258+i*12+j*2)=(short)(j*6000);
    }
}
int main(void) {
    Fn fn[2];int i,image;
    arena=(BYTE *)VirtualAlloc((void *)0x2e000000,16384,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(arena!=(BYTE *)0x2e000000)return 2;
LOADS
    for(caseId=0;caseId<6000;caseId++) {
        rng=caseId+1151;
        for(i=0;i<sizeof(initial);i++) {
            rng=rng*1664525u+1013904223u;initial[i]=(BYTE)(rng>>24);
        }
        textLength=caseId%4;
        for(i=0;i<textLength;i++)initial[256+i]|=1;
        initial[256+textLength]=0;model();
        for(image=0;image<2;image++) {
            memcpy(arena,initial,sizeof(initial));fn[image](arena+4096);
            for(i=0;i<sizeof(initial);i++)if(arena[i]!=expected[i]) {
                printf("record model mismatch case=%d image=%d offset=%x got=%02x expected=%02x\n",caseId,image,i,arena[i],expected[i]);return 3;
            }
            if(!image)memcpy(original,arena,sizeof(original));
            else if(memcmp(original,arena,sizeof(original)))return 4;
        }
    }
    printf("6000 native record reset cases: 0 differences; all record tables, bitfields, split overflow, variable labels and memory guards checked\n");return 0;
}
'''


def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    address = entity_address(entities, 0x406010)
    assert address == next(int(f['recomp'], 16) for f in report['data'] if int(f['address'], 16) == 0x406010)
    rebuilt = Path(sys.argv[3]) if len(sys.argv) > 3 else ROOT / 'build/CMR2.exe'
    source, loads = '', ''
    for image, path in enumerate([ROOT / 'cmr2bin/CMR2.exe', rebuilt]):
        pe = pefile.PE(str(path))
        label = entity_address(entities, 0x516184) if image else 0x516184
        code, calls = extract(pe, address if image else 0x406010, [(label, 4, 0x2e000100)], {}, 4)
        assert not calls
        source += 'static unsigned char code%d[]={%s};\n' % (image, ','.join(map(str, code)))
        loads += '    fn[%d]=(Fn)load(code%d,sizeof(code%d));\n' % (image, image, image)
    env = dict(os.environ, WINEDEBUG='-all', WINEPREFIX=os.environ.get('WINEPREFIX', str(TOOLS / 'wineprefix')))
    win = lambda p: 'Z:' + str(p).replace('/', '\\')
    env['INCLUDE'] = win(TOOLS / 'msvc600/VC98/Include')
    env['LIB'] = win(TOOLS / 'msvc600/VC98/Lib')
    env['WINEPATH'] = win(TOOLS / 'msvc600/VC98/Bin') + ';' + win(TOOLS / 'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-record-reset-') as tmp:
        (Path(tmp) / 'records.c').write_text(source + DRIVER.replace('LOADS', loads))
        subprocess.run(['wine', str(TOOLS / 'msvc600/VC98/Bin/CL.EXE'), '/nologo', '/O2', '/MD',
                        'records.c', '/Ferecords.exe'], cwd=tmp, env=env, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(['wine', 'records.exe'], cwd=tmp, env=env, check=True)


if __name__ == '__main__':
    main()
