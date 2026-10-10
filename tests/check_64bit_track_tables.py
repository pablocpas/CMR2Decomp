#!/usr/bin/env python3
"""Four actual table bodies on native x64 with full original-image projections."""
import itertools
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
from check_64bit_car_contacts import ROOT,body
from differential_track_tables import Tables,MODES
from differential_track_geometry import Geometry

PRELUDE=r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
typedef unsigned char BYTE;
typedef long long __int64;
#include "TrackCollisionData.h"
TrackTriangle *g_trackTriangles;
TrackCollisionHeader *g_trackCollisionHeader;
TrackCollisionCount *g_trackTriangleCountRecord;
TrackCollisionTables g_trackCollisionTables;
short *g_trackTriangleIndices,*g_trackGridRows,*g_trackGridColumns;
TrackCollisionCount *g_trackVertexCountRecord;
BYTE *g_trackCollisionBlock;
int *g_trackTriangleListCount,g_trackLevelNodeCounts[5];
BYTE StageTiming_NoOpLightRelease(){return 1;}
int registrations;
BYTE *fixtureHeap;
int nearestCalls,nearestCount;
uint32_t nearestList;
int Track_FindNearestTriangle(FixVector *point,short *out,int y,short count,short *list) {
 assert(point==reinterpret_cast<FixVector *>(fixtureHeap+0x1000));
 assert(out==reinterpret_cast<short *>(fixtureHeap+0x1100));
 assert(reinterpret_cast<uintptr_t>(list)>UINT32_MAX);
 nearestCalls++;nearestCount=count;
 nearestList=0x10000000+static_cast<uint32_t>(reinterpret_cast<BYTE *>(list)-fixtureHeap);
 *out=123;return 0x12345678;
}
class CGame { public:
 static int RegisterCallback(BYTE (*fn)(),void *context){
  assert(fn==StageTiming_NoOpLightRelease && context==nullptr);
  assert(reinterpret_cast<uintptr_t>(fn)>UINT32_MAX);registrations++;return 0;
 }
};
'''
MAIN=r'''
int main(int argc,char **argv) {
 assert(argc>=2);bool block=!strcmp(argv[1],"block");
 static_assert(sizeof(void *)==8,"native pointers");
 static_assert(sizeof(TrackTriangle)==8 && sizeof(TrackQuadNode)==8,"fixed disk records");
 static_assert(sizeof(TrackCollisionHeader)==20 && sizeof(TrackCollisionCount)==4,"fixed headers");
 static_assert(sizeof(TrackCollisionTables)==6*sizeof(void *),"contiguous runtime slots");
 static_assert(offsetof(TrackCollisionTables,vertices)==5*sizeof(void *),"sixth traversal slot aliases vertices");
 alignas(16) BYTE heap[65536],vertices[0x100000];
 assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap));fixtureHeap=heap;
 assert(reinterpret_cast<uintptr_t>(heap)>UINT32_MAX && reinterpret_cast<uintptr_t>(vertices)>UINT32_MAX);
 if(block) {
  assert(argc==2);StageTiming_IndexSerializedStageTables(heap+0x1000);assert(registrations==1);
  auto project=[&](const void *p)->uint32_t {
   assert(reinterpret_cast<uintptr_t>(p)>UINT32_MAX);
   ptrdiff_t index=static_cast<const BYTE *>(p)-heap;
   assert(index>=0 && index<65536);return 0x10000000+static_cast<uint32_t>(index);
  };
  uint32_t pointers[15]={project(g_trackCollisionBlock),project(g_trackCollisionHeader),
   project(g_trackCollisionTables.levels[0]),project(g_trackCollisionTables.levels[1]),
   project(g_trackCollisionTables.levels[2]),project(g_trackCollisionTables.levels[3]),
   project(g_trackCollisionTables.levels[4]),project(g_trackVertexCountRecord),project(g_trackVertices),
   project(g_trackTriangleCountRecord),project(g_trackTriangles),project(g_trackTriangleListCount),
   project(g_trackTriangleIndices),project(g_trackGridColumns),project(g_trackGridRows)};
  assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));
  assert(fwrite(pointers,1,sizeof(pointers),stdout)==sizeof(pointers));
  assert(fwrite(g_trackLevelNodeCounts,1,sizeof(g_trackLevelNodeCounts),stdout)==sizeof(g_trackLevelNodeCounts));
 } else if(!strcmp(argv[1],"tree")) {
  assert(argc==3);
  g_trackCollisionHeader=reinterpret_cast<TrackCollisionHeader *>(heap+0x1200);
  g_trackGridColumns=reinterpret_cast<short *>(heap+0x1220);
  g_trackGridRows=reinterpret_cast<short *>(heap+0x1224);
  g_trackTriangleIndices=reinterpret_cast<short *>(heap+0x8000);
  for(int i=0;i<5;i++)g_trackCollisionTables.levels[i]=reinterpret_cast<TrackQuadNode *>(heap+0x2000+i*0x800);
  // This controlled fixture supplies the original sixth-slot alias as node
  // records so both the level-5 leaf and the >=5 rejection are observable.
  g_trackVertices=reinterpret_cast<FixVector *>(heap+0x2000+5*0x800);
  TrackCollisionTables before=g_trackCollisionTables;
  uint32_t result=Track_FindTriangle(reinterpret_cast<FixVector *>(heap+0x1000),
   reinterpret_cast<short *>(heap+0x1100),atoi(argv[2]));
  assert(!memcmp(&before,&g_trackCollisionTables,sizeof(before)));
  uint32_t metadata[4]={result,static_cast<uint32_t>(nearestCalls),static_cast<uint32_t>(nearestCount),nearestList};
  assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));
  assert(fwrite(metadata,1,sizeof(metadata),stdout)==sizeof(metadata));
 } else {
  assert(argc==4);short tri=static_cast<short>(atoi(argv[2]));int alias=atoi(argv[3]);
  assert(fread(vertices,1,sizeof(vertices),stdin)==sizeof(vertices));
  g_trackVertices=reinterpret_cast<FixVector *>(vertices);
  g_trackTriangles=reinterpret_cast<TrackTriangle *>(heap+0x5000);
  TrackCollisionTables pointers=g_trackCollisionTables;
  unsigned short first;memcpy(&first,heap+0x6100,2);
  BYTE *out=alias==0?heap+0x6000:alias==1?vertices+first*12:
   !strcmp(argv[1],"copy")?heap+0x6100:heap+0x5000+tri*8;
  if(!strcmp(argv[1],"get"))assert(Track_GetTriangle(reinterpret_cast<FixVector *>(out),tri)==1);
  else if(!strcmp(argv[1],"copy"))StageTiming_CopyTriangleVertices(reinterpret_cast<FixVector *>(out),
    reinterpret_cast<unsigned short *>(heap+0x6100),reinterpret_cast<unsigned short *>(heap+0x6900));
  else {
   assert(!strcmp(argv[1],"face"));
   StageTiming_ReadStageFaceRecord(tri,reinterpret_cast<short *>(out),reinterpret_cast<short *>(out+2),
    reinterpret_cast<short *>(out+4),reinterpret_cast<short *>(out+6),reinterpret_cast<unsigned short *>(out+8));
  }
  assert(!memcmp(&pointers,&g_trackCollisionTables,sizeof(pointers)));
  assert(g_trackTriangles==reinterpret_cast<TrackTriangle *>(heap+0x5000));
  assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));
  assert(fwrite(vertices,1,sizeof(vertices),stdout)==sizeof(vertices));
 }
}
'''


def main():
    oracle=Tables(ROOT/'cmr2bin/CMR2.exe');source=ROOT/'CMR2Decomp'
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-track-tables-') as directory:
        tmp=Path(directory)
        shutil.copyfile(source/'TrackCollisionData.h',tmp/'TrackCollisionData.h')
        shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
        (tmp/'Graphics.h').write_text('class CGraphics{public:static const double m_oneOver65536;};\n')
        stage=(source/'StageTiming.cpp').read_text(encoding='latin1')
        track=(source/'TrackCollision.cpp').read_text(encoding='latin1')
        functions='\n'.join(body(track,name) for name in ('Track_GetTriangle','Track_FindTriangle'))
        functions+='\n'+'\n'.join(body(stage,name) for name in ('StageTiming_IndexSerializedStageTables',
                                         'StageTiming_ReadStageFaceRecord','StageTiming_CopyTriangleVertices'))
        fixture=tmp/'tables.cpp';fixture.write_text(PRELUDE+functions+MAIN)
        executable=tmp/'tables'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast','-fPIE','-pie',
                        '-fsanitize=address,undefined',str(fixture),'-o',str(executable)],check=True)
        count=0
        def check(args,expected):
            nonlocal count
            result=subprocess.run([str(executable),*map(str,args)],input=oracle.native_input,
                                  capture_output=True,env=env)
            assert result.returncode==0,(args,result.stderr.decode())
            if result.stdout!=expected:
                at=next(i for i,(a,b) in enumerate(zip(result.stdout,expected)) if a!=b)
                raise AssertionError((args,'native projection mismatch',hex(at)))
            count+=1
        for args in itertools.product(((0,0,0,0,0),(1,2,3,4,5),(5,0,1,0,2),(-1,2,0,0,0)),(0,17),(0,9),(0,7),(31,)):
            h,pointers,counts,_=oracle.block(*args)
            check(('block',),h+struct.pack('<15I',*pointers)+counts)
        for kind in ('get','copy','face'):
            for tri,mode,alias in itertools.product((0,) if kind=='copy' else (-4,-1,0,1,3),MODES,(0,1,2)):
                h,v=oracle.reader(kind,tri,mode,alias,5)
                check((kind,tri,alias),h+v)
        geometry=Geometry(ROOT/'cmr2bin/CMR2.exe')
        for seed,depth,outside in itertools.product(range(8),range(7),range(5)):
            returned,trace,h=geometry.run_tree(seed,depth,outside)
            oracle.native_input=bytes([0x5a])*65536
            # Original fixtures mutate the initial poisoned heap to add tables,
            # coordinates and the output sentinel. Restore only the provider's
            # output write to obtain the exact native input.
            initial=bytearray(h)
            if trace:initial[0x1100:0x1102]=b'ZZ'
            oracle.native_input=bytes(initial)
            metadata=(returned,len(trace),trace[0][3] if trace else 0,trace[0][4] if trace else 0)
            check(('tree',seed),h+struct.pack('<4I',*metadata))
    print(f'PASS: five actual table bodies, {count} x64 scenarios, native table/vertex pointers above 4 GB, fixed disk layouts and complete original heaps, high unsigned indices/aliases, ASan/UBSan')


if __name__=='__main__':main()
