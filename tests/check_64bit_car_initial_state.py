#!/usr/bin/env python3
"""Run six current car bodies on x64 and compare a fixed-field projection.

Car/SceneNode headers and the bodies are taken from this checkout; fixed-point
primitives use the existing portable header. The 32-bit projection belongs to
the test oracle only, never to the runtime code. All fifteen Car pointers are
native addresses and must remain intact. Providers match the x86 harness.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from check_64bit_car_contacts import ROOT, body
from differential_car_initial_state import State, TARGETS

PRELUDE = r'''
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
typedef unsigned char BYTE;
typedef long long __int64;
#include "Car.h"
unsigned short g_sqrtTable[4096];
Car *g_pCurrentCar;
CarSurfaceRamp *g_carSurfaceRamps;
int fixtureSeed;
void Car_UpdateCorners(Car *car) {
    assert(car==g_pCurrentCar);
    for(int i=0;i<8;i++) {
        car->corners[i].x=car->position.x+(i+1)*2*131+fixtureSeed*7;
        car->corners[i].y=car->position.y+(i+1)*3*131+fixtureSeed*7;
        car->corners[i].z=car->position.z+(i+1)*4*131+fixtureSeed*7;
    }
}
void StageObject_UpdateCarCornerGroundHeights(Car *car,int count) {
    assert(car==g_pCurrentCar && count==8);
    for(int i=0;i<8;i++)car->cornerHeight[i]=car->corners[i].y+((fixtureSeed+i)%5-2)*8192;
}
void Car_UpdateGroundNormal() {}
void Car_FlagAirborneCorners() {
    for(int i=0;i<8;i++)g_pCurrentCar->cornerOnGround[i]=(fixtureSeed+i)%2;
}
void Car_LiftFreeCorners(){g_pCurrentCar->position.y+=0x3000+fixtureSeed*128;}
void Car_ShareWeightOnWheels(){for(int i=0;i<4;i++)g_pCurrentCar->wheelLoad[i]=65536+i*8192;}
void Car_InvalidateTransforms(int index){assert(index==(signed char)g_pCurrentCar->index);}
void SceneNode_SetPosition(SceneNode *node,FixVector *position){node->current.position=*position;}
void Car_UpdateSurfaceParams(Car *car,int blend){car->field_0xa78=blend^0x12345678;}
void CarPhysics_IntegrateWheelSuspension() {}
Car *Car_Get(int index){assert(index==fixtureSeed-128);return g_pCurrentCar;}
int StageObject_GetCarWeatherRampValue(Car *);
void StageObject_CopyCarSurfaceNoiseTarget(Car *);
'''

MAIN = r'''
int main(int argc,char **argv) {
    assert(argc==3);fixtureSeed=atoi(argv[2]);
    static_assert(sizeof(void *)==8,"native pointers");
    static_assert(offsetof(Car,pSceneRoot)==0x720,"first native pointer alignment");
    static_assert(offsetof(Car,collisionRadius)==offsetof(Car,pSceneRoot)+15*sizeof(void *),"fifteen runtime pointers");
    static_assert(offsetof(Car,field_0xc20)-offsetof(Car,collisionRadius)==0xc20-0x758,"pointer-free trailing fields");
    BYTE heap[65536];
    assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap));
    Car car;SceneNode node={};FixMatrix world,bodyMatrix;
    CarSurfaceRamp ramps[256];
    memset(&car,0xa5,sizeof(car));
    // Fixture-only translation of the original's two pointer-free spans.
    memcpy(&car,heap+0x1000,0x71c);
    memcpy(&car.collisionRadius,heap+0x1000+0x758,0xc24-0x758);
    memcpy(&world,heap+0x3000,sizeof(world));memcpy(&bodyMatrix,heap+0x4000,sizeof(bodyMatrix));
    memcpy(&node.current,heap+0x5000+0x98,sizeof(node.current));
    memcpy(ramps,heap+0x9000-128*sizeof(CarSurfaceRamp),sizeof(ramps));
    car.pSceneRoot=car.pBodyNode=car.pAlternateBodyNode=&node;
    for(int i=0;i<4;i++)car.pExtraNodes[i]=car.pWheelNodes[i]=&node;
    car.pViewNodeNear=car.pViewNodeFar=&node;
    car.pWorld=fixtureSeed%3==1?&car.physicsMatrix:&world;
    car.pBodyMatrix=fixtureSeed%3==0?&bodyMatrix:fixtureSeed%3==1?&car.bodyMatrix:car.pWorld;
    g_pCurrentCar=&car;g_carSurfaceRamps=&ramps[128];
    BYTE pointers[15*sizeof(void *)];memcpy(pointers,&car.pSceneRoot,sizeof(pointers));
    for(unsigned i=0;i<15;i++) {
        uintptr_t pointer;memcpy(&pointer,pointers+i*sizeof(void *),sizeof(pointer));
        assert(pointer>UINT32_MAX);
    }
    assert(reinterpret_cast<uintptr_t>(g_carSurfaceRamps)>UINT32_MAX);
    for(int i=0;i<4096;i++)g_sqrtTable[i]=static_cast<unsigned short>(std::sqrt((8+16*i)*65536.0));
    int position[3],heading[3];memcpy(position,heap+0x6000,sizeof(position));memcpy(heading,heap+0x6100,sizeof(heading));
    unsigned result=0;
    if(!strcmp(argv[1],"place"))Car_PlaceAtStart(position,heading);
    else if(!strcmp(argv[1],"reset"))Car_ResetBodyBasis(&car);
    else if(!strcmp(argv[1],"travel"))Car_IntegrateWheelTravel();
    else if(!strcmp(argv[1],"ramp"))result=StageObject_GetCarWeatherRampValue(&car);
    else if(!strcmp(argv[1],"noise"))StageObject_CopyCarSurfaceNoiseTarget(&car);
    else {assert(!strcmp(argv[1],"effect"));result=StageObject_IsWheelOnActiveEffectSurface(fixtureSeed-128,fixtureSeed%8);}
    assert(!memcmp(pointers,&car.pSceneRoot,sizeof(pointers)));
    assert(g_pCurrentCar==&car && g_carSurfaceRamps==&ramps[128]);
    memcpy(heap+0x1000,&car,0x71c);
    memcpy(heap+0x1000+0x758,&car.collisionRadius,0xc24-0x758);
    memcpy(heap+0x3000,&world,sizeof(world));memcpy(heap+0x4000,&bodyMatrix,sizeof(bodyMatrix));
    memcpy(heap+0x5000+0x98,&node.current,sizeof(node.current));
    assert(!memcmp(heap+0x9000-128*sizeof(CarSurfaceRamp),ramps,sizeof(ramps)));
    assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));
    assert(fwrite(&result,1,sizeof(result),stdout)==sizeof(result));
}
'''


def main():
    source = ROOT/'CMR2Decomp'
    car = (source/'Car.cpp').read_text(encoding='latin1')
    objects = (source/'StageObjects.cpp').read_text(encoding='latin1')
    ramp = re.search(r'struct CarSurfaceRamp \{.*?\n\};', (source/'StageWeather.h').read_text(), re.S).group()
    functions = '\n'.join(body(car,name) for name in ('Car_PlaceAtStart','Car_ResetBodyBasis','Car_IntegrateWheelTravel'))
    functions += '\n'+body(objects,'StageObject_GetCarWeatherRampValue')
    functions += '\n'+body(objects,'StageObject_CopyCarSurfaceNoiseTarget')
    # The shared extractor accepts int/void; BYTE is restored after extraction.
    functions += '\n'+body(objects.replace('BYTE StageObject_IsWheelOnActiveEffectSurface(',
                                           'int StageObject_IsWheelOnActiveEffectSurface('),
                           'StageObject_IsWheelOnActiveEffectSurface').replace(
                               'int StageObject_IsWheelOnActiveEffectSurface(',
                               'BYTE StageObject_IsWheelOnActiveEffectSurface(')
    oracle = State(ROOT/'cmr2bin/CMR2.exe')
    env = dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-car-state-') as directory:
        tmp = Path(directory)
        for name in ('Car.h','SceneNode.h','Sector.h','LayoutChecks.h'):
            shutil.copyfile(source/name,tmp/name)
        shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
        (tmp/'Graphics.h').write_text('struct Graphics;struct Texture;class CGraphics{public:static const double m_oneOver65536;};\n')
        (tmp/'Mesh.h').write_text('struct Mesh;\n')
        fixture=tmp/'state.cpp';fixture.write_text(ramp+PRELUDE+functions+MAIN)
        executable=tmp/'state'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fwrapv','-fno-strict-aliasing',
                        '-fsanitize=address,undefined',str(fixture),'-o',str(executable)],check=True)
        count=0
        for kind in TARGETS:
            # Boundaries of the signed index, all normal/heading/alias modes,
            # and all eight wheel indices, with poisoned unknown fields.
            seeds=list(range(48))+[63,64,127,128,129,255,256,511,767,1023,1279]
            for seed in seeds:
                out,result,_=oracle.run(kind,seed)
                process=subprocess.run([str(executable),kind,str(seed)],input=oracle.native_input,
                                       capture_output=True,env=env)
                assert process.returncode==0,(kind,seed,process.stderr.decode())
                expected=out+result.to_bytes(4,'little')
                if process.stdout!=expected:
                    at=next(i for i,(a,b) in enumerate(zip(process.stdout,expected)) if a!=b)
                    raise AssertionError((kind,seed,'native projection mismatch',hex(at-0x1000)))
                count+=1
    print(f'PASS: six actual bodies, {count} x64 scenarios, all fifteen pointers above 4 GB intact; complete primitive heap matches original, ASan/UBSan')


if __name__=='__main__':main()
