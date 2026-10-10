#!/usr/bin/env python3
"""Actual table walk bodies on native pointers, real layouts and guarded data.

Allocation/release, audio, controller and championship sort leaves are controlled.
Stage load executes the complete failure path of its first model (including all
six pointer-table clears and sixteen scene pairs); later model/render paths are
outside this fixture's coverage.
"""
import os,re,subprocess,tempfile
from pathlib import Path
from check_64bit_car_contacts import ROOT,body

def actual(source,name):
 source=source.replace('BOOL '+name+'(', 'int '+name+'(')
 return body(source,name)
def record(source,name):
 start=source.index('struct '+name+' {');pos=source.index('{',start)+1;depth=1
 while depth:depth+=(source[pos]=='{')-(source[pos]=='}');pos+=1
 return source[start:pos]+';\n'
PRELUDE=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <numeric>
#include <initializer_list>
using BYTE=unsigned char;using DWORD=uint32_t;using BOOL=int;using INT_PTR=intptr_t;using __int64=long long;
#define TRUE 1
#define MAX_PATH 260
#include "StageTiming.h"
#include "NetPlayers.h"
#include "RallyTiming.h"
struct FixVector;struct FixAngles;
#include "CarResources.h"
struct Car;struct Menu;struct SoundSlot;struct CarFlexibleLineState;
CarPartStateTables g_carPartStateTables;ReplayLevelState g_replayLevelState;
RallyStageTables g_rallyStageTables;RallyOverallTables g_rallyOverallTables;NetPlayer g_netPlayers[8];
void *g_replayStreams[8];int g_unk0x00588d3c;int g_carPairValuePending[16];
CarFlexibleLineState **g_carLineStates;int g_carPartTableCarCount,g_soundMasterVolume;
struct CSound{static SoundSlot *m_soundSlots[32];};SoundSlot *CSound::m_soundSlots[32];
alignas(16) BYTE payload[32][32];std::vector<void*> freed,volumes;
struct CFileBuffer{static void FreeGenericFileBuffer(void *p){assert((uintptr_t)p>UINT32_MAX);freed.push_back(p);}};
void Sound_ApplySlotVolumeAttenuation(SoundSlot *p){assert((uintptr_t)p>UINT32_MAX);volumes.push_back(p);}
CarSceneRecord g_carScenes[16];CarInfoDirectory *g_carInfoBuffers[8];void *g_carAuxiliaryBuffers[16],*g_carWheelModelBuffers[16],*g_carAlternateBodyBuffers[16],*g_carModelBuffers[16];SceneNode *g_carAlternateBodyScenes[16];
unsigned stageCalls;
struct CGameInfo{static int GetGameModeOptionBit19(){++stageCalls;return 0;}static int GetSoundOptionBit30(){assert(false);return 0;}};
struct CGame{template<class...A>static void RegisterCallback(A...){assert(false);}};
struct CGraphics{static double m_65536;};double CGraphics::m_65536=65536;
unsigned selectedRound;int RallyData_GetSelectionBits12To13(){return selectedRound;}
int RallyTiming_GetPositionPoints(int i){int points[]={6,4,3,2,1,0,0,0};return points[i];}
void RallyTiming_SortOrder(int *totals,char *order,int direction,int count,char initialize){assert(direction==0 && count==8 && !initialize);std::stable_sort(order,order+8,[&](int a,int b){return totals[a]>totals[b];});}
unsigned controllerCalls;unsigned short mappings[8];
'''
MAIN=r'''
int main(){
 static_assert(sizeof(void*)==8,"native addresses");static_assert(sizeof(CarPartStateTables)==40,"parts grow");static_assert(sizeof(CarSceneRecord)==72 && offsetof(CarSceneRecord,bodyNode)==8,"scene record grows");
 assert((uintptr_t)g_replayStreams>UINT32_MAX && (uintptr_t)payload>UINT32_MAX && (uintptr_t)&g_carScenes>UINT32_MAX);unsigned cases=0;
 for(unsigned mask:{0u,1u,0x55u,0x80u,0xaau,0xffu})for(int count:{-1,0,1,8}){
  BYTE dataBefore[sizeof(payload)];memset(payload,0xc3,sizeof(payload));memcpy(dataBefore,payload,sizeof(payload));std::vector<void*> want;
  for(unsigned i=0;i<8;i++){g_replayStreams[i]=(mask&(1u<<i))?payload[i]:nullptr;if(g_replayStreams[i])want.push_back(payload[i]);}
  memset(&g_replayLevelState,0x93,sizeof(g_replayLevelState));ReplayLevelState expected=g_replayLevelState;expected.bufferCount=0;g_unk0x00588d3c=5;freed.clear();
  assert(Replay_ReleaseStageBuffers()==1 && freed==want && !g_unk0x00588d3c && !memcmp(&expected,&g_replayLevelState,sizeof(expected)));for(auto p:g_replayStreams)assert(!p);++cases;
  for(unsigned linesPresent=0;linesPresent<2;linesPresent++){
   CarFlexibleLineState *lines[8];CarFlexibleLineState *expectedLines[8];want.clear();
   for(unsigned i=0;i<4;i++){g_carPartStateTables.parts[i]=(mask&(1u<<i))?(PartState*)payload[i]:nullptr;g_carPartStateTables.modes[i]=0xa0+i;if(g_carPartStateTables.parts[i])want.push_back(payload[i]);}
   for(unsigned i=0;i<8;i++)lines[i]=expectedLines[i]=(mask&(1u<<i))?(CarFlexibleLineState*)payload[i+8]:nullptr;
   if(linesPresent){for(int i=0;i<count;i++)if(lines[i]){want.push_back(lines[i]);expectedLines[i]=nullptr;}want.push_back(lines);}
   g_carLineStates=linesPresent?lines:nullptr;g_carPartTableCarCount=count;freed.clear();assert(StageTiming_FreeCarPartNodeTables()==1 && freed==want && !g_carLineStates && !g_carPartTableCarCount && !memcmp(lines,expectedLines,sizeof(lines)));
   for(unsigned i=0;i<4;i++)assert(!g_carPartStateTables.parts[i] && g_carPartStateTables.modes[i]==0xa0+i);++cases;
  }
  for(int v:{-1,0,32768,65536,65537}){want.clear();for(unsigned i=0;i<32;i++){CSound::m_soundSlots[i]=(mask&(1u<<(i%8)))?(SoundSlot*)payload[i]:nullptr;if(CSound::m_soundSlots[i])want.push_back(payload[i]);}g_soundMasterVolume=12345;volumes.clear();Sound_SetMasterVolume(v);assert(volumes==want && g_soundMasterVolume==(v>=0 && v<=65536?v:12345));++cases;}
  assert(!memcmp(payload,dataBefore,sizeof(payload)));
 }
 for(unsigned seed=0;seed<32;seed++){
  memset(g_netPlayers,seed*7+1,sizeof(g_netPlayers));BYTE expectedPlayers[sizeof(g_netPlayers)];memcpy(expectedPlayers,g_netPlayers,sizeof(g_netPlayers));
  for(unsigned kind=0;kind<2;kind++){for(unsigned i=0;i<8;i++){unsigned value;memcpy(&value,expectedPlayers+i*sizeof(NetPlayer)+offsetof(NetPlayer,flags),4);value=kind?value|0x400000:value&~256u;memcpy(expectedPlayers+i*sizeof(NetPlayer)+offsetof(NetPlayer,flags),&value,4);}if(kind)NetPlayers_BlockStatisticsReception();else NetPlayers_ClearReadyFlags();assert(!memcmp(g_netPlayers,expectedPlayers,sizeof(g_netPlayers)));++cases;}
  memset(&g_rallyStageTables,seed+3,sizeof(g_rallyStageTables));RallyStageTables stageExpected=g_rallyStageTables;memset(&g_rallyOverallTables,seed+4,sizeof(g_rallyOverallTables));RallyOverallTables overallExpected=g_rallyOverallTables;
  for(unsigned i=0;i<16;i++){stageExpected.order[i]=stageExpected.positions[i]=overallExpected.order[i]=i;stageExpected.times[i]=overallExpected.times[i]=0;stageExpected.penalties[i]=stageExpected.tieBreak[i]=0;}
  RallyTiming_ResetStageResults();RallyTiming_ResetOverallPlayerTimes();assert(!memcmp(&stageExpected,&g_rallyStageTables,sizeof(stageExpected)) && !memcmp(&overallExpected,&g_rallyOverallTables,sizeof(overallExpected)));cases+=2;
  memset(&g_replayLevelState,seed+5,sizeof(g_replayLevelState));ReplayLevelState replayExpected=g_replayLevelState;memset(replayExpected.levels,0,sizeof(replayExpected.levels));memset(g_carPairValuePending,seed+6,sizeof(g_carPairValuePending));int pendingExpected[16];memcpy(pendingExpected,g_carPairValuePending,sizeof(pendingExpected));memset(pendingExpected,0,8*sizeof(int));StageObject_ResetPairedCarValues();assert(!memcmp(&replayExpected,&g_replayLevelState,sizeof(replayExpected)) && !memcmp(pendingExpected,g_carPairValuePending,sizeof(pendingExpected)));++cases;
  memset(g_carScenes,seed+7,sizeof(g_carScenes));CarSceneRecord sceneExpected[16];memcpy(sceneExpected,g_carScenes,sizeof(g_carScenes));for(auto &r:sceneExpected)r.bodyNode=r.rootNode=nullptr;
  memset(g_carInfoBuffers,0xd3,sizeof(g_carInfoBuffers));memset(g_carWheelModelBuffers,0xd3,sizeof(g_carWheelModelBuffers));memset(g_carAuxiliaryBuffers,0xd3,sizeof(g_carAuxiliaryBuffers));memset(g_carAlternateBodyScenes,0xd3,sizeof(g_carAlternateBodyScenes));memset(g_carAlternateBodyBuffers,0xd3,sizeof(g_carAlternateBodyBuffers));memset(g_carModelBuffers,0xd3,sizeof(g_carModelBuffers));stageCalls=0;
  assert(!StageTiming_LoadStageCarsAndEffects() && stageCalls==16 && !memcmp(g_carScenes,sceneExpected,sizeof(sceneExpected)));for(auto p:g_carInfoBuffers)assert(!p);for(auto p:g_carWheelModelBuffers)assert(!p);for(auto p:g_carAuxiliaryBuffers)assert(!p);for(auto p:g_carAlternateBodyScenes)assert(!p);for(auto p:g_carAlternateBodyBuffers)assert(!p);for(auto p:g_carModelBuffers)assert(!p);++cases;
 }
 for(unsigned seed=0;seed<16;seed++)for(int back:{-1,0,1,127})for(unsigned entering=0;entering<2;entering++){
  memset(g_controlsCopy,seed+5,sizeof(g_controlsCopy));memset(controllerTable,seed+9,sizeof(controllerTable));ControllerData expectedCopy[6],expectedTable[6];memcpy(expectedCopy,entering?controllerTable:g_controlsCopy,sizeof(expectedCopy));memcpy(expectedTable,!entering && !back?g_controlsCopy:controllerTable,sizeof(expectedTable));
  for(unsigned i=0;i<8;i++){mappings[i]=seed*7919+i*8251;g_controlsDeviceSelection.slots[i]=seed*71+i*193;}
  g_controlsDeviceSelection.lastDevice=0xcabd;unsigned short expectedSlots[8];memcpy(expectedSlots,entering?mappings:g_controlsDeviceSelection.slots,sizeof(expectedSlots));controllerCalls=0;
  if(entering)FrontendControls_EnterDeviceConfiguration(nullptr,back);else FrontendControls_LeaveDeviceConfiguration(nullptr,back);
  assert(controllerCalls==(entering?11:!back?9:0) && !memcmp(g_controlsCopy,expectedCopy,sizeof(expectedCopy)) && !memcmp(controllerTable,expectedTable,sizeof(expectedTable)) && !memcmp(expectedSlots,g_controlsDeviceSelection.slots,sizeof(expectedSlots)) && g_controlsDeviceSelection.lastDevice==0xcabd);++cases;
 }
 printf("PASS: %u actual native address walks; pointers >4 GB, native pointer-table strides, full record guards, null/mixed slots, negative counts, provider identities; complete first-model-failure stage path; ASan/UBSan\n",cases);
}
'''

def main():
 sources={n:(ROOT/'CMR2Decomp'/n).read_text(encoding='latin1') for n in ('StageTiming.cpp','Sound.cpp','NetPlayers.cpp','RallyTiming.cpp','StageObjects.cpp','FrontendMenus.cpp','Input.h')}
 code=PRELUDE
 code+=record(sources['Input.h'],'ControllerDataUnk0x210')+record(sources['Input.h'],'ControllerData')+record(sources['FrontendMenus.cpp'],'ControlsDeviceSelection')
 code+='''ControlsDeviceSelection g_controlsDeviceSelection;ControllerData g_controlsCopy[6],controllerTable[6];
#define g_unk0x0082a7c8 (g_controlsDeviceSelection.slots)
ControllerData *Input_GetControllerTable(){++controllerCalls;return controllerTable;}
unsigned short Input_GetControllerSlotMapping(unsigned short i){assert(i<8);++controllerCalls;return mappings[i];}
void Input_SetControllerSlotMapping(unsigned short i,unsigned short v){assert(i<8 && v==g_controlsDeviceSelection.slots[i]);++controllerCalls;}
struct CInput{static void RefreshControllerConfigurations(){++controllerCalls;}};
void FrontendControls_FillDeviceConfiguration(){++controllerCalls;}
'''
 # Leaves on the complete early failure path are counted; every later-path
 # provider rejects accidental execution. Actual record/header layouts remain.
 stage=body(sources['StageTiming.cpp'],'StageTiming_LoadStageCarsAndEffects')
 early={'StageTiming_GetTotalCarCount','Car_AllocateTable','StageTiming_AllocateCarReplayRecordTables','StageTiming_AllocateCarPartNodeTables','StageTiming_AllocateMotionRecordTable','StageTiming_AllocateViewWeatherRecords','Car_ClearRecords','Car_ClearWheelRotation','Physics_SetScale','StageObject_SetPhysicsScaleAndReciprocal','Car_ResetPlaybackSpeed','RallyData_GetFlag22','RallyData_GetFlag31','NetPlayers_ClearPlayerFlag23','StageTiming_LoadSelectedCarModel'}
 calls=set(re.findall(r'\b(\w+(?:::\w+)?)\(',stage))-{'sizeof','offsetof','if','while','for','memset','StageTiming_LoadStageCarsAndEffects'}
 for name in sorted(calls):
  if '::' in name:continue
  result='Car*' if name=='Car_Get' else 'short*' if name=='Car_GetOrder' else 'SceneNode*' if name=='SceneNode_FindByType' else 'int'
  effect='++stageCalls;' if name in early else 'assert(false);'
  code+=f'template<class...A> {result} {name}(A...){{{effect}return {1 if name=="StageTiming_GetTotalCarCount" else 0};}}\n'
 code+='int StageTiming_FreeSceneAndFinishResources(){assert(false);return 0;}\n'
 for file,names in {'StageTiming.cpp':['Replay_ReleaseStageBuffers','StageTiming_FreeCarPartNodeTables'], 'Sound.cpp':['Sound_SetMasterVolume'],'NetPlayers.cpp':['NetPlayers_ClearReadyFlags','NetPlayers_BlockStatisticsReception'],'RallyTiming.cpp':['RallyTiming_ResetStageResults','RallyTiming_ResetOverallPlayerTimes'],'StageObjects.cpp':['StageObject_ResetPairedCarValues'],'FrontendMenus.cpp':['FrontendControls_LeaveDeviceConfiguration','FrontendControls_EnterDeviceConfiguration']}.items():
  for name in names:code+=actual(sources[file],name)+'\n'
 code+=stage+MAIN
 with tempfile.TemporaryDirectory(prefix='cmr2-native-walks-') as d:
  p=Path(d);(p/'walks.cpp').write_text(code);(p/'windows.h').write_text('#include <cstdint>\nusing BYTE=unsigned char;using BOOL=int;using DWORD=uint32_t;\n')
  for name in ('StageTiming.h','StageWeatherParticle.h','NetPlayers.h','RallyTiming.h','LayoutChecks.h','CarResources.h'):
   text=(ROOT/'CMR2Decomp'/name).read_text().replace('#include "FixedPoint.h"','')
   (p/name).write_text(text)
  subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-fsanitize=address,undefined','-I'+d,str(p/'walks.cpp'),'-o',str(p/'walks')],check=True)
  r=subprocess.run([str(p/'walks')],capture_output=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
  assert r.returncode==0,r.stderr.decode();print(r.stdout.decode(),end='')
if __name__=='__main__':main()
