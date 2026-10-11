#!/usr/bin/env python3
"""Actual native particle draw, spawn, initializer, setter and route restore.

Uses current Car/SceneNode/Particle/RaceRecord layouts and the port's portable
fixed-point primitives. Draw callbacks, billboard submission, rand and route
reset leaves are controlled. Spawn fixtures cover pool selection, attachment,
colour/shade and init-data paths; basis/range randomization is outside this test.
"""
import os,re,shutil,subprocess,tempfile
from pathlib import Path
from check_64bit_car_contacts import ROOT,body
from check_64bit_address_loops import record
PRELUDE=r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <initializer_list>
using BYTE=unsigned char;using DWORD=uint32_t;using BOOL=int;using INT_PTR=intptr_t;using UINT_PTR=uintptr_t;using __int64=long long;
#include "Car.h"
#include "Particle.h"
#include "RaceRecord.h"
unsigned short g_sqrtTable[4096];
struct Texture{int identity;};Texture textures[32];
Particle pool[8];ParticleType types[8];FixMatrix matrices[8];Texture *frames[8][3];SceneNode camera;Car owner;
Particle *g_particles=pool;ParticleType *g_particleTypes=types,*g_pEditParticleType;int g_particleCount,g_particleTypeCount=8,g_nextParticle;
float g_oneOverRandMax=1.0f/32767;double g_minus65536=-65536;double CGraphics::m_65536=65536;
unsigned randomValue,randomCalls,routeCalls;int particleRandom(){++randomCalls;return randomValue;}
#define rand particleRandom
RaceRecord g_raceRecords[8];void RallyData_ResetPlayerRouteProbes(){++routeCalls;}
'''
LEAVES=r'''
struct Expected{bool custom;Particle *p;ParticleType *t;SceneNode *camera;BillboardDef def;Texture *texture;};std::vector<Expected> expected;unsigned outputIndex;
void customDraw(Particle *p,ParticleType *t,SceneNode *cam){assert(outputIndex<expected.size());auto &e=expected[outputIndex++];assert(e.custom && e.p==p && e.t==t && e.camera==cam && (uintptr_t)p>UINT32_MAX && (uintptr_t)cam>UINT32_MAX);}
void Billboard_Add(BillboardDef *def,Texture *texture){assert(outputIndex<expected.size());auto &e=expected[outputIndex++];assert(!e.custom && texture==e.texture);BYTE actual[36],want[36];memcpy(actual,def,36);memcpy(want,&e.def,36);actual[35]&=3;want[35]&=3;assert(!memcmp(actual,want,36));}
void unusedUpdate(void*,ParticleType*,int){}
void unusedInit(void*,ParticleType*,void*){}
int scalarMul(int a,int b){return (int)((int64_t)a*b>>16);}
'''
MAIN=r'''
int main(){
 static_assert(sizeof(void*)==8 && sizeof(size_t)==8,"native address arithmetic");static_assert(sizeof(Particle)==128 && sizeof(ParticleType)==144,"runtime particles grow");static_assert(offsetof(Particle,attachmentMatrix)==72 && offsetof(Particle,texture)==112,"named native fields");
 assert((uintptr_t)pool>UINT32_MAX && (uintptr_t)types>UINT32_MAX && (uintptr_t)&camera>UINT32_MAX && (uintptr_t)customDraw>UINT32_MAX && (uintptr_t)&owner>UINT32_MAX);unsigned n=0;
 for(unsigned seed=0;seed<16;seed++)for(int count:{-1,0,1,8})for(unsigned view:{0u,1u,3u,7u,8u}){
  memset(pool,0xa3,sizeof(pool));memset(types,0xd1,sizeof(types));memset(matrices,0x81,sizeof(matrices));expected.clear();outputIndex=0;g_particleCount=count;
  for(unsigned i=0;i<8;i++){
   Particle &p=pool[i];ParticleType &t=types[i];p.pType=&t;p.active=(seed+i)%3!=0;p.field0x55=(seed*17+i*31)&255;t.draw=(seed+i)%5==0?customDraw:nullptr;t.frames=(seed+i)%4?frames[i]:nullptr;t.flags=(seed+i)%2?32:0;t.directionFlags=(seed+i)%16;t.lifetime=0x50000;int elapsedCases[]={-65536,0,65536,131072,393216,1048576};int elapsed=elapsedCases[(seed+i)%6];p.age=t.lifetime-elapsed;t.field0x54=65536;int steps[]={0,32768,65536};t.field0x58=steps[(seed+i)%3];t.field0x50=3;
   frames[i][0]=&textures[2*i];frames[i][1]=nullptr;frames[i][2]=&textures[2*i+1];p.texture=frames[i][(seed+i)%3];p.vector0x1c={(int)seed*11993-(int)i*331,(int)i*13191-(int)seed*71,(int)seed*53-(int)i*751};matrices[i].position={(int)i*913-(int)seed*711,(int)seed*331+(int)i*1791,(int)i*11233-(int)seed*391};p.attachmentMatrix=(seed+i)%2?&matrices[i]:nullptr;int sizes[]={0,16384,65536,98304};p.size=sizes[(seed+i)%4];t.field0x3c=-49152;t.field0x40=-65536;t.field0x44=81920;t.field0x48=98304;p.field0x50=(seed*193+i*71)&4095;p.colour[0]=seed*11+i*19;p.colour[1]=seed*23+i*3;p.colour[2]=seed*31+i*17;p.type0x53=seed*23+i*31;p.field0x54=seed*13+i*7;
   if(i>=unsigned(std::max(count,0)) || !p.active || !(p.field0x55&((1u<<view)&255)))continue;
   Expected e={};e.custom=t.draw;e.p=&p;e.t=&t;e.camera=&camera;if(e.custom){expected.push_back(e);continue;}
   e.texture=p.texture;if(t.frames){unsigned frame=0;if(elapsed>t.field0x54 && t.field0x58>0){frame=(elapsed-t.field0x54)/t.field0x58;if(t.directionFlags&8)frame+=((uintptr_t)&p&65535)%3;frame=frame>=3 && !(t.directionFlags&4)?2:frame%3;}e.texture=t.frames[frame];}if(!e.texture)continue;
   e.def.pos=p.vector0x1c;if(p.attachmentMatrix){e.def.pos.x+=matrices[i].position.x;e.def.pos.y+=matrices[i].position.y;e.def.pos.z+=matrices[i].position.z;}
   e.def.top=scalarMul(t.field0x3c,p.size);e.def.left=scalarMul(t.field0x40,p.size);e.def.bottom=scalarMul(t.field0x44,p.size);e.def.right=scalarMul(t.field0x48,p.size);e.def.field_0x20=t.flags&32?p.field0x50:0;e.def.r=p.colour[0];e.def.g=p.colour[1];e.def.b=p.colour[2];e.def.a=p.type0x53;e.def.shade=p.field0x54;e.def.flags=t.directionFlags&2;expected.push_back(e);
  }
  BYTE savedPool[sizeof(pool)],savedTypes[sizeof(types)],savedMatrices[sizeof(matrices)];memcpy(savedPool,pool,sizeof(pool));memcpy(savedTypes,types,sizeof(types));memcpy(savedMatrices,matrices,sizeof(matrices));Particle_DrawAll(&camera,view);
  assert(outputIndex==expected.size() && !memcmp(savedPool,pool,sizeof(pool)) && !memcmp(savedTypes,types,sizeof(types)) && !memcmp(savedMatrices,matrices,sizeof(matrices)));++n;
 }
 for(unsigned seed=0;seed<32;seed++){
  int data[2]={(int)seed*257-5001,(int)seed<<16};Particle expectedParticle;ParticleType type={};
  for(unsigned kind=0;kind<3;kind++){
   memset(pool,0xa7,sizeof(pool));memcpy(&expectedParticle,pool,sizeof(expectedParticle));randomValue=(seed*19937)&32767;randomCalls=0;owner.index=(char)seed;owner.field_0xa70=data[0];
   if(kind==0){expectedParticle.field0x64=(short)data[0];CarEffects_InitParticleOwnerData(pool,&type,data);}
   else if(kind==1){expectedParticle.field0x64=(short)((seed<<8)+randomValue%10);expectedParticle.size=data[0];GlassShard_Init(pool,&type,&owner);}
   else{expectedParticle.field0x64=(short)((randomValue%30<<8)+seed);expectedParticle.size=data[0];Debris_Init(pool,&type,data);}
   assert(!memcmp(pool,&expectedParticle,sizeof(expectedParticle)) && randomCalls==(kind!=0));for(unsigned i=sizeof(Particle);i<sizeof(pool);i++)assert(((BYTE*)pool)[i]==0xa7);++n;
  }
  for(unsigned present=0;present<2;present++){
   memset(types,0x95,sizeof(types));ParticleType saved=types[0];if(present){saved.draw=customDraw;saved.update=saved.postUpdate=unusedUpdate;saved.callback=unusedInit;saved.field0x6c=seed*911;}g_pEditParticleType=present?types:nullptr;
   ParticleEdit_SetCallbacks(customDraw,unusedUpdate,unusedUpdate,unusedInit,seed*911);assert(!memcmp(&saved,types,sizeof(saved)));for(unsigned i=sizeof(ParticleType);i<sizeof(types);i++)assert(((BYTE*)types)[i]==0x95);++n;
  }
  memset(g_raceRecords,seed+1,sizeof(g_raceRecords));RaceRecord savedRecords[8];memcpy(savedRecords,g_raceRecords,sizeof(savedRecords));for(auto &r:savedRecords){r.routeNodeIndex=r.resetRouteNodeIndex;r.field_0x8=r.field_0xc=0;r.field_0x14=0;}routeCalls=0;RallyData_RestoreAllCarRaceRecords();assert(routeCalls==1 && !memcmp(savedRecords,g_raceRecords,sizeof(savedRecords)));++n;
 }
 // Actual spawn with typed native attachment and init-data transport. Geometry
 // basis and random-size flags are off; all untouched record bytes are checked.
 for(unsigned mask:{0u,1u,7u})for(unsigned next=0;next<3;next++)for(unsigned attached=0;attached<2;attached++)for(unsigned init=0;init<2;init++)for(unsigned suppliedColour=0;suppliedColour<2;suppliedColour++)for(unsigned shade=0;shade<2;shade++){
  memset(pool,0xab,sizeof(pool));memset(types,0,sizeof(types));auto &t=types[0];t.flags=1;t.directionFlags=shade?2:0;t.lifetime=0x70000;t.size=0x19191;t.type=113;t.colour[0]=31;t.colour[1]=61;t.colour[2]=91;t.texture=textures;t.callback=init?CarEffects_InitParticleOwnerData:nullptr;
  for(unsigned i=0;i<3;i++){pool[i].active=mask&(1u<<i);pool[i].pType=&t;pool[i].age=0x10000+i*0x10000;}g_particleCount=3;g_nextParticle=next;int indexData=0x8212;FixVector source={12345,-9191,11231},velocity={17,-31,13};BYTE colour[3]={9,19,29};unsigned selected=next;int best=0;for(unsigned j=0;j<3;j++){unsigned i=(next+j)%3;if(!pool[i].active){selected=i;break;}int remaining=t.lifetime-pool[i].age;if(remaining>best){best=remaining;selected=i;}}
  Particle saved[8];memcpy(saved,pool,sizeof(pool));Particle &e=saved[selected];e.position=velocity;e.sourceVector=e.vector0x10=e.vector0x1c=e.vector0x28=source;e.field0x48=-0x40000;e.attachmentMatrix=attached?matrices:nullptr;e.pType=&t;e.age=t.lifetime;e.type0x56=e.type0x53=e.type0x52=t.type;e.size=t.size;memcpy(e.colour,suppliedColour?colour:t.colour,3);if(shade)e.field0x54=87;e.texture=t.texture;e.field0x55=93;e.active=1;e.field0x58=0;e.field0x50=0;if(init)e.field0x64=(short)indexData;randomValue=17;randomCalls=0;
  Particle_Spawn(0,&source,&velocity,-0x40000,attached?matrices:nullptr,suppliedColour?colour:nullptr,87,init?&indexData:nullptr,93);
  assert(!memcmp(saved,pool,sizeof(pool)) && randomCalls==3 && indexData==0x8212);++n;
 }
 printf("PASS: %u actual native particle/route scenarios; >4 GB callback/camera/matrix/init-data pointers, native 128/144-byte strides, full record guards, animation/visibility/custom paths and spawn pool selection, ASan/UBSan\n",n);
}
'''

def main():
 source=ROOT/'CMR2Decomp';graphics=(source/'Graphics.cpp').read_text();effects=(source/'CarEffects.cpp').read_text();rally=(source/'RallyData.cpp').read_text();code=PRELUDE+record((source/'Graphics.h').read_text(),'BillboardDef')+LEAVES
 for name in ('CarEffects_InitParticleOwnerData','GlassShard_Init','Debris_Init'):code+=body(effects,name)+'\n'
 code+=body(graphics,'ParticleEdit_SetCallbacks')+'\n'
 code+='#define DRAW_PARTICLE (*(Particle *)(pb - offsetof(Particle, field0x55)))\n'+body(graphics,'Particle_DrawAll')+'\n#undef DRAW_PARTICLE\n'
 code+=body(graphics,'Particle_Spawn')+'\n'+body(rally,'RallyData_RestoreAllCarRaceRecords')+MAIN
 with tempfile.TemporaryDirectory(prefix='cmr2-native-particle-pointers-') as d:
  p=Path(d)
  for name in ('FloatToInt.h','Car.h','SceneNode.h','Sector.h','LayoutChecks.h','Particle.h','RaceRecord.h'):shutil.copyfile(source/name,p/name)
  shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',p/'FixedPoint.h')
  (p/'Mesh.h').write_text('struct Mesh;\n')
  (p/'Graphics.h').write_text('struct Texture;class CGraphics{public:static double m_65536;static const double m_oneOver65536;};\n')
  (p/'windows.h').write_text('#include <cstdint>\nusing BYTE=unsigned char;using DWORD=uint32_t;\n')
  (p/'particles.cpp').write_text(code)
  subprocess.run(['clang++','-include',str(ROOT/'CMR2Decomp/FloatToInt.h'),'-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-fsanitize=address,undefined','-I'+d,str(p/'particles.cpp'),'-o',str(p/'particles')],check=True)
  r=subprocess.run([str(p/'particles')],capture_output=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
  assert not r.returncode,r.stderr.decode();print(r.stdout.decode(),end='')
if __name__=='__main__':main()
