#!/usr/bin/env python3
"""Actual co-driver pointer family on native x64 against original x86.

Uses production queue/sprite declarations and five complete function bodies.
No texture fields are read; the opaque texture fixture checks full native pointer
identity, output width and rendering argument delivery under ASan/UBSan.
"""
import itertools,os,re,shutil,struct,subprocess,tempfile
from pathlib import Path
from check_64bit_car_contacts import body
from differential_codriver_calls import Calls,ROOT,HEAP,RECTS,SINES,icon_model

TAGS={name:i+1 for i,name in enumerate(['cheat','car-blocked','head-to-head','split','menu','editing','event-count','status','grid','assign','message','name','archive','register','reset','text','format','file','sprite'])}
def trace_bytes(trace):
 out=bytearray()
 for event in trace:
  out.extend(struct.pack('<I',TAGS[event[0]]))
  values=event[1:] if event[0]=='sprite' else event[1] if len(event)>1 else ()
  for value in values:out.extend(value if isinstance(value,bytes) else struct.pack('<I',value&0xffffffff))
 return out
PRELUDE=r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <vector>
typedef unsigned char BYTE;typedef unsigned int DWORD;typedef long long __int64;typedef uintptr_t UINT_PTR;
struct D3DTLVERTEX {float sx,sy,sz,rhw;DWORD colour,specular;float tu,tv;};
#include "Sprite.h"
#include "FixedPoint.h"
#include "RaceCallRecord.h"
struct Graphics {int resX,resY;};Graphics graphics,*g_pGraphics=&graphics;
struct CGraphics{static constexpr double m_65536=65536.0;};
Texture texture,*g_arrowTexture=&texture;SpriteRect arrowRects[8],*g_pArrowRects=arrowRects;
RaceCallRecord g_raceCallRecords[10];unsigned int *g_coDriverCalls,*g_loadedCoDriverCalls;
int g_raceWrongWayFlags[2],g_unk0x00537350,g_unk0x0053735c,g_unk0x00517e24=-1;
int g_sinTable[4096];BYTE image[65536];int caseId;std::vector<BYTE> trace;
char g_strCodFormat[]="%s.cod";
void bytes(std::vector<BYTE> &out,const void *p,size_t size){auto *b=(const BYTE *)p;out.insert(out.end(),b,b+size);}
void put(std::vector<BYTE> &out,uint32_t v){bytes(out,&v,4);}
void event(int tag){put(trace,tag);}void arg(int v){put(trace,(uint32_t)v);}
struct CGameInfo{
 static int IsActiveCheatEnabled(int v){assert(v==2);event(1);arg(v);return caseId%2;}
 static int IsSplitBarEnabled(){event(4);return (caseId/4)%2;}
 static int IsInRaceMenuOpen(){event(5);return (caseId/8)%2;}
};
struct CFrontend{static char m_stringDest[260];static char *GetTextString(int n){assert(n==0x57);event(16);arg(n);return (char *)image+0x7200;}};char CFrontend::m_stringDest[260];
int fixture_sprintf(char *out,const char *format,...){event(17);va_list ap;va_start(ap,format);int result=vsprintf(out,format,ap);va_end(ap);return result;}
#define sprintf fixture_sprintf
unsigned int RallyData_GetSelectionFlag26(){return caseId%2;}
int RallyData_IsHeadToHeadRaceMode(){event(3);return (caseId/2)%2;}
int InRaceMenu_IsPlayerEditingCarSetup(unsigned int n){event(6);arg(n);return (caseId/16)%2;}
BYTE StageUI_GetRaceEndEventCount(){event(7);return 1;}
BYTE *RallyData_GetDriverKnockoutOrTeamRecord(BYTE i){event(8);arg(i);return image+0x2000;}
void StageUI_DrawStageGrid(int a,int b){event(9);arg(a);arg(b);}
void Race_AssignSlotsFromCallRecord(unsigned int v){event(10);arg(v);}
void Race_DrawPlayerMessage(char *text,int *colour,int player,int shadow,int x,int y){assert(!strcmp(text,"wrong way"));event(11);bytes(trace,colour,4);arg(player);arg(shadow);arg(x);arg(y);}
int Race_IsFlag24Clear(){return !((caseId/32)%2);}
int Car_IsStateC10Set(int n){event(2);arg(n);return caseId%2;}
void Sprite_Queue(SpriteRect *src,SpriteRect *dst,Texture *t,int layer,short angle,int *centre,SpriteRect *uv,BYTE *colour,int flags){
 assert(t==&texture && (uintptr_t)t>UINT32_MAX && layer==2 && !angle && !centre && !uv);event(19);bytes(trace,src,8);bytes(trace,dst,8);bytes(trace,colour,4);arg(flags);
}
char *GameMenu_GetChampionshipTransitionState(){event(12);return (char *)image+0x7000;}
void *StageTiming_GetStageFile3(){event(13);return image+0x7100;}
int Race_ReleaseCoDriverCallData();
struct CGame {static int RegisterCallback(int(*callback)(),int v){assert(callback==Race_ReleaseCoDriverCallData && !v);event(14);return 0;}};
void Race_ResetPlayerCallState(){event(15);}
struct GenericFile{};
struct CGenericFileLoader{static void *FindFile(GenericFile *archive,char *name,int flags,DWORD *size,int other){assert(archive==(GenericFile *)(image+0x7100) && !flags && !size && !other && !strcmp(name,"route.cod"));event(18);return caseId%2?nullptr:image+0x3000;}};
int Race_DecodeCallSlotIcon(unsigned int,Texture **,SpriteRect *,unsigned int *,BYTE *);
void read(void *p,size_t n){assert(fread(p,1,n,stdin)==n);}
'''
MAIN=r'''
int main(int argc,char **argv){
 assert(argc==7 && sizeof(void *)==8);int kind=atoi(argv[1]);caseId=atoi(argv[2]);unsigned int player=atoi(argv[3]),callId=strtoul(argv[4],nullptr,0),previous=atoi(argv[5]);int freeSlot=atoi(argv[6]);
 static_assert(sizeof(RaceCallRecord)==12,"numeric queue slots");assert((uintptr_t)&texture>UINT32_MAX);
 read(image,sizeof(image));read(g_raceCallRecords,sizeof(g_raceCallRecords));read(g_sinTable,sizeof(g_sinTable));
 graphics.resX=640+(caseId%3)*160;graphics.resY=480+(caseId%3)*120;
 memcpy(arrowRects,image+0x5000,64);g_coDriverCalls=(unsigned int *)(image+0x3000);g_loadedCoDriverCalls=(unsigned int *)(image+0x4000);
 g_unk0x00537350=caseId%3?-1:2;g_raceWrongWayFlags[0]=caseId%3==1;g_raceWrongWayFlags[1]=caseId%3==2;
 auto oldImage=std::vector<BYTE>(image,image+sizeof(image));std::vector<BYTE> output;
 if(kind==0){
  struct Outputs {Texture *t;uint64_t guard;SpriteRect rect;unsigned int flag;BYTE colour[4];} out;
  memset(&out,0xa5,sizeof(out));out.guard=0x123456789abcdef0;
  int result=Race_DecodeCallSlotIcon(callId,&out.t,&out.rect,&out.flag,out.colour);
  assert(out.t==&texture && out.guard==0x123456789abcdef0);put(output,result);bytes(output,&out.rect,8);put(output,out.flag);bytes(output,out.colour,4);
 }else{
  if(kind==1){for(int i=0;i<5;i++)g_raceCallRecords[player*5+i].flags=i<freeSlot?0x100:0x200;Race_QueuePlayerCallout(player,(BYTE)callId,(BYTE)previous,77);}
  else if(kind==2){if(caseId%5==0)g_raceCallRecords[player*5].flags=caseId%51;Race_DrawPlayerCallSlots(player,0x1234);}
  else if(kind==3)Race_LoadCoDriverCallData();
  else assert(Race_ReleaseCoDriverCallData()==1);
  bytes(output,g_raceCallRecords,sizeof(g_raceCallRecords));put(output,g_coDriverCalls?0x10003000:0);put(output,g_loadedCoDriverCalls?(g_loadedCoDriverCalls==(unsigned int *)(image+0x3000)?0x10003000:0x10004000):0);
 }
 assert(!memcmp(image,oldImage.data(),sizeof(image)));bytes(output,trace.data(),trace.size());assert(fwrite(output.data(),1,output.size(),stdout)==output.size());
}
'''
def main():
 source=ROOT/'CMR2Decomp';names=['Race_DecodeCallSlotIcon','Race_QueuePlayerCallout','Race_DrawPlayerCallSlots','Race_ReleaseCoDriverCallData','Race_LoadCoDriverCallData']
 funcs='\n'.join(body((source/'Race.cpp').read_text(),name) for name in names);env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
 with tempfile.TemporaryDirectory(prefix='cmr2-native-calls-') as directory:
  tmp=Path(directory);shutil.copyfile(source/'RaceCallRecord.h',tmp/'RaceCallRecord.h');shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
  (tmp/'Texture.h').write_text('struct Texture {};\n');(tmp/'Sprite.h').write_text(re.sub(r'^#include .*dx7sdk.*\n','',(source/'Sprite.h').read_text(),flags=re.M))
  cpp=tmp/'calls.cpp';cpp.write_text(PRELUDE+funcs+MAIN);exe=tmp/'calls'
  subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-I',str(tmp),'-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast','-fsanitize=address,undefined',str(cpp),'-o',str(exe)],check=True)
  oracle=Calls(ROOT/'cmr2bin/CMR2.exe');number=0
  cases=[(0,c,0,t|(b1<<4)|(b2<<17)|(b3<<21),255,0) for t,b1,b2,b3,c in itertools.product(range(16),range(4),[0,1,2,3,14],range(4),range(2))]
  cases +=[(1,*args) for args in itertools.product(range(4),range(2),[255,0,1,4,7,11],[255,0,3,5],range(6))]
  cases +=[(kind,case,player,0,255,0) for kind,case,player in itertools.product([2,3,4],range(128),range(2))]
  for args in cases:
   kind,case,player,call,previous,free=args;image=oracle.initialize(case);data=image+oracle.read(oracle.addr(0x5371d0),120)+struct.pack('<4096i',*SINES)
   if kind==0:
    oracle.decode(call,case);model=icon_model(call,case);expected=struct.pack('<I',model[0])+model[1]+struct.pack('<I',model[2])+model[3]+trace_bytes(oracle.trace)
   else:
    result=oracle.run({1:'queue',2:'draw',3:'load',4:'release'}[kind],case,player,call,previous,free);expected=b''.join(result[:3])+trace_bytes(result[3])
   p=subprocess.run([str(exe),*map(str,args)],input=data,capture_output=True,env=env)
   assert not p.returncode,(args,p.stderr.decode());assert p.stdout==expected,(args,len(p.stdout),len(expected),[(hex(i),a,b) for i,(a,b) in enumerate(zip(p.stdout,expected)) if a!=b][:12]);number+=1
 print(f'PASS: {number} native co-driver scenarios vs original x86; five actual bodies, full queue/render/provider traces, 8-byte texture outputs, pointers above 4GB, production declarations, ASan/UBSan')
if __name__=='__main__':main()
