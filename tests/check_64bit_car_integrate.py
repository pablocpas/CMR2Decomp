#!/usr/bin/env python3
"""Run the actual rigid-body integrator and geometry helpers on native x64.

Only this fixture translates the original x86 Car's pointer-free spans.
Production bodies and headers operate on native Car/CarPartSet pointers;
compare the complete projected heap against original execution under Unicorn.
"""
import itertools
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from check_64bit_car_contacts import body
from differential_car_step import Integrator, ROOT

PRELUDE = r'''
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef long long __int64;
#include "Car.h"
#include "CarParts.h"
Car *g_pCurrentCar;
CarPartSet *g_physicsPartSet;
int g_physicsTimeStep, g_dragBase = 65536;
unsigned short g_sqrtTable[4096];
#define FIX_ABS(x) ((x) < 0 ? -(x) : (x))
void Car_UpdateCorners(Car *);
void Car_ApplyCornerOffsets(void);
void FixBasis_Integrate(FixVector *,FixVector *);
'''

MAIN = r'''
int main() {
    static_assert(sizeof(void *)==8,"native pointers");
    static_assert(offsetof(Car,pSceneRoot)==0x720,"native first-pointer alignment");
    static_assert(offsetof(Car,collisionRadius)==offsetof(Car,pSceneRoot)+15*sizeof(void *),"native pointer span");
    static_assert(offsetof(Car,bodyAxisVectors)==0x360,"pointer-free body-axis span");
    static_assert(sizeof(((Car *)0)->bodyAxisVectors)==36,"three vector rows");
    static_assert(offsetof(Car,field_0xc20)-offsetof(Car,collisionRadius)==0xc20-0x758,"pointer-free tail");
    BYTE heap[65536];
    assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap));
    assert(fread(&g_physicsTimeStep,4,1,stdin)==1);
    Car car={};CarPartSet parts={};FixMatrix world;
    memcpy(&car,heap+0x1000,0x71c);
    memcpy(&car.collisionRadius,heap+0x1000+0x758,0xc24-0x758);
    memcpy(&world,heap+0x7000,sizeof(world));
    memcpy(&parts.bodyDamageDrag,heap+0x5000+0x408,4);
    uint32_t originalWorld;memcpy(&originalWorld,heap+0x1000+0x750,4);
    assert(originalWorld==0x10001000 || originalWorld==0x10007000);
    bool alias=originalWorld==0x10001000;
    car.pWorld=alias?&car.physicsMatrix:&world;
    g_pCurrentCar=&car;g_physicsPartSet=&parts;
    assert((uintptr_t)car.pWorld>UINT32_MAX && (uintptr_t)g_physicsPartSet>UINT32_MAX);
    Car before;CarPartSet beforeParts;
    memcpy(&before,&car,sizeof(car));memcpy(&beforeParts,&parts,sizeof(parts));
    for(int i=0;i<4096;i++)g_sqrtTable[i]=(unsigned short)std::sqrt((8+16*i)*65536.0);
    Car_Integrate();
    assert(g_pCurrentCar==&car && g_physicsPartSet==&parts);
    assert(!memcmp(&parts,&beforeParts,sizeof(parts)));
    Car expected;memcpy(&expected,&before,sizeof(expected));
    memcpy(expected.corners,car.corners,sizeof(car.corners));
    expected.position=car.position;
    memcpy(expected.bodyAxisVectors,car.bodyAxisVectors,sizeof(car.bodyAxisVectors));
    expected.velocity=car.velocity;expected.velocityNext=car.velocityNext;
    expected.angularVelocity=car.angularVelocity;expected.speed=car.speed;
    expected.field_0xb60=car.field_0xb60;
    if(alias)expected.physicsMatrix=car.physicsMatrix;
    assert(!memcmp(&car,&expected,sizeof(car)));
    memcpy(heap+0x1000,&car,0x71c);
    memcpy(heap+0x1000+0x758,&car.collisionRadius,0xc24-0x758);
    if(!alias)memcpy(heap+0x7000,&world,sizeof(world));
    assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));
}
'''


def main():
    source = ROOT / 'CMR2Decomp'
    oracle = Integrator(ROOT / 'cmr2bin/CMR2.exe')
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-integrate-') as directory:
        tmp = Path(directory)
        for name in ('FloatToInt.h','X87Single.h','Car.h','CameraRecord.h','SceneNode.h',
                     'Sector.h','CarParts.h','DeformGeometry.h','CarImpact.h','CarInfo.h','LayoutChecks.h'):
            shutil.copyfile(source/name,tmp/name)
        shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
        (tmp/'Graphics.h').write_text('struct Graphics;struct Texture;class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0;};\n')
        (tmp/'Mesh.h').write_text('struct Mesh;\n')
        (tmp/'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;\n')
        car = (source/'Car.cpp').read_text()
        fixed = (source/'FixedPoint.cpp').read_text()
        macro = re.search(r'#define ADD_POSITION[\s\S]*?\n\n',car)[0]
        functions = '\n'.join(body(fixed,name) for name in ('FixMatrix_InverseRotateVector','FixBasis_Integrate'))
        functions += '\n' + '\n'.join(body(car,name) for name in ('Car_Integrate','Car_UpdateCorners','Car_ApplyCornerOffsets'))
        fixture = tmp/'integrate.cpp';fixture.write_text(PRELUDE+macro+functions+MAIN)
        exe = tmp/'integrate'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast',
                        '-fsanitize=address,undefined',str(fixture),'-o',str(exe)],check=True)
        import struct
        count = 0
        for case in itertools.product((0,1),range(3),(0,1),(0,1),(0,1),(0,16384,65536),(0,17)):
            expected = oracle.integrate(*case)
            result = subprocess.run([str(exe)],input=oracle.native_input+struct.pack('<i',case[5]),
                                    capture_output=True,env=env)
            assert result.returncode==0,(case,result.stderr.decode())
            assert result.stdout==expected,(case,[(hex(i),a,b) for i,(a,b) in enumerate(zip(result.stdout,expected)) if a!=b][:12])
            count += 1
    print(f'PASS: {count} native rigid-body integrations vs original x86; five production bodies, full guarded car/heap/setup, native pointers, matrix alias/padding and ASan/UBSan')


if __name__ == '__main__':
    main()
