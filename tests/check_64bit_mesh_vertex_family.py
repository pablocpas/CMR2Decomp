#!/usr/bin/env python3
"""Actual mesh clone on x64, above 4GB, vs complete original x86 outputs.

Production layouts and portable original math; original providers control only
allocation and final part construction/upload. An old +0x18 negative control
must fail or disagree with the independent x86 oracle.
"""
import itertools,os,shutil,struct,subprocess,tempfile
from pathlib import Path
from differential_mesh_vertex_family import Vertices,ROOT
from check_64bit_race_state_pointers import body

PRE=r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
typedef unsigned char BYTE;typedef unsigned int DWORD;typedef long long __int64;
#include "Mesh.h"
#include "MeshVertex.h"
#include "Sector.h"
class CGraphics{public:static constexpr double m_65536=65536.0,m_oneOver65536=1.0/65536.0;};
unsigned short g_sqrtTable[4096];int g_sinTable[4096];
short g_meshCloneBase[4096];Mesh *g_meshClones[4096];int g_meshCloneCount,g_meshCount;
BYTE *image;Mesh src,dst;StageObject placement;size_t cursor;int calls;
std::vector<size_t> requests;
class CFileBuffer{public:static void *AllocateLockedBuffer(size_t n){
 assert(cursor+n<0xf000);void *p=image+cursor;cursor=(cursor+n+31)&~size_t(15);
 requests.push_back(n);assert((uintptr_t)p>UINT32_MAX);return p;
}};
Mesh *Mesh_Alloc(){g_meshCount++;dst.field_0x118=0;return &dst;}
void Mesh_BuildParts(Mesh *p){assert(p==&dst && calls++==0);}
void Mesh_UploadVertices(Mesh *p){assert(p==&dst && calls++==1);}
void bytes(const void *p,size_t n){assert(fwrite(p,1,n,stdout)==n);}
void word(uint32_t v){bytes(&v,4);}
int get(size_t at){int v;memcpy(&v,image+at,4);return v;}
void canonical(){
 bytes(dst.field_0x0,12);word(dst.field_0x10);word(dst.triangleCount);word(dst.field_0x2c);word(dst.flags);
 bytes(dst.field_0x104,1);bytes(dst.field_0x108,4);assert(!dst.pCollisionRecords);word(0);
 bytes(&dst.sizeUnits,4);word(dst.field_0x114);word(dst.field_0x118);word(dst.field_0x11c);
 bytes(dst.pVertexData,dst.field_0x10*48);bytes(dst.pTriangles,dst.triangleCount*76);
 bytes(dst.pField20,dst.triangleCount*20);bytes(dst.pLightLevels,dst.field_0x10*4);
}
'''
MAIN=r'''
int main(){
 static_assert(sizeof(void *)==8 && offsetof(StageObject,matrix)==0x20,"native placement matrix");
 static_assert(sizeof(MeshVertexF)==48 && sizeof(MeshTriangle)==76,"pointer-free wire vertices and triangles");
 image=(BYTE *)malloc(65536);assert(image && (uintptr_t)image>UINT32_MAX);
 for(int i=0;i<4096;i++)g_sqrtTable[i]=(unsigned short)__builtin_sqrt((8+16*i)*65536.0);
 unsigned int settings[4];int cases=0;
 while(fread(settings,1,sizeof(settings),stdin)==sizeof(settings)){
  assert(fread(image,1,65536,stdin)==65536);auto before=std::vector<BYTE>(image,image+65536);
  int n=settings[1],tri=settings[2];
  memset(&src,0xa5,sizeof(src));memset(&dst,0xa5,sizeof(dst));memset(&placement,0,sizeof(placement));
  memcpy(src.field_0x0,image+0x1000,12);src.field_0x10=n;src.triangleCount=tri;
  src.pVertexData=(DWORD *)(image+0x2000);src.pTriangles=(MeshTriangle *)(image+0x3000);
  src.pField20=image+0x4000;src.pLightLevels=(int *)(image+0x5000);
  src.flags=get(0x1030);src.field_0x2c=get(0x102c);src.field_0x104[0]=image[0x1104];
  memcpy(src.field_0x108,image+0x1108,4);src.field_0x111=image[0x1111];src.lodIndex=image[0x1112];src.lodCount=image[0x1113];src.field_0x114=get(0x1114);
  memcpy(&placement.matrix,image+0x1418,64);placement.pMesh=&src;placement.flags=0xcafe;
  auto original=src;auto originalPlacement=placement;
  cursor=0x8000;calls=0;requests.clear();g_meshCount=17;g_meshCloneCount=2;
  g_meshCloneBase[0]=-12;g_meshCloneBase[1]=-13;g_meshCloneBase[2]=-14;g_meshCloneBase[3]=-15;
  memset(g_meshClones,0,sizeof(g_meshClones));
  assert(Mesh_CloneInto(&src,&placement)==&dst);assert(calls==2);
  assert(g_meshCount==18 && g_meshCloneCount==3 && g_meshCloneBase[2]==17 && g_meshClones[2]==&dst);
  assert((requests==std::vector<size_t>{size_t(n*48),size_t(tri*76),size_t(tri*20),size_t(n*4)}));
  assert(!memcmp(&src,&original,sizeof(src)) && !memcmp(&placement,&originalPlacement,sizeof(placement)));
  assert(!memcmp(image+0x1000,before.data()+0x1000,0x5000));
  size_t at=0x8000;
  for(size_t size:requests){assert(!memcmp(image+at-16,before.data()+at-16,16));assert(!memcmp(image+at+size,before.data()+at+size,16));at=(at+size+31)&~size_t(15);}
  canonical();cases++;
 }
 assert(cases==192);free(image);
}
'''

def main():
 source=ROOT/'CMR2Decomp'
 clone=body((source/'Mesh.cpp').read_text(),'Mesh_CloneInto')
 helpers='\n'.join(body((ROOT.parent/'OpenCMR2/game/FixedPoint.cpp').read_text(),name) for name in
 ['FixMatrix_GetRight','FixMatrix_GetUp','FixMatrix_GetForward','FixMatrix_GetPosition','FixMatrix_SetRight','FixMatrix_SetUp','FixMatrix_SetForward','FixMatrix_SetPosition','FixMatrix_InverseRotateVector'])
 old=clone.replace('&pSource->matrix','(FixMatrix *)((BYTE *)pSource + 0x18)');assert old!=clone
 oracle=Vertices(ROOT/'cmr2bin/CMR2.exe');inputs,expected=bytearray(),bytearray()
 for case in itertools.product(range(4),(0,1,4,8),(0,1,3),range(4)):
  _,_,canonical=oracle.clone(*case);inputs+=struct.pack('<4I',*case)+oracle.initial_heap;expected+=canonical
 with tempfile.TemporaryDirectory(prefix='cmr2-native-mesh-') as folder:
  tmp=Path(folder)
  for name in ('FloatToInt.h','Mesh.h','MeshVertex.h','Sector.h'):shutil.copyfile(source/name,tmp/name)
  shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
  for name,fn in [('typed',clone),('old-matrix-offset',old)]:
   cpp,exe=tmp/(name+'.cpp'),tmp/name;cpp.write_text(PRE+helpers+fn+MAIN)
   subprocess.run(['clang++','-include',str(ROOT/'CMR2Decomp/FloatToInt.h'),'-m64','-std=c++17','-O2','-fwrapv','-fno-strict-aliasing','-fsanitize=address,undefined','-fno-sanitize-recover=all','-I'+str(tmp),str(cpp),'-o',str(exe)],check=True)
   p=subprocess.run([str(exe)],input=inputs,capture_output=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
   if name=='typed':
    assert p.returncode==0,p.stderr.decode();assert p.stdout==expected,('original x86 clone outputs',next((i for i,(a,b) in enumerate(zip(p.stdout,expected)) if a!=b),None))
    print('PASS: 192 actual native mesh clones vs original x86; complete output vertices/normals, triangles/work/light levels, source guards, high pointers and ASan/UBSan')
   else:
    assert p.returncode!=0 or p.stdout!=expected,'old +0x18 negative control unexpectedly passes'
    print('PASS: old placement matrix +0x18 negative control is rejected')
if __name__=='__main__':main()
