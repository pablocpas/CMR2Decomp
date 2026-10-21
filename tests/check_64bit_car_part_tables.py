#!/usr/bin/env python3
"""Execute car-part lifecycle, visibility, attached-node and damage resets on x64.

Use the production Car/CarPartSet/SceneNode layouts and pointers above 4 GB.
Allocation, release, callback registration and damage recomputation are leaves;
check native allocation sizes, nested free order, lookup aliases and full guards.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from check_64bit_car_contacts import ROOT


def body(source, name):
    match = re.search(r'^[\w *]+\b' + name + r'\([^;]*?\)\s*\{',source,re.M)
    assert match, name
    pos, depth = match.end(), 1
    while depth:
        depth += (source[pos] == '{') - (source[pos] == '}')
        pos += 1
    return source[match.start():pos]

PRELUDE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <vector>
typedef unsigned char BYTE;typedef unsigned int DWORD;typedef long long __int64;
typedef intptr_t INT_PTR;
#include "Car.h"
#include "CarParts.h"
#include "StageTiming.h"
int g_carPartSetCount;
CarPartSet *g_carPartSets,*g_carPartSetLookup[8];
CarDamageRecord *g_carDamageRecords;
DeformVertex ***g_carOriginalPartVertices;
BYTE **g_carOriginalPartNodeTypes;
SceneNode *g_carPartNodes[4][8];
BYTE g_carPartModelIndices[4][8];
CarPartStateTables g_carPartStateTables;
Car *g_partCar;PartState *g_partState;
int g_deformPulseTicks[2],g_deformImpactTicks[2];
int g_carPartTableCarCount;
void CarPart_MarkHidden(Car *,int);
alignas(16) BYTE pool[4][16384];
std::vector<size_t> sizes;std::vector<void *> freed;
int registered,rebuilt;Car *expectedCar;int expectedSlot;
int CarPart_FreeTables(void);
struct CFileBuffer {
    static void *AllocateLockedBuffer(size_t size) {
        assert(sizes.size()<4);void *p=pool[sizes.size()];sizes.push_back(size);return p;
    }
    static void FreeGenericFileBuffer(void *p) {
        assert(reinterpret_cast<uintptr_t>(p)>UINT32_MAX);freed.push_back(p);
    }
};
struct CGame {
    static void RegisterCallback(int (*callback)(void),void *context) {
        assert(callback==CarPart_FreeTables && !context);++registered;
    }
};
void StageObject_RebuildDamagePartValues(Car *car) {
    assert(car==expectedCar && g_carPartSets[car->index].partHidden[expectedSlot]==1);++rebuilt;
}
'''

MAIN = r'''
int main() {
    static_assert(sizeof(void *)==8,"native pointers");
    static_assert(sizeof(CarPartSet)>0x4d0,"native geometry tables grow");
    assert(reinterpret_cast<uintptr_t>(pool)>UINT32_MAX);
    static CarPartSet sets[8],aliases[8],sentinel;
    static CarDamageRecord damage[8];
    static SceneNode nodes[4][8];
    static DeformVertex vertexData[8][15];
    unsigned cases=0;
    for(int count:{-1,0,1,8}) {
        memset(pool,0xa5,sizeof(pool));BYTE expectedPool[sizeof(pool)];memcpy(expectedPool,pool,sizeof(pool));
        for(auto &p:g_carPartSetLookup)p=&sentinel;
        sizes.clear();registered=0;CarPart_AllocateTables(count);
        std::vector<size_t> wanted={size_t(count)*sizeof(CarPartSet),size_t(count)*sizeof(CarDamageRecord),size_t(count)*sizeof(DeformVertex **),size_t(count)*sizeof(BYTE *)};
        assert(sizes==wanted && registered==1 && g_carPartSetCount==count);
        assert(g_carPartSets==reinterpret_cast<CarPartSet *>(pool[0]) && g_carDamageRecords==reinterpret_cast<CarDamageRecord *>(pool[1]));
        assert(g_carOriginalPartVertices==reinterpret_cast<DeformVertex ***>(pool[2]) && g_carOriginalPartNodeTypes==reinterpret_cast<BYTE **>(pool[3]));
        for(int i=0;i<8;i++)assert(g_carPartSetLookup[i]==(i<count?&g_carPartSets[i]:&sentinel));
        for(int table:{2,3})memset(expectedPool+table*sizeof(pool[0]),0,size_t(count>0?count:0)*sizeof(void *));
        assert(!memcmp(pool,expectedPool,sizeof(pool)));++cases;
    }
    for(int count:{-1,0,1,8})for(unsigned mask=0;mask<4;mask++)for(unsigned seed=0;seed<4;seed++) {
        memset(sets,seed+1,sizeof(sets));memset(damage,seed+2,sizeof(damage));
        DeformVertex *vertices[8][15],*expectedVertices[8][15];
        DeformVertex **tables[8],**expectedTables[8];BYTE types[8][15],*typeTables[8],*expectedTypeTables[8];
        memset(types,0x76,sizeof(types));std::vector<void *> wanted;
        for(int i=0;i<8;i++) {
            int counts[]={-1,0,1,15};sets[i].count=counts[(i+seed)%4];
            tables[i]=(i+seed)%3?vertices[i]:nullptr;typeTables[i]=(i+seed)%2?types[i]:nullptr;
            for(int j=0;j<15;j++)vertices[i][j]=(i+j+seed)%2?&vertexData[i][j]:nullptr;
        }
        memcpy(expectedVertices,vertices,sizeof(vertices));memcpy(expectedTables,tables,sizeof(tables));memcpy(expectedTypeTables,typeTables,sizeof(typeTables));
        CarPartSet beforeSets[8];memcpy(beforeSets,sets,sizeof(sets));CarDamageRecord beforeDamage[8];memcpy(beforeDamage,damage,sizeof(damage));
        if(mask&1) {
            for(int i=0;i<count;i++)if(tables[i]) {
                for(int j=0;j<sets[i].count;j++)if(vertices[i][j]){wanted.push_back(vertices[i][j]);expectedVertices[i][j]=nullptr;}
                wanted.push_back(tables[i]);expectedTables[i]=nullptr;
            }
            wanted.push_back(tables);
        }
        if(mask&2) {
            for(int i=0;i<count;i++)if(typeTables[i]){wanted.push_back(typeTables[i]);expectedTypeTables[i]=nullptr;}
            wanted.push_back(typeTables);
        }
        wanted.push_back(sets);wanted.push_back(damage);
        g_carPartSets=sets;g_carDamageRecords=damage;g_carPartSetCount=count;
        g_carOriginalPartVertices=mask&1?tables:nullptr;g_carOriginalPartNodeTypes=mask&2?typeTables:nullptr;freed.clear();
        assert(CarPart_FreeTables()==1 && freed==wanted && !g_carPartSets && !g_carDamageRecords && !g_carOriginalPartVertices && !g_carOriginalPartNodeTypes && !g_carPartSetCount);
        assert(!memcmp(vertices,expectedVertices,sizeof(vertices)) && !memcmp(tables,expectedTables,sizeof(tables)) && !memcmp(typeTables,expectedTypeTables,sizeof(typeTables)));
        assert(!memcmp(sets,beforeSets,sizeof(sets)) && !memcmp(damage,beforeDamage,sizeof(damage)));
        for(auto &row:types)for(auto b:row)assert(b==0x76);++cases;
    }
    for(int carIndex=0;carIndex<8;carIndex++)for(unsigned present:{0u,1u,5u,15u})for(unsigned seed=0;seed<4;seed++) {
        Car car;memset(&car,0x45,sizeof(car));car.index=carIndex;Car beforeCar=car;
        memset(sets,0x73,sizeof(sets));memset(aliases,0x84,sizeof(aliases));memset(nodes,0x97,sizeof(nodes));
        int flags[]={0,1,-1,INT32_MIN};
        for(int i=0;i<8;i++) {
            g_carPartSetLookup[i]=&aliases[i];
            for(int slot=0;slot<4;slot++) {
                sets[i].partHidden[slot]=flags[(slot+seed)%4];aliases[i].partHidden[slot]=flags[(slot+seed+1)%4];
                g_carPartNodes[slot][i]=present&(1u<<slot)?&nodes[slot][i]:nullptr;
                g_carPartModelIndices[slot][i]=BYTE(i*37+slot*97+seed);
            }
        }
        g_carPartSets=sets;CarPartSet beforeSets[8],beforeAliases[8];memcpy(beforeSets,sets,sizeof(sets));memcpy(beforeAliases,aliases,sizeof(aliases));
        SceneNode expectedNodes[4][8];memcpy(expectedNodes,nodes,sizeof(nodes));
        assert(CarPart_GetSet(carIndex)==&aliases[carIndex]);
        for(int slot=0;slot<4;slot++) {
            assert(CarPart_GetHiddenState(&car,slot)==sets[carIndex].partHidden[slot]);
            assert(CarPart_GetModelIndex(slot,carIndex)==g_carPartModelIndices[slot][carIndex]);
            assert(CarPart_GetNode(&car,slot)==g_carPartNodes[slot][carIndex]);
            int hidden=sets[carIndex].partHidden[slot];CarPart_HideNode(&car,slot,hidden);
            if(hidden && present&(1u<<slot))expectedNodes[slot][carIndex].viewMask=0;
        }
        assert(!memcmp(nodes,expectedNodes,sizeof(nodes)));
        memset(nodes,0x97,sizeof(nodes));memcpy(expectedNodes,nodes,sizeof(nodes));CarPart_ApplyHiddenStates(&car);
        for(int slot=0;slot<4;slot++)if(aliases[carIndex].partHidden[slot] && present&(1u<<slot))expectedNodes[slot][carIndex].viewMask=0;
        assert(!memcmp(nodes,expectedNodes,sizeof(nodes)));
        expectedCar=&car;expectedSlot=seed;rebuilt=0;CarPart_MarkHidden(&car,expectedSlot);beforeSets[carIndex].partHidden[expectedSlot]=1;
        assert(rebuilt==1 && !memcmp(sets,beforeSets,sizeof(sets)) && !memcmp(aliases,beforeAliases,sizeof(aliases)) && !memcmp(&car,&beforeCar,sizeof(car)));++cases;
        PartState part;memset(&part,0x63,sizeof(part));part.flags=BYTE((seed<<4)|seed|8);
        PartState beforePart=part;beforePart.flags&=0xfe;g_partState=&part;g_partCar=&car;rebuilt=0;
        CarPart_DeactivateAndMarkHidden();
        assert(rebuilt==1 && !memcmp(&part,&beforePart,sizeof(part)) && !memcmp(sets,beforeSets,sizeof(sets)));++cases;
        PartState states[4][8],expectedStates[4][8];memset(states,0x74,sizeof(states));
        for(int slot=0;slot<4;slot++)for(int i=0;i<8;i++) {
            states[slot][i].pNode=present&(1u<<slot)?&nodes[slot][i]:nullptr;
            memset(&nodes[slot][i].local,0x31+slot,sizeof(FixMatrix));
            g_carPartStateTables.parts[slot]=states[slot];
        }
        memcpy(expectedStates,states,sizeof(states));memcpy(expectedNodes,nodes,sizeof(nodes));
        for(int slot=0;slot<4;slot++)if(states[slot][carIndex].pNode) {
            expectedNodes[slot][carIndex].current=nodes[slot][carIndex].local;expectedStates[slot][carIndex].pNode=nullptr;
        }
        std::srand(400);int expectedRandom=std::rand();g_partCar=nullptr;CarPart_ResetAttachedNodes(&car);
        assert(g_partCar==&car && std::rand()==expectedRandom && !memcmp(nodes,expectedNodes,sizeof(nodes)) && !memcmp(states,expectedStates,sizeof(states)) && !memcmp(&car,&beforeCar,sizeof(car)));++cases;
    }
    for(int count:{-1,0,1,8}) {
        static_assert(sizeof(CarDamageRecord)==0x290,"pointer-free damage record");
        memset(damage,0x71,sizeof(damage));CarDamageRecord expectedDamage[8];memcpy(expectedDamage,damage,sizeof(damage));
        for(int i=0;i<count;i++) {
            BYTE *bytes=reinterpret_cast<BYTE *>(&expectedDamage[i]);
            bytes[0x20a]=bytes[0x20b]=0;
            for(int link=0;link<20;link++)bytes[0x112+link*13]=0xff;
            memset(bytes+0x24c,0,34);memset(bytes+0x270,0,28);
            memcpy(bytes,bytes+0x106,0x106);memcpy(bytes+0x20c,bytes+0x24c,0x40);
        }
        g_carDamageRecords=damage;g_carPartSetCount=count;memset(g_deformPulseTicks,0x73,sizeof(g_deformPulseTicks));memset(g_deformImpactTicks,0x84,sizeof(g_deformImpactTicks));
        CarDamage_ResetAllRecords();assert(!memcmp(damage,expectedDamage,sizeof(damage)));
        for(int i=0;i<2;i++)assert(!g_deformPulseTicks[i] && !g_deformImpactTicks[i]);++cases;
    }
    for(int first:{0,1,7})for(int count:{0,1}) {
        memset(sets,0x69,sizeof(sets));CarPartSet expectedSets[8];memcpy(expectedSets,sets,sizeof(sets));
        memset(expectedSets+first,0,count*sizeof(CarPartSet));g_carPartSets=sets;CarPart_ClearSetRange(first,count);
        assert(!memcmp(sets,expectedSets,sizeof(sets)));++cases;
    }
    for(int count:{-1,0,1,8}) {
        PartState states[4][8],expectedStates[4][8];memset(states,0x72,sizeof(states));memcpy(expectedStates,states,sizeof(states));
        for(int slot=0;slot<4;slot++) {
            g_carPartStateTables.parts[slot]=states[slot];g_carPartStateTables.modes[slot]=0x80+slot;
            memset(expectedStates[slot],0,(count>0?count:0)*sizeof(PartState));
            for(int i=0;i<8;i++){g_carPartNodes[slot][i]=&nodes[slot][i];g_carPartModelIndices[slot][i]=0x73;}
        }
        g_carPartTableCarCount=count;CarPart_ResetMovingStateTables();
        assert(!memcmp(states,expectedStates,sizeof(states)));
        for(int slot=0;slot<4;slot++) {
            assert(g_carPartStateTables.parts[slot]==states[slot] && g_carPartStateTables.modes[slot]==0x80+slot);
            for(int i=0;i<8;i++)assert(!g_carPartNodes[slot][i] && g_carPartModelIndices[slot][i]==0xff);
        }
        ++cases;
    }
    printf("PASS: %u native car-part cases; fifteen production bodies, real layouts/pointers >4 GB, allocation sizes, release order, aliases, hidden flags, attached nodes/RNG, damage/moving-table resets and full guards; ASan/UBSan\n",cases);
}
'''


def main():
    source = ROOT/'CMR2Decomp'
    with tempfile.TemporaryDirectory(prefix='cmr2-native-part-tables-') as directory:
        tmp = Path(directory)
        for name in ('FloatToInt.h','X87Single.h','Car.h','CameraRecord.h','SceneNode.h',
                     'Sector.h','CarParts.h','DeformGeometry.h','CarImpact.h','CarInfo.h','LayoutChecks.h',
                     'StageTiming.h','StageWeatherParticle.h'):
            shutil.copyfile(source/name,tmp/name)
        shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
        (tmp/'Graphics.h').write_text('struct Graphics;struct Texture;class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0;};\n')
        (tmp/'Mesh.h').write_text('struct Mesh;\n')
        (tmp/'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;\n')
        functions = []
        for file,names in {
            'StageTiming.cpp':('CarPart_FreeTables','CarPart_AllocateTables','CarPart_GetSet','CarPart_GetHiddenState',
                               'CarDamage_ResetRecord','CarDamage_ResetAllRecords','CarPart_ClearSetRange','CarPart_DeactivateAndMarkHidden','CarPart_ResetMovingStateTables'),
            'StageObjects.cpp':('CarPart_HideNode','CarPart_GetModelIndex','CarPart_GetNode','CarPart_ApplyHiddenStates','CarPart_MarkHidden','CarPart_ResetAttachedNodes'),
        }.items():
            text=(source/file).read_text()
            functions.extend(body(text,name) for name in names)
        fixture=tmp/'parts.cpp';fixture.write_text(PRELUDE+'\n'.join(functions)+MAIN)
        exe=tmp/'parts'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast',
                        '-fsanitize=address,undefined','-I'+str(tmp),str(fixture),'-o',str(exe)],check=True)
        result=subprocess.run([str(exe)],capture_output=True,
                              env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
        assert result.returncode==0,result.stderr.decode()
        print(result.stdout.decode(),end='')


if __name__=='__main__':
    main()
