#!/usr/bin/env python3
"""Execute both current surface bodies with native Car layout and real tables.

Original x86 outputs supply the oracle. The primitive projection is fixture
code; runtime bodies use actual members/offsetof/sizeof and preserve all fifteen
native pointers. Surface tables are read from the original executable.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from check_64bit_car_contacts import ROOT, body
from differential_surface_contacts import Contacts, TABLES, COMPRESSIONS
from differential_surface_params import Surfaces
from differential_menu_list import HEAP


class OriginalSurfaces(Surfaces):
    def invoke(self,address,args):
        self.native_input=self.read(HEAP,65536)
        return super().invoke(address,args)


PRELUDE=r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
typedef unsigned char BYTE;
typedef long long __int64;
#include "Car.h"
'''

MAIN=r'''
int main(int argc,char **argv) {
    assert(argc==3);static_assert(sizeof(void *)==8,"native pointers");
    BYTE heap[65536];assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap));
    Car car;SceneNode node={};FixMatrix matrix={};
    memset(&car,0xa5,sizeof(car));
    static_assert(offsetof(Car,pSceneRoot)==0x720,"aligned native pointer block");
    static_assert(offsetof(Car,collisionRadius)==offsetof(Car,pSceneRoot)+15*sizeof(void *),"fifteen pointers");
    static_assert(offsetof(Car,field_0xc20)-offsetof(Car,collisionRadius)==0xc20-0x758,"fixed trailing fields");
    memcpy(&car,heap+0x1000,0x71c);memcpy(&car.collisionRadius,heap+0x1000+0x758,0xc24-0x758);
    car.pSceneRoot=car.pBodyNode=car.pAlternateBodyNode=&node;
    for(int i=0;i<4;i++)car.pExtraNodes[i]=car.pWheelNodes[i]=&node;
    car.pViewNodeNear=car.pViewNodeFar=&node;car.pWorld=car.pBodyMatrix=&matrix;
    BYTE pointers[15*sizeof(void *)];memcpy(pointers,&car.pSceneRoot,sizeof(pointers));
    for(unsigned i=0;i<15;i++) {
        uintptr_t value;memcpy(&value,pointers+i*sizeof(void *),sizeof(value));assert(value>UINT32_MAX);
    }
    assert(offsetof(Car,wheelSurface)!=0xaae && offsetof(Car,surfaceNoiseTarget)!=0xa78);
    if(!strcmp(argv[1],"paired"))Surface_BlendWheelContactParameters(&car,atoi(argv[2]));
    else {assert(!strcmp(argv[1],"blended"));Car_UpdateSurfaceParams(&car,atoi(argv[2]));}
    assert(!memcmp(pointers,&car.pSceneRoot,sizeof(pointers)));
    memcpy(heap+0x1000,&car,0x71c);memcpy(heap+0x1000+0x758,&car.collisionRadius,0xc24-0x758);
    assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));
}
'''


def main():
    source=ROOT/'CMR2Decomp'
    text=(source/'SurfaceTypes.cpp').read_text(encoding='latin1')
    functions=text[text.index('// Typed views of the original interior cursors.'):text.index('// Rebuilds the per-corner grip')]
    functions+='\n'+body(text,'Car_UpdateSurfaceParams')+'\n'+body(text,'Surface_BlendWheelContactParameters')
    paired=Contacts(ROOT/'cmr2bin/CMR2.exe');blended=OriginalSurfaces(ROOT/'cmr2bin/CMR2.exe')
    names={'grip':('g_surfaceGrip','int','48][2'), 'grip2':('g_surfaceGrip2','int','48][2'),
           'soft':('g_surfaceSoftness','int','48][2'), 'drag':('g_surfaceDrag','int','63'),
           'drag_index':('g_surfaceDragIndex','BYTE','48'), 'effect':('g_surfaceEffect','BYTE','48][2'),
           'noise':('g_surfaceNoise','BYTE','48'), 'f0':('g_surface0x51e678','int','48'),
           'f1':('g_surface0x51e738','int','48'), 'f2':('g_surface0x51e2b8','int','48'),
           'f3':('g_surface0x51e4f8','int','48'), 'f4':('g_surface0x51e5b8','int','48')}
    tables=''
    for key,(name,type,extent) in names.items():
        tables+=type+' '+name+'['+extent+']={'+','.join(map(str,paired.tables[key]))+'};\n'
    tables+='BYTE g_surfaceNext[48]={'+','.join(map(str,paired.read(paired.addr(0x51e8e8),48)))+'};\n'
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-surfaces-') as directory:
        tmp=Path(directory)
        for name in ('Car.h','SceneNode.h','Sector.h','LayoutChecks.h'):
            shutil.copyfile(source/name,tmp/name)
        shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
        (tmp/'Graphics.h').write_text('struct Graphics;struct Texture;class CGraphics{public:static const double m_oneOver65536;};\n')
        (tmp/'Mesh.h').write_text('struct Mesh;\n')
        fixture=tmp/'surfaces.cpp';fixture.write_text(PRELUDE+tables+functions+MAIN)
        executable=tmp/'surfaces'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast',
                        '-fsanitize=address,undefined',str(fixture),'-o',str(executable)],check=True)
        count=0
        def compare(kind,value,input,expected):
            nonlocal count
            process=subprocess.run([str(executable),kind,str(value)],input=input,capture_output=True,env=env)
            assert process.returncode==0,(kind,count,process.stderr.decode())
            if process.stdout!=expected:
                at=next(i for i,(a,b) in enumerate(zip(process.stdout,expected)) if a!=b)
                raise AssertionError((kind,count,'native/original',hex(at-0x1000)))
            count+=1
        for surface in range(48):
            for i,comp in enumerate(COMPRESSIONS):
                compression=tuple(comp[j%2] for j in range(4))
                seed=surface*6+i
                out=paired.run(surface,compression,(surface+i)%7,(-0x3334,-0x3333,0,0x3333,0x3334,65536)[i],seed)
                compare('paired',seed*31337,paired.native_input,out)
            for i,blend in enumerate((0,0x4000,0xffff,0x10000)):
                out=blended.run_surface(surface,blend,(0,0x8000,0x10000,0x18000)[surface%4],(0,3,6)[(surface+i)%3])
                compare('blended',blend,blended.native_input,out)
    print(f'PASS: two actual surface bodies, {count} x64 cases, original tables/math; complete primitive heap and fifteen pointers above 4 GB preserved, ASan/UBSan')


if __name__=='__main__':main()
