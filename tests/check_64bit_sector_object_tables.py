#!/usr/bin/env python3
"""Compile actual table/mesh/moving consumers with native pointers under ASan/UBSan.

Uses production headers and bodies, plus the port's existing fixed-point leaves.
Fixture-only decoding of the original disk records is explicit; it does not
claim the game's in-place disk loader is ready for 64 bits. Native allocation
sizes, full pointer values, canonical records and immutable storage are checked
against the original x86 lifecycle, including the 40-entry eviction path.
"""
import itertools,json,os,shutil,subprocess,tempfile
from pathlib import Path
from differential_sector_object_tables import Tables,ROOT,HEAP
from check_64bit_race_state_pointers import body

PRELUDE=r'''
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
typedef unsigned char BYTE;typedef unsigned int DWORD;typedef long long __int64;typedef uintptr_t UINT_PTR;
#include "Car.h"
#include "Mesh.h"
#include "Sector.h"
#include "SceneNode.h"
#include "MovingObject.h"
#include "MeshCollisionRecord.h"
#include "Collision2D.h"
StageObject placements[5],*g_stageObjects[6000];Mesh meshes[5];MeshCollisionRecord records[5][2];
Sector cells[3],*g_sectors[14096];SceneNode nodes[40],root;Car cars[2];MovingObjects g_movingObjects;
BYTE g_farVotes[40],*g_sectorEntryCarFlags,*g_movingObjectMeshIndex;
int *g_sectorEntryRaised,*g_movingObjectTypes;Mesh **g_movingObjectMeshes;
SectorObjectEntry *g_movingSectorEntries,*g_staticSectorEntries,**g_sectorListEntriesA,**g_sectorListEntriesB;
SectorObjectBounds *g_sectorObjectBounds;ObjectPointBounds *g_objectPointBounds;
short *g_sectorListIndexA,*g_sectorListIndexB;unsigned short *g_sectorListCountA,*g_sectorListCountB;
unsigned int g_unk0x0058ca6c;int g_unk0x0058c95c,g_unk0x0058c950,g_unk0x0058ca68,g_sectorCount=3,g_fixMatrixMultiplyCount;
int g_collisionSphereRadius,g_unk0x00591494;FixVector g_collisionSphereCentre;
unsigned short g_sqrtTable[4096];int g_sinTable[4096];int g_physicsTimeStep=65536;
double g_minus65536=-65536.0;
int mode,failed,created;BYTE image[65536];
std::vector<std::pair<void *,size_t>> allocated;
class CFileBuffer{public:static void *AllocateLockedBuffer(int n){
 assert(n>=0);void *p=malloc(n?n:1);assert(p && (uintptr_t)p>UINT32_MAX);memset(p,0xa5,n);allocated.push_back({p,(size_t)n});return p;
}};
class CGame{public:static int RegisterCallback(void *,void *p){assert(!p);return 0;}};
unsigned char RallyDataState(){return mode;}
void RallyData_ValidateIndex(int){}
void RallyData_FreeSectorElementTables(){}
void StageObjects_ReleaseObjectFiles(){}
void Mesh_ResetCloneCount(){}
SceneNode *RallyData_GetChallengeRenderState(){return &root;}
SceneNode *SceneNode_Create(SceneNode *p){assert(p==&root && created<40);return &nodes[created++];}
Mesh *Mesh_CloneInto(Mesh *m,StageObject *source){auto *p=source;int i=(int)(p-placements);assert(m==&meshes[i]);return i==failed?nullptr:m;}
StageObject *StageObject_GetNext(StageObject *p){return p->pNext;}
Car *Car_Get(int i){assert(i>=0 && i<2);return &cars[i];}
short Car_GetOrderCount(){return 2;}
int Track_GetGroundHeight(FixVector *,FixVector *normal,short *triangle,unsigned short *surface,int flag){
 assert(!flag);*normal={0,65536,0};*triangle=7;*surface=3;return 0;
}
void Sector_RemoveNode(SceneNode *p){p->sector=-1;}
void Sector_InsertNodeByPosition(SceneNode *p){p->sector=2;}
int StageObject_RunNodeActionAndSoundCallback(void *,int v){return v+1;}
unsigned int StageObject_GetLoadedVariantDataAndState(SectorObjectEntry **);
void StageObject_InitMovingObject(MovingObject *,int);
void StageObject_LoadAndClassifyMeshes();void StageObject_ResetMovingObjectCountsAndPhases();
void Collision_BuildObjectPointBounds(int *,FixVector *,SectorObjectBounds *);
void RallyData_CopyRaisedElementVector(int *,SectorObjectEntry **);
int RallyData_IsElementFlagSet(SectorObjectEntry **,int);
void RallyData_SetElementFlagBit(SectorObjectEntry **,int,int);
void RallyData_MarkElementReachedByCar(SectorObjectEntry *,int);


BYTE g_unk0x00590ed0[8][0x98];CollisionBox g_sectorObjectCollisionBox;
int g_unk0x005914d8,contactResult,overlapResult,contacts,views,shadows;Car *g_collisionCar;
int Collision_DoSpheresOverlap(int a,int b,int *left,int *right){
 assert(a==65536 && b>=0 && left==(int *)&cars[0].position && (uintptr_t)left>UINT32_MAX);
 assert(right && (uintptr_t)right>UINT32_MAX);return overlapResult;
}
void StageObject_QueueViewLensFlare(int *box,int *axes,int *position,FixVector *corners){
 assert(box==(int *)g_unk0x00590ed0[0] && axes==cars[0].bodyAxes && position==(int *)&cars[0].position && corners==cars[0].corners);views++;
}
int contact(Car *c){assert(c==&cars[0]);int i=c->pendingContactCount;if(i<5)c->contactSurfaces[1][c->pendingContactCount++]=(short)(100+contacts);contacts++;return contactResult;}
int Collision_ResolveCarTurnedObjectContact(Car *c,StageObject **,CollisionBox *,CollisionBox *){return contact(c);}
int Collision_CarVsBox(Car *c,CollisionBox *,int){return contact(c);}
int Collision_ResolveStaticObstacleContact(Car *,StageObject **,FixVector *,int){return 0;}
void StageObject_UpdateCarBoxShadowLighting(CollisionBox *,Car *){shadows++;}
int Collision_TestCarAgainstSectorObjects(Car *);
void CarPhysics_UpdateWheelSlipAndVelocityDamping(Car *);

std::vector<BYTE> output;
void put(uint32_t v){auto *p=(BYTE *)&v;output.insert(output.end(),p,p+4);}
void bytes(const void *p,size_t n){if(!n)return;assert(p);auto *b=(const BYTE *)p;output.insert(output.end(),b,b+n);}
int word(size_t at){int n;memcpy(&n,image+at,4);return n;}
void canonical(int count){
 put(g_unk0x0058c95c);put(g_unk0x0058ca6c);put(g_movingObjects.meshCount);put(g_movingObjects.count);
 for(int moving=0;moving<2;moving++){
  auto *indexes=moving?g_sectorListIndexB:g_sectorListIndexA;auto *counts=moving?g_sectorListCountB:g_sectorListCountA;
  auto **spans=moving?g_sectorListEntriesB:g_sectorListEntriesA;auto *table=moving?g_movingSectorEntries:g_staticSectorEntries;
  int total=moving?g_unk0x0058ca6c:g_unk0x0058c95c;
  for(int sector=0;sector<3;sector++){int i=indexes[sector];put(i);put(i<0?0:counts[i]);put(i<0?0:spans[i]-table);}
  for(int i=0;i<total;i++){auto &e=table[i];assert((uintptr_t)e.pObject>UINT32_MAX && (uintptr_t)e.pBounds>UINT32_MAX);
   put(e.pObject-placements);put(e.pBounds-g_sectorObjectBounds);put(e.pBounds->radius);put(e.pBounds->pProjection!=nullptr);
   if(e.pBounds->pProjection){assert((uintptr_t)e.pBounds->pProjection>UINT32_MAX);bytes(e.pBounds->pProjection,12);}else{BYTE zero[12]={};bytes(zero,12);}
  }
 }
 for(int i=0;i<count;i++){bytes(&placements[i].position,12);bytes(&placements[i].flags,5);}
 bytes(g_sectorEntryCarFlags,g_unk0x0058ca6c);bytes(g_sectorEntryRaised,g_unk0x0058ca6c*4);
 if(g_unk0x0058ca6c)bytes(g_movingObjectMeshIndex,g_unk0x0058ca6c);
 for(int i=0;i<g_movingObjects.meshCount;i++){put(g_movingObjectMeshes[i]-meshes);put(g_movingObjectTypes[i]);}
 for(int i=0;i<40;i++){
  auto &s=g_movingObjects.entries[i];auto &n=nodes[i];assert(s.pObject==&n);
  if(s.pEntry)assert((uintptr_t)s.pEntry>UINT32_MAX);if(s.pMatrix)assert(s.pMatrix==&s.keyB && (uintptr_t)s.pMatrix>UINT32_MAX);
  put(s.pEntry?s.pEntry-g_movingSectorEntries:-1);put(s.pObject-nodes);put(s.pMatrix==&s.keyB);
  bytes(&s.keyB,192);bytes(&s.mode,92);
  if(n.pObject)assert((uintptr_t)n.pObject>UINT32_MAX);
  put(n.pObject?(Mesh *)n.pObject-meshes:-1);put(n.sector);bytes(&n.current,128);bytes(&n.type,5);
 }
 bytes(g_movingObjects.carDistance,1280);bytes(g_farVotes,40);
}
'''
MAIN=r'''
int main(int argc,char **argv){
 assert(argc==5 && sizeof(void *)==8);int seed=atoi(argv[1]),count=atoi(argv[2]);mode=atoi(argv[3]);failed=atoi(argv[4]);
 static_assert(sizeof(SectorObjectEntry)==16 && offsetof(SectorObjectEntry,pBounds)==8,"native entries");
 static_assert(sizeof(SectorObjectBounds)==16 && offsetof(SectorObjectBounds,pProjection)==8,"native bounds");
 static_assert(sizeof(ObjectPointBounds)==12,"packed numeric projections");
 static_assert(offsetof(MovingObject,sectorIndex)!=0x118 && offsetof(MovingObjects,meshCount)!=0x2e40,"native moving strides");
 assert((uintptr_t)placements>UINT32_MAX && (uintptr_t)&g_movingObjects>UINT32_MAX);
 assert(fread(image,1,sizeof(image),stdin)==sizeof(image));
 g_unk0x0058c950=count;
 for(int s=0;s<3;s++){g_sectors[s]=&cells[s];cells[s].pObjects=s<count?&placements[s]:nullptr;}
 for(int i=0;i<count;i++){
  auto &p=placements[i];auto &m=meshes[i];size_t at=0x1000+i*0xa0;
  memcpy(&p.position,image+at,12);p.pMesh=&m;memcpy(&p.flags,image+at+0x10,5);memcpy(&p.matrix,image+at+0x18,64);
  p.pNext=i+3<count?&placements[i+3]:nullptr;g_stageObjects[i]=&p;
  m.pVertexData=(DWORD *)(image+0x3800+i*0x60);m.field_0x10=2;m.sizeUnits=2;m.pCollisionRecords=&records[i][0];
  for(int r=0;r<2;r++){auto &record=records[i][r];size_t loc=0x3000+i*0xb8+r*0x5c;
   record.type=image[loc];memcpy(&record.matrix,image+loc+4,64);record.width=word(loc+0x44);record.length=word(loc+0x48);record.height=word(loc+0x4c);record.pNext=r?nullptr:&records[i][1];
  }
 }
 for(int i=0;i<40;i++)nodes[i].sector=-1;
 for(int i=0;i<2;i++){cars[i].velocity={32768+i*65536,0,-49152};cars[i].speed=seed%2?65536:16384;}
 for(int i=0;i<4096;i++)g_sqrtTable[i]=(unsigned short)(sqrt(8+16*i)*256);
 auto oldMeshes=std::vector<Mesh>(meshes,meshes+5);BYTE oldRecords[sizeof(records)];memcpy(oldRecords,records,sizeof(records));
 auto oldCars=std::vector<Car>(cars,cars+2);auto oldImage=std::vector<BYTE>(image,image+sizeof(image));
 RallyData_RebuildSectorStaticObjectTables();assert(created==40);canonical(count);
 // Check every allocation's native size, including both pointer tables and all pointer-bearing records.
 size_t cursor=0;auto alloc=[&](size_t size){assert(cursor<allocated.size() && allocated[cursor++].second==size);};
 alloc(6);alloc(6);if(count)alloc(count*2*sizeof(SectorObjectBounds));
 int listsA=0,listsB=0;for(int i=0;i<3;i++){listsA+=g_sectorListIndexA[i]>=0;listsB+=g_sectorListIndexB[i]>=0;}
 if(listsA){alloc(listsA*sizeof(SectorObjectEntry *));alloc(listsA*2);}
 if(listsB){alloc(listsB*sizeof(SectorObjectEntry *));alloc(listsB*2);}
 if(g_unk0x0058c95c)alloc(g_unk0x0058c95c*sizeof(SectorObjectEntry));
 if(g_unk0x0058ca6c){alloc(g_unk0x0058ca6c*sizeof(SectorObjectEntry));alloc(g_unk0x0058ca6c);alloc(g_unk0x0058ca6c*4);}
 if(count)alloc(count*sizeof(ObjectPointBounds));alloc(g_unk0x0058ca6c*sizeof(Mesh *));
 if(g_unk0x0058ca6c)alloc(g_unk0x0058ca6c);if(g_movingObjects.meshCount)alloc(g_movingObjects.meshCount*4);
 assert(cursor==allocated.size());
 for(unsigned sector: {0u,1u,2u,3u,0xffffffffu}){unsigned count;Sector_GetListA(sector,&count);Sector_GetListB(sector,&count);}
 int n=g_unk0x0058ca6c;FixVector position={};
 for(int i=0;i<n;i++){auto *e=&g_movingSectorEntries[i];RallyData_SetElementFlagBit(&e,i%2,0);RallyData_IsElementFlagSet(&e,i%2);
  RallyData_CopyRaisedElementVector((int *)&position,&e);RallyData_MarkElementReachedByCar(e,i%2);
 }
 RallyData_ApplyPlayerFlagsToObjects(0);RallyData_ApplyPlayerFlagsToObjects(1);
 if(n && g_movingObjects.meshCount){StageObject_QueueOrEvictMovingObject(g_movingSectorEntries,1,0);StageObject_ApplyInterpolatedNodeMatrices(0);
  if(seed>=2 && count==5)for(int i=0;i<40;i++)StageObject_QueueOrEvictMovingObject(&g_movingSectorEntries[i%n],i%3,i%2);
  StageObject_ApplyInterpolatedNodeMatrices(0);
 }
 canonical(count);
 // Both cached and uncached sprite-box paths retain complete pointer values and immutable backing data.
 for(int i=0;i<n;i++){
  auto *e=&g_movingSectorEntries[i];if(e->pBounds->pProjection){CollisionBox box={};StageObject_BuildSpriteExtentOrientation(&box,e,(int *)&position);
   assert(box.pVertex==&position && (uintptr_t)box.pVertex>UINT32_MAX && box.pArray==nullptr);auto before=box;
   StageObject_BuildSpriteExtentOrientation(&box,e,(int *)&position);assert(!memcmp(&box,&before,sizeof(box)));
  }else {StageObject_SetCollisionSphereAndMaterial(&position,e);assert(g_collisionSphereRadius==e->pBounds->radius && g_unk0x00591494==e->pObject->pMesh->pCollisionRecords->height);}
 }
 assert(!memcmp(meshes,oldMeshes.data(),sizeof(meshes)) && !memcmp(records,oldRecords,sizeof(records)));
 // A distant address used only as an identity must not alias slot zero after division.
 auto *distant=(SectorObjectEntry *)((uintptr_t)g_movingSectorEntries+(1ull<<36));
 BYTE beforeFlags[10]={};if(n)memcpy(beforeFlags,g_sectorEntryCarFlags,n);
 assert(!RallyData_IsElementFlagSet(&distant,0));RallyData_SetElementFlagBit(&distant,0,1);
 assert(!n || !memcmp(beforeFlags,g_sectorEntryCarFlags,n));
 assert(!memcmp(cars,oldCars.data(),sizeof(cars)) && !memcmp(image,oldImage.data(),sizeof(image)));

 // The car contains native pointers before every contact field. Guard all
 // other bytes while running the actual traversal with controlled response leaves.
 for(int overlap: {0,1})for(int response: {0,1})for(int movingOnly: {0,1}){
  cars[0]=oldCars[0];auto &c=cars[0];c.sector=0;c.neighbours.s[0]=1;c.neighbours.s[1]=2;c.neighbours.s[2]=-1;
  c.collisionRadius=65536;c.field_0xc18=movingOnly;c.field_0xc20=0;c.index=0;
  memset(c.objectContactState,0xa5,sizeof(c.objectContactState));c.previousContactCount=7;c.pendingContactCount=3;
  Car expected=c;contacts=views=shadows=0;overlapResult=overlap;contactResult=response;
  Collision_TestCarAgainstSectorObjects(&c);
  memset(expected.objectContactState,0,sizeof(expected.objectContactState));expected.pendingContactCount=expected.previousContactCount=(char)(contacts<5?contacts:5);
  for(int i=0;i<expected.pendingContactCount;i++)expected.contactSurfaces[0][i]=expected.contactSurfaces[1][i]=(short)(100+i);
  assert(!memcmp(&c,&expected,sizeof(c)));assert(views>=contacts && (overlap || !views));
 }
 // Independent fixed-point slip model, including the untouched upper four
 // corner states and all pointer-bearing/runtime regions of the car.
 auto mul=[](int a,int b){return (int)((int64_t)a*b>>16);};
 for(int trial=0;trial<3000;trial++){
  auto &c=cars[0];c=oldCars[0];c.pWorld=&c.physicsMatrix;c.speed=trial%7==0?-32768:trial%3==0?131072:trial*23;
  for(int i=0;i<8;i++){c.corners[i]={(i%2?-1:1)*(trial*13+i*171),123,-(trial*17+i*131)};c.field_0x564[i]={31,47,59};c.field_0x8fc[i]=trial%5?trial*7:0;c.objectContactState[i]=(trial+i)%3;}
  Car expected=c;int v=mul(c.speed>65536?65536:c.speed,0x6666);
  for(int i=0;i<4;i++){int d=abs(abs(c.corners[i].x)-abs(c.corners[i].z));int u=mul((d%0x401)<<6,v);
   if(!expected.objectContactState[i] || u>=expected.field_0x8fc[i]){expected.field_0x8fc[i]=u;expected.objectContactState[i]=1;expected.field_0x564[i]={0,65536,0};}}
  expected.velocity={mul(c.velocity.x,0xf851),mul(c.velocity.y,0xf851),mul(c.velocity.z,0xf851)};
  CarPhysics_UpdateWheelSlipAndVelocityDamping(&c);assert(g_collisionCar==&c && !memcmp(&c,&expected,sizeof(c)));
 }

 assert(fwrite(output.data(),1,output.size(),stdout)==output.size());for(auto &p:allocated)free(p.first);
}
'''

def main():
 source=ROOT/'CMR2Decomp'
 names={'FixedPoint.cpp':['FixMatrix_Multiply','FixMatrix_RotateVector','FixMatrix_InverseRotateVector','FixMatrix_GetRight','FixMatrix_GetUp','FixMatrix_GetForward','FixMatrix_SetRight','FixMatrix_SetUp','FixMatrix_SetForward','FixMatrix_SetPosition'],
 'RallyData.cpp':['Sector_GetListA','Sector_GetListB','RallyData_ApplyPlayerFlagsToObjects','RallyData_CopyRaisedElementVector','RallyData_IsElementFlagSet','RallyData_SetElementFlagBit','RallyData_MarkElementReachedByCar','RallyData_RebuildSectorStaticObjectTables'],
 'StageObjects.cpp':['StageObject_GetLoadedVariantDataAndState','StageObject_ResetMovingObjectCountsAndPhases','StageObject_LoadAndClassifyMeshes','StageObject_InitMovingObject','StageObject_QueueOrEvictMovingObject','StageObject_ApplyInterpolatedNodeMatrices','Collision_BuildObjectPointBounds','StageObject_SetCollisionSphereAndMaterial','StageObject_BuildSpriteExtentOrientation','Collision_TestCarAgainstSectorObjects','CarPhysics_UpdateWheelSlipAndVelocityDamping']}
 funcs='\n'.join(body((source/file).read_text(),name) for file,items in names.items() for name in items)
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
 with tempfile.TemporaryDirectory(prefix='cmr2-native-sector-objects-') as directory:
  tmp=Path(directory)
  for header in ('Car.h','SceneNode.h','Sector.h','Mesh.h','LayoutChecks.h','MovingObject.h','SectorObjectEntry.h','MeshCollisionRecord.h','Collision2D.h'):shutil.copyfile(source/header,tmp/header)
  shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
  (tmp/'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;\n')
  (tmp/'Graphics.h').write_text('struct Graphics;struct Texture;class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0;static constexpr double m_65536=65536.0;};\n')
  fixture=tmp/'sector.cpp';fixture.write_text(PRELUDE+funcs+MAIN);exe=tmp/'sector'
  subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-I',str(tmp),'-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast','-fsanitize=address,undefined',str(fixture),'-o',str(exe)],check=True)
  oracle=Tables(ROOT/'cmr2bin/CMR2.exe');number=0
  for args in itertools.product(range(4),[0,1,2,5],[1,2,3],[-1,1]):
   oracle.initialize(*args);initial=oracle.read(HEAP,0x10000);oracle.pipeline(*args)
   p=subprocess.run([str(exe),*map(str,args)],input=initial,capture_output=True,env=env);assert p.returncode==0,(args,p.stderr.decode())
   expected=oracle.produced_summary+oracle.lifecycle_summary
   assert p.stdout==expected,(args,len(p.stdout),len(expected),[(hex(i),a,b) for i,(a,b) in enumerate(zip(p.stdout,expected)) if a!=b][:16]);number+=1
 print(f'PASS: {number} native sector-table lifecycles vs original x86; 19 actual changed bodies; 768 native contact traversals and 288000 slip cases with independent full-car model, production layouts, native allocations/pointers above 4GB, full canonical records and immutable storage, ASan/UBSan')
if __name__=='__main__':main()
