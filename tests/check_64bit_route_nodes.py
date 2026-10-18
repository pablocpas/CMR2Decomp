#!/usr/bin/env python3
"""Actual .cat node family with native pointers, compared to original x86.

Compiles production layouts/bodies using existing portable fixed-point leaves.
Only resource, scene and audio providers are controlled. The disk node remains
44 bytes; cache, race/call records and checkpoint state are compared in full.
"""
import itertools,json,os,shutil,struct,subprocess,tempfile
from pathlib import Path
from check_64bit_race_state_pointers import body
from differential_route_nodes import Nodes,ROOT,HEAP

# Canonical pointer values are serialized separately from the numeric globals.
GLOBALS=[('g_unk0x00538a84',0x538a84,4),('g_unk0x00538a88',0x538a88,4),('g_unk0x00538a94',0x538a94,4),('g_routeDirKey',0x538a8c,6),('g_routeDirCount',0x538aa0,2),('g_routeDir',0x538b68,36),('g_raceRecords',0x538aa8,192),('g_unk0x00538c94',0x538c94,4),('g_unk0x0053708c',0x53708c,8),('g_unk0x00537198',0x537198,8),('g_raceWrongWayFlags',0x537248,8),('g_raceCallRecords',0x5371d0,120),('g_unk0x00537354',0x537354,4),('g_unk0x00537350',0x537350,4),('g_stageCheckpointCount',0x542c70,4),('g_stageSplitCount',0x542cac,1),('g_stageLooped',0x542cae,1),('g_splitPositionCount',0x542c74,4),('g_unk0x00542c78',0x542c78,4),('g_splitPositions',0x542c7c,48)]
PRELUDE=r'''
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
typedef unsigned char BYTE;typedef unsigned int DWORD;typedef long long __int64;
#include "Car.h"
#include "RallyRouteNode.h"
#include "RaceRecord.h"
#include "CallbackState.h"
unsigned short g_sqrtTable[4096];int g_sinTable[4096];
RallyRouteNode *g_routeNodes;short g_routeDirKey[3],g_routeDirCount;FixVector g_routeDir[3];
int g_unk0x00538a84,g_unk0x00538a88,g_unk0x00538a94,g_unk0x00538c94;
RaceRecord g_raceRecords[8];
#include "RaceCallRecord.h"
RaceCallRecord g_raceCallRecords[10];
int g_unk0x0053708c[2],g_unk0x00537198[2],g_raceWrongWayFlags[2],g_unk0x005371a0=1,g_unk0x00537350,g_unk0x00537354;
BYTE g_coDriverFirstSample[32];
int g_stageCheckpointCount,g_splitPositionCount,g_unk0x00542c78,g_splitPositions[12];char g_stageLooped,g_stageSplitCount;
Car car,*g_pCurrentCar;CallbackStateRecord states[2];CallbackStateMachine machine;
BYTE image[65536];int caseId,nodeCount,missing,best,overlaps;unsigned int selection;
char g_str0x00519704[8]="%s.cat";
struct CFrontend {static char m_stringDest[260];};char CFrontend::m_stringDest[260];
struct GenericFile{};
struct CGenericFileLoader {static void *FindFile(GenericFile *p,char *name,int a,DWORD *size,int b){assert(p==(GenericFile *)(image+0x7100) && !a && !b && !strcmp(name,"route.cat"));*size=missing?0:nodeCount*44+7;return missing?nullptr:image+0x3000;}};
struct CGame{static int RegisterCallback(void *,int n){assert(!n);return 0;}};
char *GameMenu_GetChampionshipTransitionState(){return (char *)image+0x7000;}
void *StageTiming_GetStageFile3(){return image+0x7100;}
void StageTiming_NoOpResourceRelease(){}
void RallyData_ResetPlayerRouteProbes(){}
int RallyData_GetSelectionBits16To19(){return 3;}
int RallyData_GetRouteAvailabilityState(){return g_unk0x00538a84;}
int RallyData_GetRouteAdditionalState(){return g_unk0x00538a94;}
unsigned int RallyData_GetFlag24(){return (selection>>24)&1;}
unsigned int RallyData_GetFlag25(){return (selection>>25)&1;}
unsigned int RallyData_GetSelectionFlag26(){return (selection>>26)&1;}
unsigned int RallyData_GetSelectionFlag28(){return (selection>>28)&1;}
Car *Car_Get(int i){assert(i==0 || i==1);return &car;}
int RallyData_GetActiveCarRaceRecordField0(Car *p){assert(p==&car);return best;}
int Car_OverlapsOtherCar(int){int result=overlaps>0;overlaps-=result;return result;}
void Car_PlaceAtStart(int *,int *){}
void Race_StorePlayerRouteAndClearSlots(int){}
void Particle_KillAll(){}
void View_ResetCameras(unsigned int){}
BYTE RallyDataState(){return 2;}
BYTE StageTiming_GetCheckpointField18(int){return caseId%2;}
int StageTiming_GetClockTime(){return 80+caseId%20;}
void *StageUI_GetRaceResultTable(){return &machine;}
int Car_IsStateC10Set(int){return (caseId/3)%2;}
int RallyData_GetCarRecordValueRatio(Car *){return 32768;}
void StageObject_SetBoundedWeatherKind(int){}
void Race_AssignUnusedSlot(BYTE *){}
void RaceCalls_UpdatePlaybackQueue(){}
int Race_IsFlag24Clear(){return !RallyData_GetFlag24();}
void Race_QueuePlayerCallout(int,BYTE,BYTE,BYTE){}
void RallyRoute_GetNodeDirection(FixVector *,unsigned int);
void RallyRoute_CopyIndexedNodeDirection(unsigned int,FixVector *);
void RallyData_GetRouteNodeGroundPosition(int,int *);
RallyRouteNode *RallyData_GetAvailableRouteNodeRecord(int);
std::vector<BYTE> output;
void bytes(const void *p,size_t n){auto *b=(const BYTE *)p;output.insert(output.end(),b,b+n);}
void read(void *p,size_t n){assert(fread(p,1,n,stdin)==n);}
int word(int at){int n;memcpy(&n,image+at,4);return n;}
'''
MAIN=r'''
int main(int argc,char **argv){
 assert(argc==5 && sizeof(void *)==8);int kind=atoi(argv[1]);caseId=atoi(argv[2]);nodeCount=atoi(argv[3]);missing=atoi(argv[4]);
 static_assert(sizeof(RallyRouteNode)==44 && offsetof(RallyRouteNode,callId)==25,"pointer-free disk nodes");
 static_assert(offsetof(Car,field_0xbf8)!=0xbf8,"native Car fields move");
 for(int i=0;i<4096;i++)g_sqrtTable[i]=(unsigned short)(sqrt(8+16*i)*256);
 read(image,sizeof(image));
 INPUT_GLOBALS
 g_routeNodes=(RallyRouteNode *)(image+0x3000);assert((uintptr_t)g_routeNodes>UINT32_MAX && (uintptr_t)&car>UINT32_MAX);
 best=caseId%8;overlaps=caseId%3;selection=(caseId%2<<24)|((caseId/2)%2<<25)|((caseId/4)%2<<26)|((caseId/8)%2<<28);
 car.index=image[0x1000+0xb1a];memcpy(&car.rearWheelDir,image+0x1000+0x3f0,12);memcpy(&car.position,image+0x1000+0x2d0,12);memcpy(&car.right,image+0x1000+0x360,12);
 car.field_0xbf8=word(0x1000+0xbf8);car.field_0xa7c=word(0x1000+0xa7c);car.field_0xa80=word(0x1000+0xa80);
 states[0].packedState=caseId%4?8:9;states[1].packedState=8;machine.records=states;
 BYTE oldNodes[16*44];memcpy(oldNodes,image+0x3000,nodeCount*44);
 if(kind==0)RallyData_LoadSelectedRouteNodes();
 else if(kind==1)Car_ResetToStage(caseId%2);
 else if(kind==2)Race_UpdateCarRouteAndWrongWayState(caseId%2);
 else if(kind==3)StageTiming_BuildCheckpointSplitPositions();
 else for(int i=0;i<nodeCount;i++){auto *n=RallyData_GetAvailableRouteNodeRecord(i);assert(n==&g_routeNodes[i]);RallyData_GetRouteNodeGroundPosition(i,(int *)(image+0x8000));RallyRoute_GetNodeDirection((FixVector *)(image+0x8100),i);}
 assert(!memcmp(oldNodes,image+0x3000,nodeCount*44));
 OUTPUT_GLOBALS
 uint32_t canonicalPointer=g_routeNodes?0x10003000:0;bytes(&canonicalPointer,4);
 int fields[]={car.field_0xbf8,car.field_0xa7c,car.field_0xa80};bytes(fields,sizeof(fields));
 if(kind==4){bytes(image+0x8000,12);bytes(image+0x8100,12);}
 assert(fwrite(output.data(),1,output.size(),stdout)==output.size());
}
'''
def main():
 source=ROOT/'CMR2Decomp';names={'RallyRoute.cpp':['RallyRoute_GetNodeDirection','RallyRoute_CopyIndexedNodeDirection'],'RallyData.cpp':['RallyData_GetAvailableRouteNodeRecord','RallyData_GetRouteNodeGroundPosition','RallyData_LoadSelectedRouteNodes'],'Car.cpp':['Car_ResetToStage'],'Race.cpp':['Race_UpdateCarRouteAndWrongWayState'],'StageTiming.cpp':['StageTiming_BuildCheckpointSplitPositions']}
 funcs='\n'.join(body((source/file).read_text(),name) for file,items in names.items() for name in items)
 code=MAIN.replace('INPUT_GLOBALS','\n'.join(f'read(&{n},{size});' for n,_,size in GLOBALS)).replace('OUTPUT_GLOBALS','\n'.join(f'bytes(&{n},{size});' for n,_,size in GLOBALS))
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1');oracle=Nodes(ROOT/'cmr2bin/CMR2.exe');number=0
 with tempfile.TemporaryDirectory(prefix='cmr2-native-route-') as directory:
  tmp=Path(directory)
  for h in ('FloatToInt.h','Car.h','SceneNode.h','Sector.h','Mesh.h','LayoutChecks.h','RallyRouteNode.h','RaceRecord.h','RaceCallRecord.h','CallbackState.h'):shutil.copyfile(source/h,tmp/h)
  shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
  (tmp/'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;\n')
  (tmp/'Graphics.h').write_text('class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0;static constexpr double m_65536=65536.0;};\n')
  cpp=tmp/'route.cpp';cpp.write_text(PRELUDE+funcs+code);exe=tmp/'route'
  subprocess.run(['clang++','-include',str(ROOT/'CMR2Decomp/FloatToInt.h'),'-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-I',str(tmp),'-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast','-fsanitize=address,undefined',str(cpp),'-o',str(exe)],check=True)
  for kind,case,count,missing in itertools.product(range(5),range(32),[12,16],[False,True]):
   if kind==4 and missing:continue
   image=oracle.initialize(case,count,missing);data=image+b''.join(oracle.read(oracle.addr(a),size) for _,a,size in GLOBALS)
   oracle.run(['load','reset','calls','splits','get'][kind],case,count,missing)
   expected=b''.join(oracle.read(oracle.addr(a),size) for _,a,size in GLOBALS)+oracle.read(oracle.addr(0x538a9c),4)
   expected+=b''.join(oracle.read(HEAP+0x1000+at,4) for at in (0xbf8,0xa7c,0xa80))
   if kind==4:expected+=oracle.read(HEAP+0x8000,12)+oracle.read(HEAP+0x8100,12)
   result=subprocess.run([str(exe),*map(str,(kind,case,count,int(missing)))],input=data,capture_output=True,env=env)
   assert result.returncode==0,((kind,case,count,missing),result.stderr.decode());assert result.stdout==expected,((kind,case,count,missing),[(hex(i),a,b) for i,(a,b) in enumerate(zip(result.stdout,expected)) if a!=b][:20]);number+=1
 print(f'PASS: {number} native route-node scenarios vs original x86; eight actual bodies, production disk/Car layouts, full cache/race/call/checkpoint state, pointers above 4GB, ASan/UBSan')
if __name__=='__main__':main()
