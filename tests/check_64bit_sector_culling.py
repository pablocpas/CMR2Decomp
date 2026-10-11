#!/usr/bin/env python3
"""Native culling with production records: immutable bounds, full records and one negative control."""
from pathlib import Path
import sys,shutil,subprocess,tempfile
sys.path.insert(0, str(Path(__file__).resolve().parent))
from check_64bit_car_contacts import body
root=Path(__file__).resolve().parents[1];port=root.parent/'OpenCMR2'
main=r'''
int main(){
 for(int i=0;i<4096;i++)g_sqrtTable[i]=(unsigned short)sqrt((8+16*i)*65536.0);
 const int rows=8,columns=8,count=64,size=10;
 g_sectorsPerRow=columns;g_sectorRows=rows;g_sectorCount=count;g_sectorSize=size*65536;g_sectorHalfSize=size*32768;g_sectorScale=65536/size;
 auto *cells=(Sector *)calloc(count,sizeof(Sector));assert((uintptr_t)cells>UINT32_MAX);
 for(int r: {0,3,4,7})for(int c:{0,3,4,7})for(int yaw=0;yaw<8;yaw++)for(int radius:{20,50,100}){
  for(int i=0;i<count;i++){
   Sector &s=cells[i];memset(&s,0xa5,sizeof(s));s.x=(i%columns)*size*65536;s.y=0;s.z=-(i/columns)*size*65536;
   int loX=s.x-g_sectorHalfSize,hiX=s.x+g_sectorHalfSize,loZ=s.z-g_sectorHalfSize,hiZ=s.z+g_sectorHalfSize;
   int rect[4][2]={{loX,loZ},{hiX,loZ},{hiX,hiZ},{loX,hiZ}};memcpy(s.bounds,rect,sizeof(rect));g_sectors[i]=&s;
  }
  SceneNode node={};node.world.position={c*size*65536,0,-r*size*65536};
  double angle=yaw*3.141592653589793/4;node.world.forward={int(llround(sin(angle)*65536)),0,int(llround(cos(angle)*65536))};
  CGraphics::m_farPlaneFixed=radius*65536;
  Sector original[count];memcpy(original,cells,sizeof(original));
  unsigned short previous[count];int previousCount=-1;
  for(int pass=0;pass<3;pass++){
   Sector_CullGridAroundViewNode(&node,NULL);
   if(pass){assert(previousCount==g_sectorCullEnabled);assert(!memcmp(previous,g_visibleSectorIndices,previousCount*sizeof(short)));}
   previousCount=g_sectorCullEnabled;memcpy(previous,g_visibleSectorIndices,previousCount*sizeof(short));
   bool marked[count]={};for(int j=0;j<g_sectorCullEnabled;j++){int id=g_visibleSectorIndices[j];assert(id>=0&&id<count&&!marked[id]);marked[id]=true;}
   for(int i=0;i<count;i++){
    assert(!memcmp(original[i].bounds,cells[i].bounds,sizeof(cells[i].bounds)));
    Sector expected=original[i];if(marked[i]){
     int dx=expected.x-node.world.position.x,dz=expected.z-node.world.position.z;
     expected.viewDistanceSquared=int((int64_t(dx)*dx>>16)+(int64_t(dz)*dz>>16));expected.field_0x80=0;expected.field_0x84=1;
    }
    assert(!memcmp(&expected,&cells[i],sizeof(expected)));assert(bool(g_sectorVisibleBits[i>>5]&(1u<<(i&31)))==marked[i]);
   }
  }
 }
 free(cells);puts("PASS 384 native culling cases, three passes each: stable bounds, full records, masks and sector lists");
}
'''
pre=r'''
#include "port/types.h"
#include "Sector.h"
#include "SceneNode.h"
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <initializer_list>
#include <cstdint>
unsigned short g_sqrtTable[4096];int g_sectorsPerRow,g_sectorHalfSize,g_sectorSize,g_sectorRows,g_sectorScale,g_sectorCount,g_sectorCullEnabled,g_sectorCullDisabled,g_sectorOriginX,g_sectorOriginZ;
Sector *g_sectors[14096];int g_sectorVisibleBits[128];unsigned short g_visibleSectorIndices[4096];
class CGraphics{public:static int m_farPlaneFixed;};int CGraphics::m_farPlaneFixed;
'''
sector=(root/'CMR2Decomp/Sector.cpp').read_text();fixed=(root/'CMR2Decomp/FixedPoint.cpp').read_text();gfx=(root/'CMR2Decomp/Graphics.cpp').read_text()
code=pre+'\n'.join([body(fixed,n) for n in ('FixMatrix_GetPosition','FixMatrix_GetForward')]+[body(gfx,'Tri2D_Contains')]+[body(sector,n) for n in ('Sector_FromPosition','Sector_CullGridAroundViewNode')])+main
with tempfile.TemporaryDirectory(prefix='cmr2-sector-native-') as temp:
 d=Path(temp)
 for name in ['Sector.h','SceneNode.h','Mesh.h','LayoutChecks.h']:
  shutil.copyfile(root/'CMR2Decomp'/name,d/name)
 shutil.copyfile(port/'game/FixedPoint.h',d/'FixedPoint.h')
 old=code.replace('g_sectors[index]->viewDistanceSquared','*(int *)((BYTE *)g_sectors[index] + 0x7c)').replace('g_sectors[index]->field_0x80','*(int *)((BYTE *)g_sectors[index] + 0x80)').replace('g_sectors[index]->field_0x84','*(int *)((BYTE *)g_sectors[index] + 0x84)')
 for name,source in [('original-offsets',old),('typed-fields',code)]:
  (d/(name+'.cpp')).write_text(source)
  subprocess.run(['clang++','-std=c++17','-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fms-extensions','-Wno-everything','-I'+str(d),'-I'+str(port/'src'),str(d/(name+'.cpp')),'-o',str(d/name)],check=True)
  r=subprocess.run([str(d/name)],capture_output=True,text=True);print(name,r.returncode,r.stdout,r.stderr[-600:]);assert (r.returncode==0)==(name=='typed-fields')
