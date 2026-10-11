#!/usr/bin/env python3
"""Execute actual lookup/timing/telemetry bodies natively against x86 oracles.

Production headers define every recovered record. Projection of Car's original
pointer-free spans belongs to the fixture only. Route/math leaves are controlled;
full input heaps, output workspaces and native pointers are checked.
"""
import itertools,json,math,os,random,shutil,struct,subprocess,tempfile
from pathlib import Path
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_FPCW
from differential_lookup_pointers import Lookup,cases
from differential_rally_times import Times
from differential_ai_telemetry import Telemetry
from differential_menu_list import ROOT,HEAP
from check_64bit_race_state_pointers import body

PRELUDE=r'''
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <vector>
#include <array>
typedef unsigned char BYTE;
typedef long long __int64;
#include "Car.h"
#include "SoundCurve.h"
#include "AIReferenceRoute.h"
#include "StageTimingSpread.h"
unsigned short g_sqrtTable[4096];int g_sinTable[4096];
AIReferenceRoute *g_aiReferenceRoutes[9];
char *g_unk0x0054241c;
int fixturePoints,fixtureSplits;
std::array<int,3> fixtureOrigin;
std::vector<std::array<int,3>> fixtureNodes,fixtureAngles;
size_t angleCursor;
int RallyData_GetRouteAvailabilityState(){return fixturePoints;}
int RallyData_GetRouteStateValue(){return (int)fixtureNodes.size();}
int GetStageSplitCount(){return fixtureSplits;}
int Stage_GetSplitPositionFixed(int index){return index*(10000/fixtureSplits)*65536;}
void RallyData_GetRouteNodeGroundPosition(int index,int *out){
 const auto &p=fixtureNodes.empty()?fixtureOrigin:fixtureNodes.at(index);
 memcpy(out,p.data(),12);
}
int StageObject_Atan2Degrees(int y,int x){
 const auto &p=fixtureAngles.at(angleCursor++);assert(p[0]==y && p[1]==x);return p[2];
}
double g_fixedDegreesToAngle12=4096.0/(360.0*65536.0);
double g_minus65536=-65536.0;
'''
MAIN=r'''
int main(int argc,char **argv){
 assert(argc>=2);static_assert(sizeof(void *)==8,"native pointers");
 static_assert(sizeof(SoundCurve)==32 && offsetof(SoundCurve,pSamples)==16 && offsetof(SoundCurve,pBytes)==24,"native sound fields");
 static_assert(sizeof(AIReferenceRoute)==0x1720 && offsetof(AIReferenceRoute,positionZ)==0x1158,"unchanged disk layout");
 static_assert(sizeof(StageTimingSpread)==0xb0 && offsetof(StageTimingSpread,startBlendFixed)==0xa8,"numeric workspace");
 alignas(16) BYTE heap[65536];assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap));
 assert((uintptr_t)heap>UINT32_MAX && (uintptr_t)g_aiReferenceRoutes>UINT32_MAX);
 unsigned result=0;bool routes=!strcmp(argv[1],"route");
 if(!strcmp(argv[1],"curve")){
  int value=atoi(argv[2]),target=atoi(argv[3]);SoundCurve curve;
  memcpy(&curve,heap+0x800,12);curve.pSamples=(unsigned short *)(heap+0x2000);curve.pBytes=heap+0x1000;auto before=curve;
  assert((uintptr_t)&curve>UINT32_MAX && (uintptr_t)curve.pSamples>UINT32_MAX && (uintptr_t)curve.pBytes>UINT32_MAX);
  if(target==0x427ab0)result=NetRace_IsValueWithinCurveRange(value,&curve);
  else if(target==0x427ad0)result=NetRace_InterpolateByteCurve(value,&curve);
  else {assert(target==0x427b70);result=NetRace_InterpolateWordCurve(value,&curve);}
  assert(!memcmp(&curve,&before,sizeof(curve)));
 }else if(routes){
  int count=atoi(argv[2]);fixturePoints=atoi(argv[3]);fixtureOrigin={atoi(argv[4]),atoi(argv[5]),atoi(argv[6])};
  for(int i=0;i<9;i++)g_aiReferenceRoutes[i]=(AIReferenceRoute *)(heap+0xf000+i*4);
  BYTE *end=StageTiming_RelocateCarRecordRoutePoints(heap+0x1000,nullptr,count);
  assert((uintptr_t)end>UINT32_MAX);result=0x10000000u+(unsigned)(end-heap);
 }else if(!strcmp(argv[1],"spread")){
  fixtureSplits=atoi(argv[2]);fixturePoints=10000;g_unk0x0054241c=(char *)(heap+0x1000);
  StageTimingSpread workspace;memcpy(&workspace,heap+0x3000,sizeof(workspace));
  assert((uintptr_t)&workspace>UINT32_MAX);
  StageTiming_BlendCarPaintDecalRows(&workspace,(int *)(heap+0x4000));
  StageTiming_BuildSplitInterpolationFactors(&workspace);
  StageTiming_InterpolateCarFixedSampleRows(&workspace,(int *)(heap+0x5000));
  memcpy(heap+0x3000,&workspace,sizeof(workspace));
 }else {
  assert(!strcmp(argv[1],"telemetry"));unsigned mask=(unsigned)strtoul(argv[2],nullptr,10);int variant=atoi(argv[3]);
  fixtureNodes.resize(12);assert(fread(fixtureNodes.data(),12,12,stdin)==12);
  unsigned angles;assert(fread(&angles,4,1,stdin)==1);fixtureAngles.resize(angles);assert(fread(fixtureAngles.data(),12,angles,stdin)==angles);
  Car car;SceneNode node={};memset(&car,0xa5,sizeof(car));
  // Fixture-only translation of the original Car's two numeric spans.
  memcpy(&car,heap+0x1000,0x71c);memcpy(&car.collisionRadius,heap+0x1000+0x758,0xc24-0x758);
  car.pSceneRoot=car.pBodyNode=car.pAlternateBodyNode=&node;
  for(int i=0;i<4;i++)car.pExtraNodes[i]=car.pWheelNodes[i]=&node;
  car.pViewNodeNear=car.pViewNodeFar=&node;car.pWorld=&car.physicsMatrix;car.pBodyMatrix=&car.bodyMatrix;
  auto before=car;
  for(int i=0;i<9;i++)g_aiReferenceRoutes[i]=(AIReferenceRoute *)(heap+0x4000);
  for(int i=0;i<4096;i++){g_sqrtTable[i]=(unsigned short)(sqrt((8+16*i)/65536.0)*65536);g_sinTable[i]=(int)(sin(i*6.283185307179586/4096)*65536);}
  assert((uintptr_t)&car>UINT32_MAX && (uintptr_t)car.pWorld>UINT32_MAX && offsetof(Car,speed)!=0x778);
  StageTiming_RecomputeCarSplitBarSamples(&car,mask,(int *)(heap+0x3000),variant);
  assert(!memcmp(&car,&before,sizeof(car)) && angleCursor==fixtureAngles.size());
 }
 assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));assert(fwrite(&result,4,1,stdout)==1);
 if(routes)for(int i=0;i<9;i++){assert((uintptr_t)g_aiReferenceRoutes[i]>UINT32_MAX);unsigned p=0x10000000u+(unsigned)((BYTE *)g_aiReferenceRoutes[i]-heap);assert(fwrite(&p,4,1,stdout)==1);}
}
'''

def main():
 source=ROOT/'CMR2Decomp'
 names={'NetRace.cpp':['NetRace_IsValueWithinCurveRange','NetRace_InterpolateByteCurve','NetRace_InterpolateWordCurve'],'StageTiming.cpp':['StageTiming_RelocateCarRecordRoutePoints','StageTiming_BlendCarPaintDecalRows','StageTiming_BuildSplitInterpolationFactors','StageTiming_InterpolateCarFixedSampleRows','StageTiming_RecomputeCarSplitBarSamples']}
 funcs='\n'.join(body((source/file).read_text(),name) for file,items in names.items() for name in items)
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
 with tempfile.TemporaryDirectory(prefix='cmr2-native-lookups-') as directory:
  tmp=Path(directory)
  for header in ('FloatToInt.h','Car.h','SceneNode.h','Sector.h','LayoutChecks.h','SoundCurve.h','AIReferenceRoute.h','StageTimingSpread.h'):shutil.copyfile(source/header,tmp/header)
  shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
  (tmp/'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;\n')
  (tmp/'Graphics.h').write_text('struct Graphics;struct Texture;class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0;static constexpr double m_65536=65536.0;};\n')
  (tmp/'Mesh.h').write_text('struct Mesh;\n')
  fixture=tmp/'lookup.cpp';fixture.write_text(PRELUDE+funcs+MAIN);exe=tmp/'lookup'
  subprocess.run(['clang++','-include',str(ROOT/'CMR2Decomp/FloatToInt.h'),'-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-I',str(tmp),'-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast','-fsanitize=address,undefined',str(fixture),'-o',str(exe)],check=True)
  def native(args,input,expected):
   p=subprocess.run([str(exe),*map(str,args)],input=input,capture_output=True,env=env)
   assert p.returncode==0,p.stderr.decode()
   if p.stdout!=expected:
    differing=[i for i,(a,b) in enumerate(zip(p.stdout,expected)) if a!=b]
    raise AssertionError((args,differing[:20],[(hex(i),p.stdout[i:i+4].hex(),expected[i:i+4].hex()) for i in differing[:4]]))
  oracle=Lookup(ROOT/'cmr2bin/CMR2.exe');count=0
  for kind,args in cases():
   observed=getattr(oracle,kind)(*args)
   if kind=='curve':_,_,value,target=args;command=[kind,value,target]
   else:_,records,points=args;command=[kind,records,points,*oracle.route_origin]
   expected=observed[0]+struct.pack('<I',observed[1])+(observed[2] if kind=='route' else b'')
   native(command,oracle.initial,expected);count+=1
  times=Times(ROOT/'cmr2bin/CMR2.exe');times.difficulty=times.mode=0
  for seed,splits in itertools.product(range(8),[2,4,5,8,9]):
   times.reset();times.splits=splits;r=random.Random(seed);times.u.mem_write(HEAP,r.randbytes(0x10000))
   times.put(times.addr(0x54241c),'<I',HEAP+0x1000);times.u.mem_write(HEAP+0x1000,bytes(r.randrange(1,25) for _ in range(80)))
   for i in range(160):times.put(HEAP+0x1080+i*4,'<I',r.randrange(40,140)<<16)
   times.put(HEAP+0x3000+0xa0,'<4I',1200<<16,8000<<16,seed*4096,65536-seed*4096)
   initial=times.read(HEAP,0x10000)
   times.invoke(0x455f00,[HEAP+0x3000,HEAP+0x4000]);times.invoke(0x456330,[HEAP+0x3000]);times.invoke(0x4564d0,[HEAP+0x3000,HEAP+0x5000])
   native(['spread',splits],initial,times.read(HEAP,0x10000)+struct.pack('<I',0));count+=1
  telemetry=Telemetry(ROOT/'cmr2bin/CMR2.exe');capture={}
  def before(u,address,size,data):
   if address==0x498620:capture['heap']=telemetry.read(HEAP,0x10000)
  telemetry.u.hook_add(UC_HOOK_CODE,before)
  for seed,mask,variant in itertools.product(range(8),[0,3,0xfffffffc,0xffffffff],[1,2,3]):
   # Native C++ casts truncate; /QIfist uses the ambient x87 rounding mode.
   # The registered x86 harness separately preserves its original default mode.
   telemetry.u.reg_write(UC_X86_REG_FPCW,0xe7f)  # 53-bit precision, truncating conversion.
   heap,trace=telemetry.run(seed,mask,variant)
   nodes=b''.join(struct.pack('<3i',*node) for node in telemetry.nodes)
   angles=struct.pack('<I',len(trace))+b''.join(struct.pack('<3i',y,x,int(math.atan2(y,x)*57.295827908797776*65536)) for y,x in trace)
   native(['telemetry',mask,variant],capture['heap']+nodes+angles,heap+struct.pack('<I',0));count+=1
  print(f'PASS: {count} actual x64 lookup/route/timing/telemetry scenarios against original x86; production headers, full guarded heaps, pointers above 4 GB, ASan/UBSan')

if __name__=='__main__':main()
