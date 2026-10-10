#!/usr/bin/env python3
"""Two complete collision bodies on native x64, compared with original fixtures."""
import itertools
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

from check_64bit_car_contacts import ROOT,body
from differential_collision_boxes import Boxes,GLOBALS,HEAP

PRELUDE=r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
typedef unsigned char BYTE;
typedef long long __int64;
#include "Collision2D.h"
unsigned short g_sqrtTable[4096];
FixVector g_collisionPush,g_collisionPushB,g_collisionContactNormal,g_collisionSphereCentre;
int g_collisionSphereRadius;
'''
MAIN=r'''
int main(int argc,char **argv) {
 assert(argc>=3);bool split=!strcmp(argv[1],"split");
 static_assert(sizeof(void *)==8,"native pointers");
 static_assert(offsetof(CollisionBox,pArray)==0x90,"fixed scalar prefix");
 static_assert(offsetof(CollisionBox,pVertex)==0x98,"centre follows native corner pointer");
 static_assert(sizeof(CollisionBox)==0xa0,"native runtime box");
 alignas(16) BYTE heap[65536];assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap));
 FixVector *globals[]={&g_collisionPush,&g_collisionPushB,&g_collisionContactNormal};
 for(auto p:globals)assert(fread(p,1,sizeof(*p),stdin)==sizeof(*p));
 assert(fread(&g_collisionSphereCentre,1,sizeof(g_collisionSphereCentre),stdin)==sizeof(g_collisionSphereCentre));
 assert(fread(&g_collisionSphereRadius,1,sizeof(g_collisionSphereRadius),stdin)==sizeof(g_collisionSphereRadius));
 assert(fread(g_sqrtTable,1,sizeof(g_sqrtTable),stdin)==sizeof(g_sqrtTable));
 CollisionBox boxes[2];FixVector centres[2],vertices[2][8],direction;
 memcpy(&centres[0],heap+0x3000,sizeof(FixVector));memcpy(&centres[1],heap+0x3100,sizeof(FixVector));
 memcpy(vertices[0],heap+0x4000,sizeof(vertices[0]));memcpy(vertices[1],heap+0x4100,sizeof(vertices[1]));
 for(int i=0;i<2;i++) {
  memset(&boxes[i],0xcc,sizeof(CollisionBox));memcpy(&boxes[i],heap+0x1000+i*0x200,0x90);
  uint32_t array,centre;memcpy(&array,heap+0x1090+i*0x200,4);memcpy(&centre,heap+0x1094+i*0x200,4);
  if(array){assert(array==0x10004000 || array==0x10004100);boxes[i].pArray=vertices[array==0x10004100];}
  else boxes[i].pArray=nullptr;
  if(centre){assert(centre==0x10003000 || centre==0x10003100);boxes[i].pVertex=&centres[centre==0x10003100];}
  else boxes[i].pVertex=nullptr;
  if(boxes[i].pArray)assert(reinterpret_cast<uintptr_t>(boxes[i].pArray)>UINT32_MAX);
  if(boxes[i].pVertex)assert(reinterpret_cast<uintptr_t>(boxes[i].pVertex)>UINT32_MAX);
 }
 CollisionBox before[2];memcpy(before,boxes,sizeof(boxes));
 assert(reinterpret_cast<uintptr_t>(&boxes[0])>UINT32_MAX);
 uint32_t result=0;
 if(split) {
  assert(argc==4);memcpy(&direction,heap+0x6000,sizeof(direction));
  Collision_SplitBoxSeparationMovement(&boxes[0],&boxes[1],&direction,atoi(argv[2]),atoi(argv[3]));
 } else {
  assert(argc==3 && !strcmp(argv[1],"sphere"));
  int along0;unsigned along1;memcpy(&along0,heap+0x6100,4);memcpy(&along1,heap+0x6104,4);
  result=Collision_SphereVsBox(&boxes[0],&along0,&along1,atoi(argv[2]))&255;
  memcpy(heap+0x6100,&along0,4);memcpy(heap+0x6104,&along1,4);
 }
 for(int i=0;i<2;i++) {
  assert(boxes[i].pArray==before[i].pArray && boxes[i].pVertex==before[i].pVertex);
  memcpy(heap+0x1000+i*0x200,&boxes[i],0x90);
 }
 memcpy(heap+0x3000,&centres[0],sizeof(FixVector));memcpy(heap+0x3100,&centres[1],sizeof(FixVector));
 memcpy(heap+0x4000,vertices[0],sizeof(vertices[0]));memcpy(heap+0x4100,vertices[1],sizeof(vertices[1]));
 assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));
 for(auto p:globals)assert(fwrite(p,1,sizeof(*p),stdout)==sizeof(*p));
 assert(fwrite(&result,1,sizeof(result),stdout)==sizeof(result));
}
'''


class NativeOracle(Boxes):
    def result(self,*args,**kwargs):
        globals_=b''.join(self.read(self.addr(a),n) for a,n in GLOBALS)
        sphere=self.read(self.addr(0x591498),12)+self.read(self.addr(0x591490),4)
        roots=self.read(self.addr(0x6e0ef4),8192)
        result=super().result(*args,**kwargs)
        self.native_input+=globals_+sphere+roots
        return result


def main():
    oracle=NativeOracle(ROOT/'cmr2bin/CMR2.exe');source=ROOT/'CMR2Decomp'
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-boxes-') as directory:
        tmp=Path(directory)
        shutil.copyfile(source/'Collision2D.h',tmp/'Collision2D.h')
        shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
        (tmp/'Graphics.h').write_text('class CGraphics{public:static const double m_oneOver65536;};\n')
        functions='\n'.join(body((source/'Collision2D.cpp').read_text(),name) for name in
                            ('Collision_SplitBoxSeparationMovement','Collision_SphereVsBox'))
        fixture=tmp/'boxes.cpp';fixture.write_text(PRELUDE+functions+MAIN)
        executable=tmp/'boxes'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast',
                        '-fsanitize=address,undefined',str(fixture),'-o',str(executable)],check=True)
        count=0
        def check(result,args):
            nonlocal count
            native=subprocess.run([str(executable),*map(str,args)],input=oracle.native_input,
                                  capture_output=True,env=env)
            assert native.returncode==0,(args,native.stderr.decode())
            expected=result[0]+b''.join(result[1])+struct.pack('<I',result[2] or 0)
            if native.stdout!=expected:
                at=next(i for i,(a,b) in enumerate(zip(native.stdout,expected)) if a!=b)
                raise AssertionError((args,'native/original difference',hex(at)))
            count+=1
        for direction,amount,scale,nulls,alias in itertools.product(((0,0,0),(-32768,8192,49152)),
                                                                  (0,1,196609),(0,32768,65536),
                                                                  (0,1,2,3,4,8,15),(0,1)):
            check(oracle.movement(direction,amount,scale,nulls,alias,17),('split',amount,scale))
        for x,z,scale,no_array,rotation in itertools.product((-98304,-65536,0,65536,98304),
                                                            (-65536,0,65536),(0,32768,65536),
                                                            (0,1),(0,1)):
            check(oracle.sphere(x,z,scale,no_array,rotation,3),('sphere',scale))
    print(f'PASS: two actual collision bodies, {count} x64 cases; complete original heap/globals with native box/geometry pointers above 4 GB, ASan/UBSan')


if __name__=='__main__':main()
