#!/usr/bin/env python3
"""Execute both production car-step schedulers with native runtime pointers.

Compare their complete fixed-field heap projection against original x86
execution. Physics/terrain providers match differential_car_step; matrix and
fixed-point helpers remain real. Pointer spans and native padding are guarded.
"""
import itertools
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

from check_64bit_car_contacts import ROOT, body
from differential_car_step import Step, Preparation

PRELUDE = r'''
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
typedef unsigned char BYTE;typedef unsigned int DWORD;typedef long long __int64;
#include "Car.h"
#include "CarParts.h"
Car *g_pCurrentCar;
CarPartSet *g_physicsPartSet;
int g_gravityScale,g_unk0x0053c9d4;
FixVector g_carStepAccel;
unsigned short g_sqrtTable[4096];
CarPartSet parts[8];
void Car_BalanceTwoPlayerRideHeight(Car *,short *,short) {}
int StageObject_GetCarWeatherRampValue(Car *) {return 0;}
void Car_UpdateSurfaceParams(Car *,int) {}
void Car_ScaleRatesByTimeStep(Car *) {}
void AutoGear_UpdateCarGearState(Car *) {}
CarPartSet *CarPart_GetSet(int index) {assert(index>=0 && index<8);return &parts[index];}
short Sector_GetNeighbours(FixVector *,short *out) {out[0]=17;out[1]=23;out[2]=31;return 5;}
void CarDamage_StepClimbingCarRecords(short *,short) {}
void Car_UpdateCorners(Car *) {}
void StageObject_UpdateCarCornerGroundHeights(Car *,int count) {assert(count==8);}
#define LEAF(name) void name() {}
LEAF(Car_StartPendingCountdown)
LEAF(Car_UpdateLowSpeedWheelLoadTimer)
void CarEffects_UpdateBrokenLightFlicker(Car *) {}
LEAF(Car_UpdateCornerVelocities)
LEAF(Car_ShareWeightOnWheels)
LEAF(Car_UpdateSuspension)
LEAF(Car_UpdateSlopeGrip)
LEAF(Car_UpdateCornerLoads)
LEAF(Car_UpdateSteering)
LEAF(Car_UpdateEngineTorque)
LEAF(Car_UpdateCornerFriction)
LEAF(Car_IntegrateContacts)
LEAF(Car_UpdateEngineSpeed)
LEAF(Car_UpdateResponseValueCountdown)
LEAF(Car_FlagAirborneCorners)
LEAF(Car_UpdateGroundNormal)
LEAF(Car_LiftOutOfGround)
LEAF(Car_IntegrateWheelTravel)
LEAF(Car_StoreBodyMatrix)
'''

MAIN = r'''
int main(int argc,char **argv) {
    assert(argc==3);int count=atoi(argv[2]);assert(count>=-1 && count<=8);
    static_assert(sizeof(void *)==8,"native pointers");
    static_assert(offsetof(Car,pSceneRoot)==0x720,"native pointer alignment");
    static_assert(offsetof(Car,collisionRadius)==offsetof(Car,pSceneRoot)+15*sizeof(void *),"native pointer span");
    BYTE heap[65536];assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap));
    assert(fread(&g_gravityScale,4,1,stdin)==1);
    assert(fread(&g_unk0x0053c9d4,4,1,stdin)==1);
    assert(fread(&g_carStepAccel,12,1,stdin)==1);
    static Car cars[8];static SceneNode node;
    FixMatrix worlds[8];short order[8];
    memcpy(worlds,heap+0x8000,sizeof(worlds));memcpy(order,heap+0x9000,sizeof(order));
    memset(cars,0xa5,sizeof(cars));memset(parts,0xa5,sizeof(parts));
    for(int i=0;i<count;i++) {
        memcpy(&cars[i],heap+0x1000+i*0xc24,0x71c);
        memcpy(&cars[i].collisionRadius,heap+0x1000+i*0xc24+0x758,0xc24-0x758);
        memcpy(parts[i].centres,heap+0xa000+i*0x500+0xb4,0x4d0-0xb4);
        cars[i].pSceneRoot=cars[i].pBodyNode=cars[i].pAlternateBodyNode=&node;
        for(int k=0;k<4;k++)cars[i].pExtraNodes[k]=cars[i].pWheelNodes[k]=&node;
        cars[i].pViewNodeNear=cars[i].pViewNodeFar=&node;
        uint32_t originalWorld;memcpy(&originalWorld,heap+0x1000+i*0xc24+0x750,4);
        assert(originalWorld>=0x10008000 && originalWorld<0x10008200 && !(originalWorld%64));
        cars[i].pWorld=&worlds[(originalWorld-0x10008000)/64];
        cars[i].pBodyMatrix=&worlds[i];
    }
    Car before[8];CarPartSet beforeParts[8];
    memcpy(before,cars,sizeof(cars));memcpy(beforeParts,parts,sizeof(parts));
    for(int i=0;i<4096;i++)g_sqrtTable[i]=(unsigned short)std::sqrt((8+16*i)*65536.0);
    if(!strcmp(argv[1],"step"))Car_StepAll(cars,order,(short)count);
    else {assert(!strcmp(argv[1],"prepare"));Car_PrepareStep(cars,order,(short)count);}
    for(int i=0;i<8;i++) {
        BYTE *runtime=(BYTE *)&cars[i],*saved=(BYTE *)&before[i];
        assert(!memcmp(runtime+0x71c,saved+0x71c,offsetof(Car,collisionRadius)-0x71c));
        assert(!memcmp(runtime+offsetof(Car,field_0xc20)+4,saved+offsetof(Car,field_0xc20)+4,
                       sizeof(Car)-offsetof(Car,field_0xc20)-4));
        assert(!memcmp(&parts[i],&beforeParts[i],offsetof(CarPartSet,centres)));
        size_t partTail=offsetof(CarPartSet,brokenLightFlickerPhase)+sizeof(int);
        assert(!memcmp((BYTE *)&parts[i]+partTail,(BYTE *)&beforeParts[i]+partTail,sizeof(CarPartSet)-partTail));
        if(i<count) {
            for(int k=0;k<15;k++) {
                uintptr_t pointer;memcpy(&pointer,runtime+offsetof(Car,pSceneRoot)+k*sizeof(void *),sizeof(pointer));
                assert(pointer>UINT32_MAX);
            }
            memcpy(heap+0x1000+i*0xc24,&cars[i],0x71c);
            memcpy(heap+0x1000+i*0xc24+0x758,&cars[i].collisionRadius,0xc24-0x758);
            memcpy(heap+0xa000+i*0x500+0xb4,parts[i].centres,0x4d0-0xb4);
        } else assert(!memcmp(&cars[i],&before[i],sizeof(Car)) && !memcmp(&parts[i],&beforeParts[i],sizeof(CarPartSet)));
    }
    memcpy(heap+0x8000,worlds,sizeof(worlds));assert(!memcmp(order,heap+0x9000,sizeof(order)));
    uint32_t selected[2]={g_pCurrentCar?uint32_t(0x10001000+(g_pCurrentCar-cars)*0xc24):0,
                          g_physicsPartSet?uint32_t(0x1000a000+(g_physicsPartSet-parts)*0x500):0};
    assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));
    assert(fwrite(selected,4,2,stdout)==2);
    assert(fwrite(&g_unk0x0053c9d4,4,1,stdout)==1);
    assert(fwrite(&g_carStepAccel,12,1,stdout)==1);
}
'''


def main():
    source = ROOT / 'CMR2Decomp'
    oracles = {'step': Step(ROOT/'cmr2bin/CMR2.exe'), 'prepare': Preparation(ROOT/'cmr2bin/CMR2.exe')}
    with tempfile.TemporaryDirectory(prefix='cmr2-native-step-') as directory:
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
        helpers = ('FixMatrix_InverseRotateVector','FixMatrix_GetPosition','FixMatrix_GetRight',
                   'FixMatrix_GetUp','FixMatrix_GetForward','FixMatrix_SetPosition',
                   'FixMatrix_SetRight','FixMatrix_SetUp','FixMatrix_SetForward')
        fixture = tmp/'step.cpp'
        fixture.write_text(PRELUDE+'\n'.join(body(fixed,name) for name in helpers)+'\n'+
                           body(car,'Car_StepAll')+'\n'+body(car,'Car_PrepareStep')+MAIN)
        exe = tmp/'step'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast',
                        '-fsanitize=address,undefined','-I'+str(tmp),str(fixture),'-o',str(exe)],check=True)
        count = 0
        for kind in oracles:
            counts = (0,1,2,8) if kind=='step' else (-1,0,1,2,8)
            for case in itertools.product(counts,range(8),(False,True)):
                oracle = oracles[kind]
                result = getattr(oracle,'run_'+kind)(*case)
                expected = result[0] + (result[1] if kind=='step' else result[1].to_bytes(4,'little')+bytes(20))
                globals_input = oracle.native_globals if kind=='step' else bytes(20)
                process = subprocess.run([str(exe),kind,str(case[0])],input=oracle.native_input+globals_input,
                    capture_output=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
                assert process.returncode==0,(kind,case,process.stderr.decode())
                if process.stdout!=expected:
                    at=next(i for i,(a,b) in enumerate(zip(process.stdout,expected)) if a!=b)
                    raise AssertionError((kind,case,'original / native projection',hex(at-0x1000)))
                count += 1
    print(f'PASS: {count} native scheduler/preparation cases, two production passes and nine real matrix helpers; '
          'complete projected heap/globals equal original, all fifteen car pointers >4 GB intact; ASan/UBSan')


if __name__=='__main__':
    main()
