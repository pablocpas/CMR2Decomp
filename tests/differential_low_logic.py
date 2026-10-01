#!/usr/bin/env python3
"""Execute low matching logic against original code and independent memory models.

Covers scene initialization, saturated progress fields, aliased colour outputs,
gear limits, approach steps, HUD outputs and the split reset prefix. Providers are mocked;
whole memory, guards and call traces are checked. Usage: report.json entities.json.
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
TOOLS = Path(os.environ.get("CMR2_TOOLS", ROOT.parent / "tools"))
DRIVER = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef void (__stdcall *VoidFn)(void);
typedef void (__stdcall *ColourFn)(int,DWORD *,DWORD *,DWORD *);
typedef void (__stdcall *ApproachFn)(BYTE *,int);
typedef void (__stdcall *HudFn)(BYTE *,BYTE *,int);
static BYTE *active,initial[65536],expected[65536],original[65536];
static int caseId,stage,count,trace[256][6],originalTrace[256][6],traceCount,originalCount;
static unsigned int seed;
static unsigned int next(void) {seed=seed*1664525u+1013904223u;return seed;}
static void fail(const char *reason,int where) {printf("%s stage=%d case=%d position=%d\n",reason,stage,caseId,where);exit(3);}
static int *record(int id) {int *p;if(traceCount>=256)fail("trace overflow",0);p=trace[traceCount++];memset(p,0,24);p[0]=id;return p;}
static BYTE __stdcall players(void) {record(1);return (BYTE)count;}
static void __stdcall validate(int i) {record(2)[1]=i;}
static int __stdcall callback(void *fn,void *arg) {int *p=record(3);p[1]=(int)fn;p[2]=(int)arg;return 0;}
static BYTE __stdcall state(void) {record(4);return caseId%3;}
static BYTE __stdcall offset(void) {record(5);return caseId%32;}
static void __stdcall hudInputs(int *vector,short *value,int *other,int which) {
    int *p=record(6);p[1]=which;vector[0]=which*101+caseId;vector[1]=-caseId;vector[2]=which*379;
    *value=(short)(0x8123+caseId);*other=which*7919;
}
static void __stdcall hudVector(BYTE index,int *vector) {int *p=record(7);p[1]=index;p[2]=vector[0];p[3]=vector[1];p[4]=vector[2];}
static void __stdcall hudOther(BYTE index,int value) {int *p=record(8);p[1]=index;p[2]=value;}
static void __stdcall hudValue(BYTE index,short value) {int *p=record(9);p[1]=index;p[2]=value;}
static void __stdcall hudMatrix(BYTE *settings,BYTE *matrix) {
    int *p=record(10);p[1]=settings-active;p[2]=matrix-active;
    if(settings!=active+0xa000||matrix!=active+0xa100)fail("HUD pointer",p[2]);
}
static void __stdcall splitBefore(void) {record(11);}
static void __stdcall splitAfter(void) {record(12);}
static void *mocks[]={players,validate,callback,state,offset,hudInputs,hudVector,hudOther,hudValue,hudMatrix,splitBefore,splitAfter};
static void *load(const BYTE *bytes,int size,const int (*calls)[2],int n) {
    BYTE *p=(BYTE *)VirtualAlloc(0,size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);int i,disp;
    if(!p)exit(2);memcpy(p,bytes,size);
    for(i=0;i<n;i++){disp=(BYTE *)mocks[calls[i][1]]-(p+calls[i][0]+4);memcpy(p+calls[i][0],&disp,4);}
    FlushInstructionCache(GetCurrentProcess(),p,size);return p;
}
static void prepare(void) {
    int i;seed=caseId+123;for(i=0;i<65536;i++)active[i]=(BYTE)(next()>>24);
    count=caseId%9;
    for(i=0;i<8;i++)*(DWORD *)(active+256+0x1f70+i*0x30)=((caseId+i)%4)<<18;
    if(stage==2&&caseId%5==0)*(DWORD *)(active+256+0x1f70+(caseId%8)*0x30)=15<<18;
    *(BYTE **)(active+0x8500)=active+0x9000;
    active[0x9000+0x1d0]=caseId%4==0||caseId%4==3;
    active[0x9000+0x1d1]=caseId%4==1||caseId%4==3;
    {static int edge[]={0,-1,1,-65536,65536,-65537,65537};
     *(int *)(active+0x9000+0x818)=caseId%8<7?edge[caseId%8]:(int)(next()%600001)-300000;}
    for(i=0;i<4;i++){*(int *)(active+0x8200+i*4)=(int)(next()%200001)-100000;*(int *)(active+0x8300+i*4)=(int)(next()%200001)-100000;}
    if(stage==4&&caseId%7<6){int car=caseId%4,step=caseId%65537,delta;
        switch(caseId%7){case 0:delta=0;break;case 1:delta=step;break;case 2:delta=-step;break;case 3:delta=step+1;break;case 4:delta=-step-1;break;default:delta=step?step-1:0;}
        *(int *)(active+0x8300+car*4)=*(int *)(active+0x8200+car*4)+delta;}
    active[0xa000]=caseId%8;active[0xa001]=caseId%2;active[0xa002]=caseId%4;active[0xa003]=0x55;
    memcpy(initial,active,65536);memcpy(expected,active,65536);traceCount=0;
}
static void zero(int offset,int bytes) {memset(expected+0x6000+offset,0,bytes);}
static void put(int offset,int value) {*(int *)(expected+0x6000+offset)=value;}
static DWORD *output(int channel,int category) {
    int mode=(caseId/(channel==0?1:channel==1?3:9))%4;
    if(!mode)return 0;
    if(mode==1)return (DWORD *)(active+256+0x63c+(category==15?0:category)*0x650);
    return (DWORD *)(active+0xb000+(mode==2?channel*4:16));
}
static void model(void) {
    int i,j,v,which,category,car,step,d,current,target;DWORD word,*p,colour;
    if(stage==0){
        put(0x408,0);zero(0x1dc,0x20);zero(0xa0,0x20);zero(0x1fc,0x20);put(0x40c,0);
        memset(expected+0x6410,0xff,0x1c);memset(expected+0x6000,0xff,0x30);
        put(0xc0,1);put(0x400,0);put(0xc4,1);put(0x110,1);put(0x114,1);put(0x404,0);
        zero(0x30,2);expected[0x61d8]=3;expected[0x61d9]=3;
        for(i=0;i<2;i++){
            int t=0xcc+i*0x24,s=0x23c+i*0xc,x=0x224+i*8,p=0x298+i*0x1c;
            zero(t-2,6);zero(p-4,16);zero(s+4,4);put(s,1);put(t+0x14,0);zero(x-4,8);
            put(t+0x18,0x3333);put(s-4,0);put(t+0x1c,0);
        }
        zero(0x118,0xc0);
        if(traceCount!=1||trace[0][0]!=3||trace[0][1]!=0x4779e0||trace[0][2])fail("callback trace",0);
    } else if(stage==1){
        for(i=0;i<count;i++){
            category=(*(DWORD *)(expected+256+0x1f70+i*0x30)>>18)&15;
            p=(DWORD *)(expected+256+0x67c+category*0x650);word=*p;
            if(((word>>7)&255)<255)*p=(word&~0x7f80u)|((((word>>7)&255)+1)<<7);
        }
        if(traceCount!=count+1)fail("count calls",traceCount);
        for(i=0;i<traceCount;i++)if(trace[i][0]!=1)fail("count trace",i);
    } else if(stage==2){
        category=(*(DWORD *)(expected+256+0x1f70+(caseId%8)*0x30)>>18)&15;
        for(i=0;i<3;i++){
            p=output(i,category);if(!p)continue;
            if(category==15)v=0x45;
            else {colour=*(DWORD *)(expected+256+0x63c+category*0x650);v=i==0?(colour>>16)&31:i==1?(colour>>8)&15:colour&255;}
            *(DWORD *)(expected+((BYTE *)p-active))=v;
        }
        if(traceCount!=1||trace[0][0]!=2||trace[0][1]!=caseId%8)fail("validation trace",0);
    } else if(stage==3){
        current=*(int *)(expected+0x9818);
        if(expected[0x91d0]){if(current<0)current=0;current+=65536;if(current>65536)current=65536;}
        else if(expected[0x91d1]){if(current>0)current=0;current-=65536;if(current< -65536)current=-65536;}
        else current=0;
        *(int *)(expected+0x9818)=current;
    } else if(stage==4){
        car=caseId%4;step=caseId%65537;current=*(int *)(expected+0x8200+car*4);target=*(int *)(expected+0x8300+car*4);d=target-current;
        if((d<0?-d:d)<step)current=target;else current+=d>0?step:-step;
        *(int *)(expected+0x8200+car*4)=current;
    } else if(stage==5) {
        expected[0x8100+expected[0xa000]]=(BYTE)(caseId*7919u);
        i=1;if(trace[0][0]!=4)fail("HUD state",0);
        which=0;if(expected[0xa002]<caseId%3){if(trace[i++][0]!=5)fail("HUD offset",i);which=caseId%32+expected[0xa002];}
        j=i;
        if(traceCount!=j+5||trace[j][0]!=6||trace[j][1]!=which)fail("HUD inputs",j);
        if(trace[j+1][0]!=7||trace[j+1][1]!=expected[0xa001]||trace[j+1][2]!=which*101+caseId||trace[j+1][3]!=-caseId||trace[j+1][4]!=which*379)fail("HUD vector",j+1);
        if(trace[j+2][0]!=8||trace[j+2][1]!=expected[0xa001]||trace[j+2][2]!=which*7919)fail("HUD other",j+2);
        if(trace[j+3][0]!=9||trace[j+3][1]!=expected[0xa001]||trace[j+3][2]!=(short)(0x8123+caseId))fail("HUD short output",j+3);
        if(trace[j+4][0]!=10||trace[j+4][1]!=0xa000||trace[j+4][2]!=0xa100)fail("HUD matrix",j+4);
    } else {
        for(i=1;i<13;i++)*(int *)(expected+0xc098+i*4)=0;
        memset(expected+0xc200,0,8);memset(expected+0xc220,0,8);
        memset(expected+0xc240,0,8);memset(expected+0xc260,0,8);
        memset(expected+0xc280,0xff,24);
        for(i=0;i<4;i++)*(int *)(expected+0xc2a0+i*16)=0;
        memset(expected+0xc2e0,0,2);
        for(i=0;i<2;i++){
            int *p=(int *)(expected+0xc000+i*72);
            p[0]=p[1]=p[2]=0;p[3]=-1;
            for(j=5;j<18;j++)p[j]=0;
        }
        memset(expected+0xc300,0,64);memset(expected+0xc360,0,12);
        if(traceCount!=2||trace[0][0]!=11||trace[1][0]!=12)fail("split reset trace",0);
    }
    if(memcmp(active,expected,65536)){for(i=0;i<65536;i++)if(active[i]!=expected[i]){printf("image memory initial=%u expected=%u actual=%u current=%d target=%d\n",initial[i],expected[i],active[i],*(int *)(initial+0x8200+(caseId%4)*4),*(int *)(initial+0x8300+(caseId%4)*4));fail("independent memory model",i);}}
}
int main(void) {
    void *fns[7][2];int image,differences=0,category;
    active=(BYTE *)VirtualAlloc((void *)0x27000000,65536,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(active!=(BYTE *)0x27000000)exit(2);
'''
TAIL = r'''
    for(stage=0;stage<7;stage++)for(caseId=0;caseId<6000;caseId++)for(image=0;image<2;image++){
        prepare();
        if(stage==2){category=(*(DWORD *)(active+256+0x1f70+(caseId%8)*0x30)>>18)&15;((ColourFn)fns[stage][image])(caseId%8,output(0,category),output(1,category),output(2,category));}
        else if(stage==4){BYTE car=(BYTE)(caseId%4);((ApproachFn)fns[stage][image])(&car,caseId%65537);}
        else if(stage==5)((HudFn)fns[stage][image])(active+0xa000,active+0xa100,caseId*7919u);
        else ((VoidFn)fns[stage][image])();
        model();
        if(!image){memcpy(original,active,65536);originalCount=traceCount;memcpy(originalTrace,trace,traceCount*24);}
        else if(memcmp(original,active,65536)||traceCount!=originalCount||memcmp(originalTrace,trace,traceCount*24))differences++;
    }
    printf("42000 native cases: %d differences; complete memory, scene writes, progress saturation, aliased outputs, gear/step boundaries HUD call traces and owned split prefix checked\n",differences);
    return differences!=0;
}
'''

def main():
    report = json.loads(Path(sys.argv[1]).read_text())
    entities = json.loads(Path(sys.argv[2]).read_text())
    addresses = {int(e['address'], 16): int(e['recomp'], 16) for e in report['data'] if e.get('recomp')}
    # Include the split-pair cursor bound four bytes past the array; the
    # memory model still permits writes only to the actual 64-byte table.
    regions = [(0x52f3e0,0x620,256),(0x52fa00,8,256+0x620),
               (0x52fa08,0x1940,256+0x628),(0x531348,8,256+0x1f68),
               (0x531350,0x300,256+0x1f70),(0x58d2a0,0x430,0x6000),(0x53cff8,8,0x8100),
               (0x591690,0x10,0x8200),(0x591710,0x10,0x8300),(0x59226c,4,0x8500),
               (0x4779e0,1,0x4779e0-0x27000000),
               (0x536df8,0x98,0xc000),(0x536e90,52,0xc098),
               (0x536c0c,8,0xc200),(0x537054,8,0xc220),(0x536c18,8,0xc240),
               (0x536fe4,8,0xc260),(0x536c94,24,0xc280),(0x536bfc,4,0xc2a0),
               (0x537050,4,0xc2b0),(0x536c40,4,0xc2c0),(0x536c3c,4,0xc2d0),
               (0x536c20,4,0xc2e0),(0x536c48,69,0xc300),(0x536c30,12,0xc360)]
    providers = [0x405d70,0x4083f0,0x49c0a0,0x4074f0,0x41b370,0x408c20,0x447d20,0x447ec0,0x447e20,0x447a40,0x411550,0x411b20]
    routines = [(0x475f80,0),(0x4ec1a0,0),(0x408b10,16),(0x494540,0),(0x48dc30,8),(0x447530,12),(0x4111a0,0)]
    source,loads = '#include <windows.h>\n',''
    for image in range(2):
        pe = pefile.PE(str(ROOT/('build/CMR2.exe' if image else 'cmr2bin/CMR2.exe')))
        def entity(a): return entity_address(entities,a,0x536d14 if 0x536df8 <= a < 0x536ec4 else None) if image else a
        globals_ = [(entity(a),size,0x27000000+offset) for a,size,offset in regions]
        callees = {entity(a):i for i,a in enumerate(providers)}
        for stage,(a,cleanup) in enumerate(routines):
            code,calls = extract(pe,addresses[a] if image else a,globals_,callees,cleanup)
            name='s%d_i%d'%(stage,image)
            source+='static BYTE code_%s[]={%s};\n'%(name,','.join(map(str,code)))
            source+='static int calls_%s[][2]={%s};\n'%(name,','.join('{%d,%d}'%c for c in calls) or '{0,0}')
            loads+='    fns[%d][%d]=load(code_%s,sizeof(code_%s),calls_%s,%d);\n'%(stage,image,name,name,name,len(calls))
    env=dict(os.environ,WINEDEBUG='-all',WINEPREFIX=os.environ.get('WINEPREFIX', str(TOOLS/'wineprefix')))
    def win(path): return subprocess.check_output(['winepath','-w',str(path)],env=env,text=True).strip()
    env['INCLUDE']=win(TOOLS/'msvc600/VC98/Include');env['LIB']=win(TOOLS/'msvc600/VC98/Lib')
    env['WINEPATH']=win(TOOLS/'msvc600/VC98/Bin')+';'+win(TOOLS/'msvc600/Common/MSDev98/Bin')
    with tempfile.TemporaryDirectory(prefix='cmr2-low-logic-') as tmp:
        (Path(tmp)/'logic.c').write_text(source+DRIVER+loads+TAIL)
        subprocess.run(['wine',str(TOOLS/'msvc600/VC98/Bin/CL.EXE'),'/nologo','/O2','/MD','logic.c','/Felogic.exe'],cwd=tmp,env=env,check=True,stdout=subprocess.DEVNULL)
        subprocess.run(['wine','logic.exe'],cwd=tmp,env=env,check=True)

if __name__=='__main__': main()
