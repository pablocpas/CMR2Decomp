#!/usr/bin/env python3
"""Actual callback-machine storage, lifecycle and clients on native x64."""
from pathlib import Path
import os,re,shutil,subprocess,tempfile
from check_64bit_car_contacts import ROOT

def body(source,name):
 match=re.search(r'^[^\n]*?\b' + re.escape(name) + r'\([^;]*?\)\s*\{',source,re.M)
 assert match,name
 pos,depth=match.end(),1
 while depth:
  depth+=(source[pos]=='{')-(source[pos]=='}');pos+=1
 return source[match.start():pos]

PRELUDE=r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <initializer_list>
#include <cstdio>
#include <cstring>
#include "RaceCallbackStorage.h"
class CGame {
public:
 static int m_skipRenderCallbacks;
 static CallbackIndex m_callbackIndex;
 static CallbackStateRecord *m_currentCallbackRecord;
 static void InitializeCallbackStateRecord(CallbackStateRecord *,int,int);
 static void InitializeCallbackStateMachine(CallbackStateMachine *,BYTE,CallbackStateRecord *,StateCallbackPair *,unsigned int *);
 static void RunStateUpdateCallbacks(CallbackStateMachine *);
 static void RunStateRenderCallbacks(CallbackStateMachine *);
 static void AdvanceCallbackStateTimers(CallbackStateMachine *);
 static BOOL UpdateInRaceCallbackMachine();
 static void SaveRaceCallbackDepth(){}
 static int PromoteCallbackEntryByRule(CallbackStateMachine *,BYTE,BYTE,int);
};
int CGame::m_skipRenderCallbacks;
CallbackIndex CGame::m_callbackIndex;
CallbackStateRecord *CGame::m_currentCallbackRecord;
struct CGameInfo {
 static int GetGameModeOptionBit19(){return 0;}
 static int GetConfiguredPlayerCount();static int GetConfiguredGameMode();static int IsConfiguredMultiplayer();static int IsInRaceMenuOpen();
};
struct CInput{template<class... T>static void SetInputRepeatTimingParameters(T...) {}};
struct CFrontend{static void ClearFrontendPlayerInputCounters(){}};
struct CMain{static int GetFrameDelta();};
RaceCallbackStorage g_raceCallbackStorage;
StateCallbackPair g_raceStateCallbacks[16];unsigned int g_raceStateRules[2]={0xffffffff,0xaaaaaaaa};
BYTE g_unk0x00537ef4,g_unk0x00537ef5;int g_unk0x00537ef8,g_unk0x00537efc,g_unk0x00537df0;
int g_unk0x0082a938;unsigned int g_unk0x0082b0a4;
static int entrants,mode,menu,available,queries,clock_value,promotions;
static unsigned int championship;static BYTE selection;
ptrdiff_t g_unk0x00537f0c[6];
BYTE g_unk0x00537fd4,g_unk0x00537ffa,g_unk0x0053810d;
int g_unk0x00538100,g_unk0x0053810c,g_unk0x00538108,g_unk0x00537f94,g_raceResourcesFreed;
void NetRace_SetRaceControlFlag(BYTE){}void NetPlayers_ResetStatisticsSequences(){}
void NetPlayers_RebuildStageResults(int,int,int){}void Menu_SetInputStateFlag(int){}
void Knockout_ClearRoundActiveFlag(){}int RallyDataStageIndex(){return 0;}int RallyDataCountryIndex(){return 0;}
void RallyTiming_ResetStageResults(){}void RallyTiming_ResetOverallPlayerTimes(){}void RallyTiming_ResetDriverSplitState(){}void RallyTiming_RebuildChampionshipPositionMap(){}
int RallyData_IsFirstAvailableStageSelected(){return 0;}int RallyData_GetSelectionBits12To13(){return 0;}
void NetRace_InitializeRaceSoundVolumes(){}void Race_BuildSelectionPaths(){}void StageTiming_OpenRegionalStageArchives(){}
void Surface_LoadStageFonts(){}void SurfaceText_LoadRegionalStringTable(){}void RallyData_InitializeRaceRendering(){}void StageObject_BuildInRaceActionMenu(){}
static CallbackStateMachine *lastMachine;
static int updates,renders,menuFrames,advances;
static BYTE updateIndices[4],renderIndices[4];
int CGameInfo::GetConfiguredPlayerCount(){return entrants;}
int CGameInfo::GetConfiguredGameMode(){return mode;}
int CGameInfo::IsConfiguredMultiplayer(){return 0;}
int CGameInfo::IsInRaceMenuOpen(){return menu;}
int CMain::GetFrameDelta(){return clock_value;}
unsigned int *RallyData_GetChampionshipState(){return &championship;}
void StageUI_ResetRaceEndEventCount(){}
int RallyData_IsChampionshipFinalStage(){return 0;}
char RallyData_GetUsableRecordCategory(BYTE){return -1;}
void RallyData_SetSelectionBits14To15(BYTE value){selection=value;}
void Car_BuildRaceOrder(int){}void Dash_Reset(){}
void InRaceMenu_UpdateStateMachineFrame(CallbackStateMachine *m){assert(m==&g_raceCallbackStorage.machine);menuFrames++;}
void InRaceMenu_AdvanceScreenEntries(BYTE *m){assert(m==(BYTE *)&g_raceCallbackStorage.machine);advances++;}
int RallyDataState(){return entrants;}
int View_IsModeAvailable(BYTE index,int requested){assert(index<2 && requested==2);queries++;return available;}
int CGame::PromoteCallbackEntryByRule(CallbackStateMachine *m,BYTE index,BYTE value,int level){assert(m==&g_raceCallbackStorage.machine && index<=2 && level==2);lastMachine=m;promotions++;return value;}
void update(CallbackStateMachine *m,BYTE index){assert(m==&g_raceCallbackStorage.machine && index<m->count);updateIndices[updates++]=index;}
void render(CallbackStateMachine *m,BYTE index){assert(m==&g_raceCallbackStorage.machine && index<m->count);renderIndices[renders++]=index;}
'''
MAIN=r'''
int main(){
 static_assert(sizeof(void *)==8,"native ABI");
 static_assert(sizeof(CallbackStateRecord)==8 && sizeof(StateCallbackPair)==16,"native records and callback pointers");
 static_assert(offsetof(CallbackStateMachine,records)==8 && offsetof(CallbackStateMachine,callbacks)==16 && offsetof(CallbackStateMachine,rules)==24,"native machine fields");
 static_assert(sizeof(CallbackStateMachine)==32 && offsetof(RaceCallbackStorage,records)==32 && sizeof(RaceCallbackStorage)==48,"owned storage grows with its pointers");
 assert((uintptr_t)&g_raceCallbackStorage>UINT32_MAX && (uintptr_t)g_raceStateCallbacks>UINT32_MAX && (uintptr_t)&update>UINT32_MAX);
 int cases=0;
 for(int count=0;count<=2;count++)for(int initial=0;initial<16;initial++)for(int pending=0;pending<4;pending++){
  memset(&g_raceCallbackStorage,0xcc,sizeof(g_raceCallbackStorage));
  for(auto &pair:g_raceStateCallbacks)pair={update,render};
  for(int i=0;i<2;i++)g_raceCallbackStorage.records[i]={(int)(0xa8000000u | (unsigned(pending)<<24) | (unsigned((initial+5+i)%16)<<16) | unsigned((initial+i)%16)),99+i};
  auto original=g_raceCallbackStorage;
  CGame::InitializeCallbackStateMachine(&g_raceCallbackStorage.machine,count,g_raceCallbackStorage.records,g_raceStateCallbacks,g_raceStateRules);
  assert(g_raceCallbackStorage.machine.records==g_raceCallbackStorage.records && g_raceCallbackStorage.machine.callbacks==g_raceStateCallbacks && g_raceCallbackStorage.machine.rules==g_raceStateRules);
  assert(!memcmp(g_raceCallbackStorage.records,original.records,sizeof(original.records)));
  assert(!memcmp(g_raceCallbackStorage.machine.pad,original.machine.pad,3));
  assert(StageUI_GetRaceResultTable()==(BYTE *)&g_raceCallbackStorage.machine);
  updates=renders=0;CGame::m_skipRenderCallbacks=0;CGame::RunStateUpdateCallbacks(&g_raceCallbackStorage.machine);CGame::RunStateRenderCallbacks(&g_raceCallbackStorage.machine);
  assert(updates==count && renders==count);for(int i=0;i<count;i++)assert(updateIndices[i]==i && renderIndices[i]==i);
  CGame::m_skipRenderCallbacks=1;CGame::RunStateRenderCallbacks(&g_raceCallbackStorage.machine);assert(renders==count && CGame::m_skipRenderCallbacks==0);
  CGame::AdvanceCallbackStateTimers(&g_raceCallbackStorage.machine);
  for(int i=0;i<2;i++){
   unsigned before=original.records[i].packedState;unsigned expected=before;int ticks=original.records[i].elapsedTicks;
   if(i<count){if(pending){expected=((before&0xffffff00u)|((before>>16)&255))&0xfc00ffffu;ticks=0;}else ticks++;}
   assert((unsigned)g_raceCallbackStorage.records[i].packedState==expected && g_raceCallbackStorage.records[i].elapsedTicks==ticks);
  }
  cases++;
 }
 for(int count=1;count<=2;count++)for(int late=0;late<=1;late++)for(int seed=0;seed<16;seed++){
  memset(&g_raceCallbackStorage,0xcc^seed,sizeof(g_raceCallbackStorage));auto before=g_raceCallbackStorage;
  entrants=count;mode=0;g_unk0x00537ef4=g_unk0x00537ef5=0;g_unk0x00537ef8=late;g_unk0x00537efc=0;
  CGame::UpdateInRaceCallbackMachine();
  int expectedCount=count>1?2:1,level=count>1?1:0;
  assert(selection==expectedCount && g_unk0x00537df0==level && g_unk0x00537efc==late);
  assert(g_raceCallbackStorage.machine.count==expectedCount && g_raceCallbackStorage.machine.records==g_raceCallbackStorage.records);
  for(int i=0;i<2;i++)if(i<expectedCount)assert((unsigned)g_raceCallbackStorage.records[i].packedState==((unsigned(before.records[i].packedState)&0xfc000000u) | (unsigned(level)<<8) | unsigned(late?5:0)) && g_raceCallbackStorage.records[i].elapsedTicks==0);else assert(!memcmp(&g_raceCallbackStorage.records[i],&before.records[i],8));
  assert(!memcmp(g_raceCallbackStorage.machine.pad,before.machine.pad,3));cases++;
 }
 for(int count=1;count<=2;count++)for(int state: {7,8,9,10,13,255})for(int permit=0;permit<=1;permit++){
  entrants=count;available=permit;g_raceCallbackStorage.machine.count=count;g_raceCallbackStorage.machine.records=g_raceCallbackStorage.records;
  for(int i=0;i<2;i++)g_raceCallbackStorage.records[i]={(int)(0xab123400u|unsigned(state+i)),0};
  auto before=g_raceCallbackStorage;
  assert(Race_IsMultiplayerRecordMode10()==(count>1 && state==10));
  for(int i=0;i<count;i++){queries=0;assert(RallyData_IsDriverViewModeAllowed(i,2)==(state+i!=10 && permit));assert(queries==(state+i!=10));}
  assert(!memcmp(&g_raceCallbackStorage,&before,sizeof(before)));cases++;
 }
 for(unsigned delta: {0u,50u,51u,0xffffffffu})for(int flagged=0;flagged<=1;flagged++){
  g_unk0x0082a938=flagged;g_unk0x0082b0a4=100;clock_value=(int)(100u+delta);promotions=0;lastMachine=nullptr;
  OptionMenu_StartDelayedFade(&g_raceCallbackStorage.machine,0);
  assert(promotions==(delta>50));if(promotions)assert(lastMachine==&g_raceCallbackStorage.machine);cases++;
 }
 for(int event=1;event<=2;event++){
  for(int i=0;i<6;i++)g_unk0x00537f0c[i]=i-11;
  auto machine=&g_raceCallbackStorage.machine;promotions=0;
  Race_EnterCurrentModeScene(machine,event);
  assert((CallbackStateMachine *)g_unk0x00537f0c[5]==machine);
  for(int i=0;i<5;i++)assert(Race_ReadPlayerState37F0C(i)==i-11 && g_unk0x00537f0c[i]==i-11);
  assert(promotions==1);cases++;
 }
 printf("PASS: %d actual x64 storage/lifecycle/query/fade scenarios, native production headers, pointers/callbacks above 4 GB, whole records and padding guarded, ASan/UBSan\n",cases);
}
'''
def main():
 source=ROOT/'CMR2Decomp'
 names=('CGame::InitializeCallbackStateRecord','CGame::InitializeCallbackStateMachine','CGame::RunStateUpdateCallbacks','CGame::RunStateRenderCallbacks','CGame::AdvanceCallbackStateTimers','CGame::UpdateInRaceCallbackMachine')
 funcs='\n'.join(body((source/'Game.cpp').read_text(),name) for name in names)
 for file,name in (('StageUI.cpp','StageUI_GetRaceResultTable'),('Race.cpp','Race_IsMultiplayerRecordMode10'),('RallyData.cpp','RallyData_IsDriverViewModeAllowed'),('GameInfo.cpp','OptionMenu_StartDelayedFade'),('Race.cpp','Race_EnterCurrentModeScene'),('Race.cpp','Race_ReadPlayerState37F0C')):funcs+='\n'+body((source/file).read_text(),name)
 with tempfile.TemporaryDirectory(prefix='cmr2-native-race-state-') as directory:
  tmp=Path(directory)
  for header in ('CallbackState.h','RaceCallbackStorage.h'):shutil.copyfile(source/header,tmp/header)
  (tmp/'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;\n#define FALSE 0\n#define TRUE 1\n')
  fixture=tmp/'race.cpp';fixture.write_text(PRELUDE+'\nBYTE *StageUI_GetRaceResultTable();\n'+funcs+MAIN)
  exe=tmp/'race'
  subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-I',str(tmp),'-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast','-fsanitize=address,undefined',str(fixture),'-o',str(exe)],check=True)
  subprocess.run([str(exe)],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
if __name__=='__main__':main()
