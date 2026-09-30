#!/usr/bin/env python3
"""Execute original/rebuilt text grouping and DirectPlay Receive with memory models.

Mocks validate COM argument positions, output IDs, buffer growth, allocation
failure and call order. Checks the entire arena and untouched guards.
Usage: report.json entities.json.
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
typedef void (__stdcall *TextFn)(char *);
typedef int (__stdcall *ReceiveFn)(DWORD *,void **);
struct Object {void **table;};
static struct Object directPlay;
static void *table[32];
static BYTE *active,initial[65536],expected[65536],original[65536];
static int caseId,stage,trace[16][8],originalTrace[16][8],traceCount,originalCount;
static unsigned int seed;
static DWORD statuses[]={0,0x8877001e,0x88770082,0x80004005,0x80070057,0x88770096,0x887700be,0x88770078,0x88770001,1};
static DWORD status(void) {return statuses[caseId%10];}
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *why,int offset) {printf("%s stage=%d case=%d offset=%d\n",why,stage,caseId,offset);exit(3);}
static int *record(int kind) {int *p;if(traceCount>=16)fail("trace capacity",0);p=trace[traceCount++];memset(p,0,32);p[0]=kind;return p;}
static struct Object *__stdcall getDirectPlay(void) {record(1);return caseId%7?&directPlay:NULL;}
static HRESULT __stdcall receive(struct Object *self,DWORD *sender,DWORD *receiver,DWORD flags,void *buffer,DWORD *size) {
    int *p=record(2);int i;
    if(self!=&directPlay||sender!=(DWORD *)(active+48)||flags!=1||receiver==sender||size==sender||receiver==size)fail("Receive arguments",0);
    if(buffer!=*(void **)(active+32))fail("Receive buffer",0);
    p[1]=flags;p[2]=buffer?(BYTE *)buffer-active:0;p[3]=*size;
    *sender=0x81234500u+caseId;*receiver=0x90123400u+caseId;
    if(!status()&&buffer){for(i=0;i<8;i++)((BYTE *)buffer)[i]=(BYTE)(caseId+i);}
    *size=128+(caseId%256);return status();
}
static void __stdcall freeBuffer(void *p) {record(3)[1]=(BYTE *)p-active;}
static void *__stdcall allocate(int bytes) {record(4)[1]=bytes;return caseId%3?active+7000:NULL;}
static void *mocks[]={getDirectPlay,freeBuffer,allocate};
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);int i,disp;
    if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){disp=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);}
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i,len;seed=caseId+979;for(i=0;i<65536;i++)active[i]=(BYTE)(next()>>24);
    *(void **)(active+32)=caseId%4?active+6000:NULL;
    *(DWORD *)(active+36)=caseId%1025;
    len=caseId%129;
    for(i=0;i<len;i++)active[4096+i]=(BYTE)(1+next()%255);
    active[4096+len]=0;
    memcpy(initial,active,65536);memcpy(expected,active,65536);traceCount=0;
}
static void model(int result) {
    int i,len,out,n=0;DWORD word;BYTE *buffer;
    if(stage==0){
        len=(int)strlen((char *)initial+4096);memcpy(expected+256,initial+4096,len+1);
        out=0;for(i=0;i<len;i++){expected[4096+out++]=initial[4096+i];if((i+1)%4==0)expected[4096+out++]=' ';}
        expected[4096+out]=0;if(traceCount)fail("text provider calls",0);
    } else {
        if(traceCount<1||trace[n++][0]!=1)fail("getDirectPlay trace",0);
        if(caseId%7){
            if(traceCount<2||trace[n][0]!=2||trace[n][1]!=1||trace[n][2]!=(caseId%4?6000:0)||trace[n][3]!=caseId%1025)fail("Receive trace",n);
            n++;*(DWORD *)(expected+48)=0x81234500u+caseId;
            if(!status()){
                if(caseId%4){for(i=0;i<8;i++)expected[6000+i]=(BYTE)(caseId+i);}
                memcpy(expected+52,initial+32,4);if(result!=1)fail("Receive success result",result);
            } else {
                if(result)fail("Receive error result",result);
                if(status()==0x8877001e){
                    if(caseId%4){if(traceCount<=n||trace[n][0]!=3||trace[n][1]!=6000)fail("free trace",n);n++;}
                    if(traceCount<=n||trace[n][0]!=4||trace[n][1]!=128+caseId%256)fail("allocation trace",n);
                    n++;buffer=caseId%3?active+7000:NULL;memcpy(expected+32,&buffer,4);
                    if(buffer)*(DWORD *)(expected+36)=128+caseId%256;
                }
            }
        } else if(result)fail("null DirectPlay result",result);
        if(traceCount!=n)fail("extra calls",traceCount);
    }
    if(memcmp(active,expected,65536))for(i=0;i<65536;i++)if(active[i]!=expected[i])fail("independent memory model",i);
}
int main(void) {
    void *fns[2][2];int image,result,differences=0;
    active=(BYTE *)VirtualAlloc((void *)0x28000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(active!=(BYTE *)0x28000000)exit(2);
    directPlay.table=table;table[25]=receive;
'''
TAIL = r'''
    for(stage=0;stage<2;stage++)for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();result=0;
        if(stage==0)((TextFn)fns[stage][image])((char *)active+4096);
        else result=((ReceiveFn)fns[stage][image])((DWORD *)(active+48),(void **)(active+52));
        model(result);
        if(!image){memcpy(original,active,65536);originalCount=traceCount;memcpy(originalTrace,trace,traceCount*32);}
        else if(memcmp(original,active,65536)||traceCount!=originalCount||memcmp(originalTrace,trace,traceCount*32))differences++;
    }
    printf("12000 native cases: %d differences; full text, COM arguments, outputs, allocation failures, error paths and memory guards checked\n",differences);
    return differences!=0;
}
'''

def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    addresses = {int(e['address'],16):int(e['recomp'],16) for e in report['data'] if e.get('recomp')}
    regions = [(0x663b60,260,256),(0x5a1fb8,4,32),(0x5a1fbc,4,36)]
    providers = [0x4aad40,0x4aade0,0x4aad70]
    routines = [(0x4f8a90,4),(0x4a1b90,8)]
    source,loads='#include <windows.h>\n',''
    for image in range(2):
        pe=pefile.PE(str(ROOT/('build/CMR2.exe' if image else 'cmr2bin/CMR2.exe')))
        def entity(a): return entity_address(entities,a) if image else a
        globals_=[(entity(a),size,0x28000000+offset) for a,size,offset in regions]
        callees={entity(a):i for i,a in enumerate(providers)}
        for stage,(a,cleanup) in enumerate(routines):
            code,calls=extract(pe,addresses[a] if image else a,globals_,callees,cleanup)
            name='s%d_i%d'%(stage,image)
            source+='static BYTE code_%s[]={%s};\n'%(name,','.join(map(str,code)))
            source+='static int calls_%s[][2]={%s};\n'%(name,','.join('{%d,%d}'%c for c in calls) or '{0,0}')
            loads+='    fns[%d][%d]=load(code_%s,sizeof(code_%s),calls_%s,%d);\n'%(stage,image,name,name,name,len(calls))
    env=dict(os.environ,WINEDEBUG='-all',WINEPREFIX=str(TOOLS/'wineprefix'))
    def win(path):return subprocess.check_output(['winepath','-w',str(path)],env=env,text=True).strip()
    env['INCLUDE']=win(TOOLS/'msvc600/VC98/Include');env['LIB']=win(TOOLS/'msvc600/VC98/Lib')
    env['WINEPATH']=win(TOOLS/'msvc600/VC98/Bin')+';'+win(TOOLS/'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-low-callbacks-') as tmp:
        (Path(tmp)/'callbacks.c').write_text(source+DRIVER+loads+TAIL)
        subprocess.run(['wine',str(TOOLS/'msvc600/VC98/Bin/CL.EXE'),'/nologo','/O2','/MD','callbacks.c','/Fecallbacks.exe'],cwd=tmp,env=env,check=True,stdout=subprocess.DEVNULL)
        subprocess.run(['wine','callbacks.exe'],cwd=tmp,env=env,check=True)

if __name__=='__main__':main()
