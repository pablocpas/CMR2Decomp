#!/usr/bin/env python3
"""Compare actual ride-height/spring bodies with original x86 on native x64.

The fixture alone translates the original Car's pointer-free spans; production
uses real headers, native fields and pointers. The cheat query is the only leaf.
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

from check_64bit_car_contacts import ROOT, body
from differential_car_spring_state import Springs, cases

PRELUDE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
typedef unsigned char BYTE;typedef unsigned int DWORD;typedef long long __int64;
#include "Car.h"
Car *g_pCurrentCar;
int g_physicsTimeStep,g_physicsScale,cheat,queries;
class CGameInfo {public:static int IsActiveCheatEnabled(int code) {assert(code==6);++queries;return cheat;}};
'''

MAIN = r'''
int main(int argc,char **argv) {
    assert(argc==6);cheat=atoi(argv[3]);g_physicsTimeStep=atoi(argv[4]);g_physicsScale=atoi(argv[5]);
    static_assert(sizeof(void *)==8 && sizeof(int)==4,"native pointer/fixed-word sizes");
    static_assert(offsetof(Car,pSceneRoot)==0x720,"native pointer alignment");
    static_assert(offsetof(Car,collisionRadius)==offsetof(Car,pSceneRoot)+15*sizeof(void *),"native pointer span");
    static_assert(offsetof(Car,wheelSpringDamping)==offsetof(Car,wheelSpringStiffness)+4*sizeof(int),"adjacent independent arrays");
    BYTE heap[65536];assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap));
    Car car;SceneNode node={};FixMatrix world;
    memset(&car,0xa5,sizeof(car));
    memcpy(&car,heap+0x1000,0x71c);
    memcpy(&car.collisionRadius,heap+0x1000+0x758,0xc24-0x758);
    memcpy(&world,heap+0x3000,sizeof(world));
    car.pSceneRoot=car.pBodyNode=car.pAlternateBodyNode=&node;
    for(int i=0;i<4;i++)car.pExtraNodes[i]=car.pWheelNodes[i]=&node;
    car.pViewNodeNear=car.pViewNodeFar=&node;
    uint32_t originalWorld;memcpy(&originalWorld,heap+0x1000+0x750,4);
    car.pWorld=originalWorld==0x10001000?&car.physicsMatrix:&world;
    car.pBodyMatrix=&world;g_pCurrentCar=&car;
    Car before;memcpy(&before,&car,sizeof(car));
    for(int i=0;i<15;i++) {
        uintptr_t pointer;memcpy(&pointer,(BYTE *)&car.pSceneRoot+i*sizeof(void *),sizeof(pointer));
        assert(pointer>UINT32_MAX);
    }
    if(!strcmp(argv[1],"setter"))Car_SetRideHeight(atoi(argv[2]));
    else {assert(!strcmp(argv[1],"integrate"));CarPhysics_IntegrateWheelSuspension();}
    assert(g_pCurrentCar==&car);
    assert(!memcmp((BYTE *)&car+0x71c,(BYTE *)&before+0x71c,offsetof(Car,collisionRadius)-0x71c));
    size_t tail=offsetof(Car,field_0xc20)+sizeof(int);
    assert(!memcmp((BYTE *)&car+tail,(BYTE *)&before+tail,sizeof(Car)-tail));
    memcpy(heap+0x1000,&car,0x71c);
    memcpy(heap+0x1000+0x758,&car.collisionRadius,0xc24-0x758);
    memcpy(heap+0x3000,&world,sizeof(world));
    assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));
    assert(fwrite(&queries,4,1,stdout)==1);
}
'''


def main():
    source = ROOT/'CMR2Decomp'
    oracle = Springs(ROOT/'cmr2bin/CMR2.exe')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-springs-') as directory:
        tmp = Path(directory)
        for name in ('FloatToInt.h','X87Single.h','Car.h','CameraRecord.h','SceneNode.h','Sector.h','LayoutChecks.h'):
            shutil.copyfile(source/name,tmp/name)
        shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
        (tmp/'Graphics.h').write_text('struct Graphics;struct Texture;class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0;};\n')
        (tmp/'Mesh.h').write_text('struct Mesh;\n')
        (tmp/'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;\n')
        fixed = (source/'FixedPoint.cpp').read_text()
        fixture = tmp/'springs.cpp'
        fixture.write_text(PRELUDE+body(fixed,'FixMatrix_InverseRotateVector')+'\n'+
                           body((source/'Car.cpp').read_text(),'Car_SetRideHeight')+'\n'+
                           body(fixed,'CarPhysics_IntegrateWheelSuspension')+MAIN)
        exe = tmp/'springs'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast',
                        '-fsanitize=address,undefined','-I'+str(tmp),str(fixture),'-o',str(exe)],check=True)
        for i,(method,args) in enumerate(cases()):
            out,trace = getattr(oracle,method)(*args,3001+i)
            setting,cheat = args[:2] if method=='setter' else (0,0)
            step,scale = args[-2:] if method=='integrate' else (0,0)
            process = subprocess.run([str(exe),method,str(setting),str(cheat),str(step),str(scale)],
                input=oracle.native_input,capture_output=True,
                env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
            assert process.returncode==0,(method,args,process.stderr.decode())
            expected = out+len(trace).to_bytes(4,'little')
            if process.stdout!=expected:
                at=next(i for i,(a,b) in enumerate(zip(process.stdout,expected)) if a!=b)
                raise AssertionError((method,args,'original / native projection',hex(at-0x1000)))
    print(f'PASS: {len(cases())} native spring/setup cases; three production bodies, real layouts and all fifteen '
          'car pointers >4 GB intact; complete projected heap and cheat calls equal original, ASan/UBSan')


if __name__=='__main__':
    main()
