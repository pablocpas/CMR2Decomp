#!/usr/bin/env python3
"""Actual changed runtime bodies with production native records and high pointers.
ASan/UBSan; guards and independent field/sample models; five negative controls.
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from check_64bit_car_contacts import body
ROOT=Path(__file__).resolve().parents[1]
PORT=ROOT.parent/'OpenCMR2'
PRELUDE=r'''
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include "port/types.h"
#include "port/net.h"
#include "Car.h"
#include "SceneNode.h"
#include "ReplayStream.h"
typedef NetSessionDesc DPSESSIONDESC2;
NetSessionDesc g_networkSession;
ReplayStream *g_activeReplayStream;
BYTE *g_unk0x00537f3c[8];void **g_replaySlots[16];Car car;
class CGameInfo {public:
 static NetSessionDesc m_enumeratedSessions[20];static char m_unk0x005a00b8[264],m_unk0x005a02c0[264];
 static BOOL m_unk0x005a0060;static HRESULT m_unk0x005a1814;static BYTE m_unk0x005a01bc;
 static void ResetNetworkSessionState();static int EnumerateNetworkSessionsWithUserData(char *pPassword);
};
NetSessionDesc CGameInfo::m_enumeratedSessions[20];char CGameInfo::m_unk0x005a00b8[264],CGameInfo::m_unk0x005a02c0[264];
BOOL CGameInfo::m_unk0x005a0060;HRESULT CGameInfo::m_unk0x005a1814;BYTE CGameInfo::m_unk0x005a01bc;
class CGame {public:static void *GetDirectPlay(){return (void *)1;}};
int g_unk0x00511cd8[4]={1,2,3,4},enumCalls;char *expectedPassword;
BOOL Session_EnumCallback(const NetSessionDesc *,void *){return TRUE;}
HRESULT Net_EnumSessions(const NetGuid *application,DWORD timeout,NetSessionCallback callback,void *context){
 assert(!memcmp(application,g_unk0x00511cd8,16)&&timeout==1000&&callback==Session_EnumCallback&&!context);
 assert(g_networkSession.dwSize==sizeof(NetSessionDesc)&&g_networkSession.lpszPasswordA==expectedPassword);
 assert(g_networkSession.lpszSessionNameA==CGameInfo::m_unk0x005a00b8);enumCalls++;return NET_OK;
}
int reads,pushes,alphas,colours,encoded;ReplaySample *expectedSample;
BOOL Network_ReadJoinedSessionDescription(){reads++;return TRUE;}
bool Network_TestLocalPlayerStatus(){pushes++;return true;}
Car *Car_Get(int index){assert(index==1);return &car;}
void Mesh_SetVertexAlpha(Mesh *,BYTE a){assert(a==100);alphas++;}
void Mesh_SetVertexColours(Mesh *,BYTE *rgb){assert(rgb[0]==128&&rgb[1]==128&&rgb[2]==128&&rgb[3]==0);colours++;}
void Replay_EncodeCarPoseSample(Car *p,ReplaySample *sample){assert(p==&car&&sample==expectedSample);memset(sample,0x3c,sizeof(*sample));encoded++;}
int Replay_StopRecording(BYTE *pBuffer){ReplayStream *p=(ReplayStream *)pBuffer;if(!p||!p->recording)return 0;p->recording=0;p->laneCount=1;p->recordStarted=0;p->field_0x18=0;return 1;}
'''
MAIN=r'''
int main(){
 assert(sizeof(void *)==8);static_assert(sizeof(ReplayStream)==0x138,"native stream");
 static_assert(sizeof(NetSessionDesc)==0x58,"native session");
 char *name=(char *)malloc(64),*password=(char *)malloc(64);assert((uintptr_t)name>UINT32_MAX&&(uintptr_t)password>UINT32_MAX);
 strcpy(name,"session");strcpy(password,"password");memset(&g_networkSession,0xa5,sizeof(g_networkSession));
 auto expected=g_networkSession;Session_SetName(name);expected.lpszSessionNameA=name;assert(!memcmp(&expected,&g_networkSession,sizeof(expected)));
 Session_SetPassword(password);expected.lpszPasswordA=password;assert(!memcmp(&expected,&g_networkSession,sizeof(expected)));
 for(int index: {0,1,2,3,255})for(int value:{INT32_MIN,-1,0,1,INT32_MAX}){
  auto before=g_networkSession;Session_SetUserValue((BYTE)index,value);
  switch(index){case 0:before.dwUser1=value;break;case 1:before.dwUser2=value;break;case 2:before.dwUser3=value;break;default:before.dwUser4=value;}
  assert(!memcmp(&before,&g_networkSession,sizeof(before)));assert(Session_GetUserValue((BYTE)index)==value);
 }
 Session_SetMaxPlayers(8);assert(g_networkSession.dwMaxPlayers==8);Session_SetOpen(0);assert(g_networkSession.dwFlags&32);Session_SetOpen(1);assert(!(g_networkSession.dwFlags&32));
 assert(reads==pushes+25&&g_networkSession.lpszSessionNameA==name&&g_networkSession.lpszPasswordA==password);
 expectedPassword=password;memset(CGameInfo::m_enumeratedSessions,0xa5,sizeof(CGameInfo::m_enumeratedSessions));
 CGameInfo::m_unk0x005a1814=0;assert(CGameInfo::EnumerateNetworkSessionsWithUserData(password)==1&&enumCalls==1);
 BYTE empty[sizeof(CGameInfo::m_enumeratedSessions)]={};assert(!memcmp(empty,CGameInfo::m_enumeratedSessions,sizeof(empty)));
 auto descriptor=g_networkSession;CGameInfo::m_unk0x005a1814=1;
 assert(CGameInfo::EnumerateNetworkSessionsWithUserData(NULL)==0&&enumCalls==1&&!memcmp(&descriptor,&g_networkSession,sizeof(descriptor)));
 auto *mesh=(Mesh *)calloc(1,sizeof(Mesh));auto *tri=(MeshTriangle *)calloc(1,sizeof(MeshTriangle));mesh->pTriangles=tri;mesh->flags=0x80;
 SceneNode node={};node.pObject=mesh;node.type=0;StageTiming_SetNodeMeshAlpha(&node,100,1);
 assert(mesh->pTriangles==tri&&mesh->flags==0x40088&&alphas==1&&colours==1);
 node.type=3;StageTiming_SetNodeMeshAlpha(&node,100,1);node.type=0;node.pObject=NULL;StageTiming_SetNodeMeshAlpha(&node,100,1);assert(alphas==1&&colours==1);
 for(int playing:{-1,0,1}){
  struct {BYTE guard[16];ReplayStream stream;BYTE tail[16];} block;
  memset(&block,0xa5,sizeof(block));block.stream.steeringModeOption=0xabcdef01;block.stream.playing=playing;auto before=block;
  int result=Replay_ResetBufferIfActive((int *)&block.stream);
  if(playing){before.stream.playing=0;before.stream.playStarted=0;before.stream.field_0x14=0;}
  assert(result==bool(playing)&&!memcmp(&before,&block,sizeof(block)));
  g_activeReplayStream=&block.stream;assert(Replay_GetActiveBufferState()==0);
  g_unk0x00537f3c[1]=(BYTE *)&block.stream;block.stream.playing=1;block.stream.recording=-1;
  assert(Race_GetPlayerRecordField4(1)==1&&Race_GetPlayerRecordFieldC(1)==-1);
 }
 for(int phase=0;phase<3;phase++)for(int count=0;count<=2;count++){
  const int capacity=2;size_t payload=2*sizeof(ReplayStateLane)+2*capacity*sizeof(ReplaySample)+2*sizeof(short);
  BYTE *allocation=(BYTE *)malloc(16+sizeof(ReplayStream)+payload+16);assert((uintptr_t)allocation>UINT32_MAX);memset(allocation,0xa5,16+sizeof(ReplayStream)+payload+16);
  auto *p=(ReplayStream *)(allocation+16);memset(p,0,sizeof(*p));p->type=2;p->car=1;p->recording=1;p->recordStarted=1;p->step=phase;p->laneCount=1;p->samplesPerLane=capacity;
  p->pSamples=(BYTE *)p+sizeof(*p)+2*sizeof(ReplayStateLane);p->pLaneSamples=(short *)(p->pSamples+2*capacity*sizeof(ReplaySample));p->pLaneSamples[1]=count;
  void *slot=p;memset(g_replaySlots,0,sizeof(g_replaySlots));g_replaySlots[0]=&slot;car.simulationStepsRemaining=1;expectedSample=(ReplaySample *)p->pSamples+capacity+count;
  BYTE *before=(BYTE *)malloc(16+sizeof(*p)+payload+16);memcpy(before,allocation,16+sizeof(*p)+payload+16);encoded=0;
  Replay_RecordPeriodicCarSamples();assert(encoded==(phase==0&&count<capacity));assert(p->pLaneSamples[1]==count+(phase==0&&count<capacity));assert(p->step==(phase<2?phase+1:0));assert(p->recording==!(phase==0&&count==capacity));
  for(int i=0;i<16;i++){assert(allocation[i]==0xa5&&allocation[16+sizeof(*p)+payload+i]==0xa5);}
  auto *model=(ReplayStream *)(before+16);model->step=phase<2?phase+1:0;
  if(phase==0){
   if(count<capacity){
    memset(before+((BYTE *)expectedSample-allocation),0x3c,sizeof(ReplaySample));
    *(short *)(before+((BYTE *)&p->pLaneSamples[1]-allocation))=count+1;
   }else{model->recording=0;model->recordStarted=0;model->field_0x18=0;model->laneCount=1;}
  }
  assert(!memcmp(before,allocation,16+sizeof(*p)+payload+16));
  free(before);free(allocation);
 }
 free(name);free(password);free(mesh);free(tri);puts("PASS native session, ghost flags, replay queries/reset and pose recording with high pointers");
}
'''
def main():
 src=ROOT/'CMR2Decomp';game=(src/'Game.cpp').read_text();stage=(src/'StageObjects.cpp').read_text();timing=(src/'StageTiming.cpp').read_text();race=(src/'Race.cpp').read_text()
 functions=[body(game,n) for n in ('Session_SetName','Session_SetPassword','Session_SetUserValue','Session_GetUserValue','Session_SetOpen')]
 # 'char' return is outside the generic extractor's return set.
 functions.append(body(game.replace('char Session_SetMaxPlayers','int Session_SetMaxPlayers'),'Session_SetMaxPlayers'))
 functions+=[body(timing,'StageTiming_SetNodeMeshAlpha'),body(timing,'Replay_RecordCarPoseSample')]
 functions+=[body(stage,n) for n in ('Replay_ResetBufferIfActive','Replay_GetActiveBufferState','Replay_RecordPeriodicCarSamples')]
 functions+=[body(race,n) for n in ('Race_GetPlayerRecordField4','Race_GetPlayerRecordFieldC')]
 info=(PORT/'game/GameInfo.cpp').read_text();functions+=[body(info,n) for n in ('CGameInfo::ResetNetworkSessionState','CGameInfo::EnumerateNetworkSessionsWithUserData')]
 code=PRELUDE+'\n#include <initializer_list>\n'+'\n'.join(functions)+MAIN
 with tempfile.TemporaryDirectory(prefix='cmr2-native-crash-records-') as d:
  d=Path(d)
  for name in ('Car.h','SceneNode.h','Sector.h','Mesh.h','ReplayStream.h','LayoutChecks.h'):
   shutil.copyfile(src/name,d/name)
  shutil.copyfile(PORT/'game/FixedPoint.h',d/'FixedPoint.h')
  (d/'Graphics.h').write_text('class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0;};\n')
  variants={'good':code,'ghost-offset':code.replace('pMesh->flags |= 0x40008;','*(int *)((char *)pMesh+0x30)|=0x40008;'),
   'replay-reset':code.replace('stream->playing = 0;','stream->recording = 0;'),
   'password-offset':code.replace('g_networkSession.lpszPasswordA = (LPSTR)pPassword;','*(void **)((BYTE *)&g_networkSession+0x34)=pPassword;'),
   'browser-password-truncation':code.replace('g_networkSession.lpszPasswordA = pPassword;','g_networkSession.lpszPasswordA = (char *)(uintptr_t)(unsigned)(uintptr_t)pPassword;'),
   'sample-truncation':code.replace('Replay_RecordCarPoseSample((ReplaySample *)p->pSamples + p->samplesPerLane * p->laneCount + n, p->car);','Replay_RecordCarPoseSample((ReplaySample *)(uintptr_t)(unsigned)((uintptr_t)p->pSamples+16*(p->samplesPerLane*p->laneCount+n)), p->car);')}
  for name,contents in variants.items():
   source=d/(name+'.cpp');source.write_text(contents);exe=d/name
   subprocess.run(['clang++','-std=c++17','-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fms-extensions','-Wno-everything','-I'+str(d),'-I'+str(PORT/'src'),str(source),'-o',str(exe)],check=True)
   run=subprocess.run([str(exe)],capture_output=True,text=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'))
   if name=='good':assert run.returncode==0,run.stderr;print(run.stdout.strip())
   else:assert run.returncode!=0,'negative control passed: '+name;print('PASS negative control detected:',name)
 return 0
if __name__=='__main__':raise SystemExit(main())
