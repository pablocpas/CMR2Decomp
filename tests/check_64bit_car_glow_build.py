#!/usr/bin/env python3
"""Run the actual glow builder and allocator on native pointers vs original x86.

Production structs and bodies stay native; only fixture addresses and the
serialized pointer-free spans are projected to the original's guarded heap.
CIN, texture setup and scene creation are controlled asset leaves.
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

from check_64bit_car_part_tables import body
from check_64bit_car_glow_update import PRELUDE
from differential_car_glow_build import BuildGlows,ROOT,HEAP,REGIONS,build_cases

EXTRA = r'''
SceneNode *g_carPartNodes[4][8];
Texture *g_carLightTexA[5],*g_carLightTexB[5];
FixVector g_rearViewLightOffsetA,g_rearViewLightOffsetB;
SceneNode *g_rearViewLightNodeA,*g_rearViewLightNodeB;
int g_rearViewLightAttenuation;
CarLightPoint *g_carExhaustPoints[8][2];GlowLight *g_carExhaustGlows[8][2];
GlowLight *g_glowEntries;int g_glowCapacity,g_glowAllocatedCount;
struct FixtureProfile {int count;CarLightPoint points[20];} profile;
CarLightProfile dummyProfile;CarLightPoint dummyPoint;GlowLight dummyGlow;
SceneNode bodyNode,partNodes[4],rearNodes[2],parentNode,dummyNode;
BYTE textureTokens[10];unsigned sceneCalls,textureCalls,parentCalls;
CarLightProfile *profileView(){return reinterpret_cast<CarLightProfile *>(&profile);}
void *CarInfo_GetSection(Car *car,int section){assert(car==expectedCar && section==0);return profileView();}
SceneNode *RallyData_GetChallengeRenderState(){++parentCalls;return &parentNode;}
SceneNode *Scene_CreateLight(int type,int r,int g,int b,FixVector *p,FixAngles *a,SceneNode *parent) {
    assert(type==0 && r==0x140000 && r==g && r==b && !p->x && !p->y && !p->z);
    assert(!a->x && !a->y && static_cast<unsigned short>(a->z)==0xff1d && !a->pad && parent==&parentNode && sceneCalls<2);
    return &rearNodes[sceneCalls++];
}
void Frontend_SetObjectField118(Texture *texture,int value) {
    assert(value==1);bool found=false;
    for(int i=0;i<10;i++)if(texture==reinterpret_cast<Texture *>(&textureTokens[i]))found=true;
    assert(found);++textureCalls;
}
uint32_t textureAddress(Texture *p) {
    for(int i=0;i<10;i++)if(p==reinterpret_cast<Texture *>(&textureTokens[i]))return HEAP_BASE+0xc000+i*64;
    assert(false);return 0;
}
uint32_t nodeAddress(SceneNode *p) {
    if(!p)return 0;if(p==&bodyNode)return HEAP_BASE+0xa000;if(p==&dummyNode)return HEAP_BASE+0xe000;
    for(int i=0;i<4;i++)if(p==&partNodes[i])return HEAP_BASE+0xb000+i*0x200;
    for(int i=0;i<2;i++)if(p==&rearNodes[i])return HEAP_BASE+0xd000+i*0x200;
    assert(false);return 0;
}
GlowLight pool[24];
uint32_t glowAddress(GlowLight *p) {
    if(!p)return 0;if(p==&dummyGlow)return HEAP_BASE+0xe000;
    for(int i=0;i<24;i++)if(p==&pool[i])return HEAP_BASE+0x5000+i*92;
    assert(false);return 0;
}
uint32_t pointAddress(CarLightPoint *p) {
    if(!p)return 0;if(p==&dummyPoint)return HEAP_BASE+0xe000;
    for(int i=0;i<20;i++)if(p==&profile.points[i])return HEAP_BASE+0x6004+i*40;
    assert(false);return 0;
}
void word(BYTE *p,uint32_t v){memcpy(p,&v,4);}
'''

MAIN = r'''
int main() {
    static_assert(sizeof(void *)==8 && sizeof(CarPartSet)>0x4d0,"native pointer tables");
    static_assert(sizeof(CarLightPoint)==40 && sizeof(DeformVertex)==32,"serialized pointer-free spans");
    BYTE heap[65536],globals[GLOBALS_BYTES];int enabled;unsigned present;
    assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap) && fread(globals,1,sizeof(globals),stdin)==sizeof(globals));
    assert(fread(&enabled,4,1,stdin)==1 && fread(&g_glowCapacity,4,1,stdin)==1 && fread(&present,4,1,stdin)==1);
    memcpy(&g_glowAllocatedCount,globals+GLOBALS_BYTES-4,4);
    Car car;memset(&car,0x73,sizeof(car));memcpy(&car,heap+0x1000,0x71c);memcpy(&car.collisionRadius,heap+0x1000+0x758,0xc24-0x758);
    car.pBodyNode=&bodyNode;Car beforeCar=car;expectedCar=&car;int carIndex=car.index;
    assert(carIndex>=0 && carIndex<8 && reinterpret_cast<uintptr_t>(&car)>UINT32_MAX && reinterpret_cast<uintptr_t>(pool)>UINT32_MAX);
    memcpy(&profile,heap+0x6000,sizeof(profile));FixtureProfile expectedProfile=profile;
    CarPartSet sets[8],beforeSets[8];memset(sets,0x64,sizeof(sets));DeformVertex vertices[3][4],beforeVertices[3][4];
    for(int i=0;i<8;i++) {
        memcpy(&sets[i].centres,heap+0x2000+i*0x4d0+0xb4,0x4d0-0xb4);g_carPartSetLookup[i]=&sets[i];
        g_carLightProfiles[i]=&dummyProfile;g_carLightPoints[i]=&dummyPoint;
        for(int side=0;side<2;side++){g_carExhaustPoints[i][side]=&dummyPoint;g_carExhaustGlows[i][side]=&dummyGlow;}
    }
    for(int mesh=0;mesh<3;mesh++){memcpy(vertices[mesh],heap+0x7000+mesh*0x300,sizeof(vertices[mesh]));sets[carIndex].geometry.vertices[mesh]=vertices[mesh];}
    memcpy(beforeSets,sets,sizeof(sets));memcpy(beforeVertices,vertices,sizeof(vertices));
    g_carDamageModelEnabled[carIndex]=enabled;
    for(int i=0;i<160;i++)g_carLightGlowSlots[i]=&dummyGlow;
    for(int slot=0;slot<4;slot++)g_carPartNodes[slot][carIndex]=present&(1u<<slot)?&partNodes[slot]:nullptr;
    for(int i=0;i<5;i++){g_carLightTexA[i]=reinterpret_cast<Texture *>(&textureTokens[i]);g_carLightTexB[i]=reinterpret_cast<Texture *>(&textureTokens[i+5]);}
    memset(&g_rearViewLightOffsetA,0x71,sizeof(FixVector));memset(&g_rearViewLightOffsetB,0x71,sizeof(FixVector));
    g_rearViewLightNodeA=&dummyNode;g_rearViewLightNodeB=&dummyNode;g_rearViewLightAttenuation=0x71717171;memset(g_carDamageHazardTimer,0x71,sizeof(g_carDamageHazardTimer));
    memset(pool,0x81,sizeof(pool));GlowLight beforePool[24];
    for(int i=0;i<24;i++) {
        memcpy(&pool[i],heap+0x5000+i*92,0x48);memcpy(&pool[i].enabled,heap+0x5000+i*92+0x50,4);memcpy(&pool[i].field_0x58,heap+0x5000+i*92+0x58,4);
        pool[i].pTexture=nullptr;pool[i].pLayerTexture=nullptr;pool[i].pNode=nullptr;
    }
    memcpy(beforePool,pool,sizeof(pool));g_glowEntries=pool;CarLight_BuildPointGlows(&car);
    assert(!memcmp(&car,&beforeCar,sizeof(car)) && !memcmp(sets,beforeSets,sizeof(sets)) && !memcmp(vertices,beforeVertices,sizeof(vertices)));
    assert(parentCalls==(carIndex==0?2u:0u) && sceneCalls==parentCalls && textureCalls==2u*unsigned(profile.count>0?profile.count:0));
    for(int i=0;i<20;i++)if(enabled && i<profile.count) {
        expectedProfile.points[i].object=profile.points[i].object;expectedProfile.points[i].vertex=profile.points[i].vertex;
    }
    assert(!memcmp(&profile,&expectedProfile,sizeof(profile)));memcpy(heap+0x6000,&profile,sizeof(profile));
    for(int i=0;i<24;i++) {
        bool allocated=beforePool[i].type==0 && pool[i].type==2;
        GlowLight expected=beforePool[i];
        if(allocated) {
            expected.type=pool[i].type;expected.pos=pool[i].pos;expected.dir=pool[i].dir;
            expected.sizeX=pool[i].sizeX;expected.sizeY=pool[i].sizeY;expected.intensity=pool[i].intensity;expected.projectedSizeScale=pool[i].projectedSizeScale;
            expected.pTexture=pool[i].pTexture;expected.pLayerTexture=pool[i].pLayerTexture;expected.pNode=pool[i].pNode;
            expected.enabled=pool[i].enabled;expected.projectedBrightness=pool[i].projectedBrightness;
            word(heap+0x5000+i*92+0x48,textureAddress(pool[i].pTexture));word(heap+0x5000+i*92+0x4c,textureAddress(pool[i].pLayerTexture));word(heap+0x5000+i*92+0x54,nodeAddress(pool[i].pNode));
        }
        assert(!memcmp(&pool[i],&expected,sizeof(GlowLight)));
        memcpy(heap+0x5000+i*92,&pool[i],0x48);memcpy(heap+0x5000+i*92+0x50,&pool[i].enabled,4);memcpy(heap+0x5000+i*92+0x58,&pool[i].field_0x58,4);
    }
    size_t off=0;
    for(int i=0;i<8;i++) {
        assert(g_carLightProfiles[i]==profileView() || g_carLightProfiles[i]==&dummyProfile);
        word(globals+off+4*i,g_carLightProfiles[i]==profileView()?HEAP_BASE+0x6000:HEAP_BASE+0xe000);
    }
    off+=32;
    for(int i=0;i<8;i++)word(globals+off+4*i,pointAddress(g_carLightPoints[i]));off+=32;
    for(int i=0;i<160;i++)word(globals+off+4*i,glowAddress(g_carLightGlowSlots[i]));off+=640;
    for(int i=0;i<8;i++)for(int side=0;side<2;side++)word(globals+off+4*(i*2+side),pointAddress(g_carExhaustPoints[i][side]));off+=64;
    for(int i=0;i<8;i++)for(int side=0;side<2;side++)word(globals+off+4*(i*2+side),glowAddress(g_carExhaustGlows[i][side]));off+=64;
    memcpy(globals+off,&g_rearViewLightOffsetA,12);off+=12;memcpy(globals+off,&g_rearViewLightOffsetB,12);off+=12;
    word(globals+off,nodeAddress(g_rearViewLightNodeA));off+=4;word(globals+off,nodeAddress(g_rearViewLightNodeB));off+=4;
    memcpy(globals+off,&g_rearViewLightAttenuation,4);off+=4;memcpy(globals+off,g_carDamageHazardTimer,32);off+=32;memcpy(globals+off,&g_glowAllocatedCount,4);off+=4;
    assert(off==sizeof(globals));
    assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap) && fwrite(globals,1,sizeof(globals),stdout)==sizeof(globals));
}
'''


def main():
    source=ROOT/'CMR2Decomp';oracle=BuildGlows(ROOT/'cmr2bin/CMR2.exe')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-glow-build-') as directory:
        tmp=Path(directory)
        for name in ('FloatToInt.h','X87Single.h','Car.h','CameraRecord.h','SceneNode.h','Sector.h','CarParts.h',
                     'DeformGeometry.h','CarImpact.h','CarInfo.h','LayoutChecks.h','Mesh.h','MeshVertex.h','Glow.h','CarLightState.h'):
            shutil.copyfile(source/name,tmp/name)
        shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
        (tmp/'Graphics.h').write_text('struct Graphics;struct Texture;class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0,m_65536=65536.0;};\n')
        (tmp/'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;\n')
        functions=[]
        for file,names in {
            'StageTiming.cpp':('CarPart_GetSet','CarExhaust_SetPoint','CarExhaust_SetGlow'),
            'Graphics.cpp':('Glow_FindFreeSlot','Glow_Add','Glow_SetEnabled'),
            'StageObjects.cpp':('CarDamage_GetModelEnabled','CarPart_GetNode','CarLight_BuildPointGlows'),
        }.items():
            text=(source/file).read_text();functions.extend(body(text,name) for name in names)
        code=(PRELUDE+EXTRA+'\n'.join(functions)+MAIN).replace('HEAP_BASE',str(HEAP)).replace('GLOBALS_BYTES',str(sum(n for _,n in REGIONS)))
        cpp=tmp/'build.cpp';cpp.write_text(code);exe=tmp/'build'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast','-fsanitize=address,undefined',str(cpp),'-o',str(exe)],check=True)
        env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1');cases=0
        for i,args in enumerate(build_cases()):
            original=oracle.build(*args,30000+i);expected=original[0]+original[1]
            result=subprocess.run([str(exe)],input=oracle.native_input,capture_output=True,env=env)
            assert result.returncode==0,(args,result.stderr.decode())
            assert result.stdout==expected,(args,[(hex(i),a,b) for i,(a,b) in enumerate(zip(result.stdout,expected)) if a!=b][:12]);cases+=1
    print(f'PASS: {cases} native glow builds vs original x86; nine production bodies, native pointers/layouts, real allocation/exhaust links, full heap/globals and padding guards; ASan/UBSan')


if __name__=='__main__':main()
