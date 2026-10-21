#!/usr/bin/env python3
"""Actual native car-glow update and helpers versus complete original x86 runs.

Translate only fixture pointers and pointer-free spans. Production bodies use
real Car, CarPartSet, Mesh, GlowLight and channel-state headers on native x64.
Weather/surface providers are controlled identically to the differential test.
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

from check_64bit_car_part_tables import body
from differential_car_light_state import GlowUpdate, ROOT, glow_update_cases

PRELUDE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
using BYTE=unsigned char;using DWORD=unsigned int;using __int64=long long;using INT_PTR=intptr_t;
#include "Car.h"
#include "CarParts.h"
#include "CarLightState.h"
#include "Glow.h"
#include "MeshVertex.h"
#include "X87Single.h"
CarPartSet *g_carPartSets,*g_carPartSetLookup[8];
CarLightProfile *g_carLightProfiles[8];CarLightPoint *g_carLightPoints[8];
GlowLight *g_carLightGlowSlots[160];CarLightTextureState g_carLightTextureStates[8];
int g_carDamageModelEnabled[8],g_carDamageHazardTimer[8],g_stageObjectTimeStep;
int ramp;Car *expectedCar;unsigned weatherCalls,surfaceCalls;
int StageObject_GetCarWeatherRampValue(Car *car) {
    assert(car==expectedCar && weatherCalls==surfaceCalls);++weatherCalls;return ramp;
}
int Surface_GetTransitionBlend(int surface,int value) {
    assert(value==ramp && weatherCalls==surfaceCalls+1);++surfaceCalls;return surface*997+value;
}
'''

MAIN = r'''
int main() {
    static_assert(sizeof(void *)==8,"native pointers");
    static_assert(sizeof(DeformVertex)==32 && sizeof(CarLightPoint)==40 && sizeof(MeshVertexF)==48,"pointer-free source records");
    static_assert(sizeof(CarLightTextureState)-offsetof(CarLightTextureState,levels)==64,"pointer-free channel suffix");
    static_assert(offsetof(Car,field_0xc20)-offsetof(Car,collisionRadius)==0xc20-0x758,"pointer-free car tail");
    BYTE heap[65536],records[584],timers[32];uint32_t present;int enabled;
    assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap));
    assert(fread(records,1,sizeof(records),stdin)==sizeof(records));
    assert(fread(timers,1,sizeof(timers),stdin)==sizeof(timers));
    assert(fread(&present,4,1,stdin)==1 && fread(&ramp,4,1,stdin)==1 && fread(&enabled,4,1,stdin)==1 && fread(&g_stageObjectTimeStep,4,1,stdin)==1);
    Car car;memset(&car,0xa5,sizeof(car));
    memcpy(&car,heap+0x1000,0x71c);memcpy(&car.collisionRadius,heap+0x1000+0x758,0xc24-0x758);
    Car beforeCar=car;expectedCar=&car;int carIndex=car.index;
    assert(carIndex>=0 && carIndex<8 && reinterpret_cast<uintptr_t>(&car)>UINT32_MAX);
    CarPartSet sets[8];memset(sets,0x96,sizeof(sets));
    for(int i=0;i<8;i++) {
        memcpy(&sets[i].centres,heap+0x2000+i*0x4d0+0xb4,0x4d0-0xb4);
        g_carPartSetLookup[i]=&sets[i];
        memcpy(&g_carLightTextureStates[i].levels,records+4+i*72+8,64);
    }
    g_carPartSets=sets;g_carDamageModelEnabled[carIndex]=enabled;
    memcpy(g_carDamageHazardTimer,timers,32);
    CarLightProfile profile;memcpy(&profile.count,heap+0x6000,4);
    CarLightPoint points[20],beforePoints[20];memcpy(points,heap+0x6100,sizeof(points));memcpy(beforePoints,points,sizeof(points));
    g_carLightProfiles[carIndex]=&profile;g_carLightPoints[carIndex]=points;
    Mesh meshes[2];memset(meshes,0x78,sizeof(meshes));
    MeshVertexF floats[2][4];DeformVertex vertices[2][4];
    for(int mesh=0;mesh<2;mesh++) {
        memcpy(floats[mesh],heap+0x8000+mesh*0x200,sizeof(floats[mesh]));
        memcpy(vertices[mesh],heap+0x9000+mesh*0x200,sizeof(vertices[mesh]));
        meshes[mesh].pVertexData=reinterpret_cast<DWORD *>(floats[mesh]);
        sets[carIndex].geometry.meshes[mesh]=&meshes[mesh];sets[carIndex].geometry.vertices[mesh]=vertices[mesh];
    }
    CarPartSet beforeSets[8];Mesh beforeMeshes[2];MeshVertexF beforeFloats[2][4];DeformVertex beforeVertices[2][4];
    memcpy(beforeSets,sets,sizeof(sets));memcpy(beforeMeshes,meshes,sizeof(meshes));memcpy(beforeFloats,floats,sizeof(floats));memcpy(beforeVertices,vertices,sizeof(vertices));
    GlowLight glows[20],beforeGlows[20];memset(glows,0x97,sizeof(glows));
    for(int i=0;i<20;i++) {
        memcpy(&glows[i],heap+0x5000+i*92,0x48);
        memcpy(&glows[i].enabled,heap+0x5000+i*92+0x50,4);memcpy(&glows[i].field_0x58,heap+0x5000+i*92+0x58,4);
        glows[i].pTexture=nullptr;glows[i].pLayerTexture=nullptr;glows[i].pNode=nullptr;
        g_carLightGlowSlots[carIndex*20+i]=present&(1u<<i)?&glows[i]:nullptr;
    }
    memcpy(beforeGlows,glows,sizeof(glows));CarLightTextureState beforeStates[8];memcpy(beforeStates,g_carLightTextureStates,sizeof(beforeStates));
    CarLight_UpdateFlagsAndGlows(&car);
    assert(!memcmp(&car,&beforeCar,sizeof(car)) && !memcmp(sets,beforeSets,sizeof(sets)) && !memcmp(points,beforePoints,sizeof(points)));
    assert(!memcmp(meshes,beforeMeshes,sizeof(meshes)) && !memcmp(floats,beforeFloats,sizeof(floats)) && !memcmp(vertices,beforeVertices,sizeof(vertices)));
    unsigned calls=0;
    for(int i=0;i<20;i++) {
        bool touched=i<profile.count && (present&(1u<<i));
        if(touched) {
            beforeGlows[i].enabled=glows[i].enabled;
            if(!car.field_0xb70) {
                if(enabled){beforeGlows[i].pos=glows[i].pos;beforeGlows[i].dir=glows[i].dir;}
                beforeGlows[i].planePoint=glows[i].planePoint;beforeGlows[i].planeNormal=glows[i].planeNormal;
                beforeGlows[i].intensity=glows[i].intensity;beforeGlows[i].layerIntensity=glows[i].layerIntensity;++calls;
            }
        }
        assert(!memcmp(&glows[i],&beforeGlows[i],sizeof(GlowLight)));
        memcpy(heap+0x5000+i*92,&glows[i],0x48);memcpy(heap+0x5000+i*92+0x50,&glows[i].enabled,4);memcpy(heap+0x5000+i*92+0x58,&glows[i].field_0x58,4);
    }
    assert(weatherCalls==calls && surfaceCalls==calls);
    for(int i=0;i<8;i++) {
        beforeStates[i].channelMasks[0]=g_carLightTextureStates[i].channelMasks[0];beforeStates[i].channelMasks[1]=g_carLightTextureStates[i].channelMasks[1];
        assert(!memcmp(&beforeStates[i],&g_carLightTextureStates[i],sizeof(CarLightTextureState)));
        memcpy(records+4+i*72+8,&g_carLightTextureStates[i].levels,64);
    }
    assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap) && fwrite(records,1,sizeof(records),stdout)==sizeof(records) && fwrite(g_carDamageHazardTimer,1,32,stdout)==32);
}
'''


def main():
    source=ROOT/'CMR2Decomp'
    oracle=GlowUpdate(ROOT/'cmr2bin/CMR2.exe')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-glow-update-') as directory:
        tmp=Path(directory)
        for name in ('FloatToInt.h','X87Single.h','Car.h','CameraRecord.h','SceneNode.h','Sector.h',
                     'CarParts.h','DeformGeometry.h','CarImpact.h','CarInfo.h','LayoutChecks.h',
                     'Mesh.h','MeshVertex.h','Glow.h','CarLightState.h'):
            shutil.copyfile(source/name,tmp/name)
        shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
        (tmp/'Graphics.h').write_text('struct Graphics;struct Texture;class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0,m_65536=65536.0;};\n')
        (tmp/'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;\n')
        functions=[]
        for file,names in {
            'StageTiming.cpp':('CarPart_GetSet','CarLight_GetBrokenFlags'),
            'Graphics.cpp':('Glow_SetEnabled','Glow_SetIntensity','Glow_NoOpEntryCallback','Glow_SetLayerPlane','Glow_SetPosition'),
            'StageObjects.cpp':('Mesh_ReadVertexFixed','CarDamage_GetModelEnabled','CarLight_SetChannelMasks','CarLight_GetScaledChannelLevels','CarLight_UpdateFlagsAndGlows'),
        }.items():
            text=(source/file).read_text();functions.extend(body(text,name) for name in names)
        cpp=tmp/'glows.cpp';cpp.write_text(PRELUDE+'\n'.join(functions)+MAIN)
        exe=tmp/'glows'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing',
                        '-DCMR2_QIFIST','-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast',
                        '-fsanitize=address,undefined',str(cpp),'-o',str(exe)],check=True)
        env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
        cases=0
        for i,args in enumerate(glow_update_cases()):
            result=oracle.update(*args,20000+i)
            expected=b''.join(result[:3])
            native=subprocess.run([str(exe)],input=oracle.native_input,capture_output=True,env=env)
            assert native.returncode==0,(args,native.stderr.decode())
            assert native.stdout==expected,(args,[(hex(i),a,b) for i,(a,b) in enumerate(zip(native.stdout,expected)) if a!=b][:12])
            cases+=1
    print(f'PASS: {cases} actual native glow updates vs original x86; twelve production bodies, real headers, pointers >4 GB, channel aliases, conversion ties, full heap/record guards; ASan/UBSan')


if __name__=='__main__':
    main()
