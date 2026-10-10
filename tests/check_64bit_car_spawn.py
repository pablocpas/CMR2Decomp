#!/usr/bin/env python3
"""Compare the actual native Car_Spawn body with the original's complete heap.

Selection, tuning and model providers use the controlled x86 spawn fixture.
Car and SceneNode are the current runtime headers; numeric offsets below belong
only to the original-image projection. No runtime access uses this projection.
"""
import itertools
import math
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

from check_64bit_car_contacts import ROOT, body
from differential_car_spawn import Spawn, HEAP


class OriginalSpawn(Spawn):
    def provider(self, address, count):
        if self.native_input is None:
            self.native_input = self.read(HEAP, 65536)
            self.u.mem_write(self.addr(0x6e0ef4), self.square_roots)
        super().provider(address, count)

    def run(self, *args):
        self.native_input = None
        return super().run(*args)


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
FixVector g_gravityDir;
int g_gravityScale;
BYTE *fixtureHeap;
int mode,flag25,network,players,slot,special,cheats;
class CGameInfo { public:
 static int GetConfiguredGameMode(){return mode;}
 static int IsConfiguredMultiplayer(){return players;}
 static int GetGameModeOptionBit19(){return network;}
 static int GetSoundOptionBit30(){return special;}
 static int GetGameInfoSessionFlag(){return 0;}
 static int IsActiveCheatEnabled(int bit){return !!(cheats&(1<<bit));}
};
class CFrontend { public:
 static int GetArchivePrimaryFlagEntry(int){return 0;}
 static int GetArchiveSecondaryFlagEntry(int){return 1;}
 static int GetArchivePrimaryIDEntry(int index){return index&255;}
};
int RallyData_GetSelectionFlag28(){return special;}
int StageTiming_GetTotalCarCount(){return 2;}
int Replay_GetSelectionStateByte(){return -1;}
int RallyData_GetDriverRecordSelectionValue(BYTE){return 0;}
int RallyData_GetSelectionFlag26(){return 0;}
int RallyDataState(){return 1;}
int RallyData_GetFlag24(){return 0;}
int RallyData_GetFlag25(){return flag25;}
int RallyData_GetUsableRecordCategory(BYTE){return -1;}
int RallyData_GetDriverOrCategoryFlag(BYTE){return 0;}
int StageUI_GetRaceEndEventCount(){return 0;}
BYTE *RallyData_GetDriverSkillRecord(int index){return fixtureHeap+0x6000+(index&255)*16;}
char *RallyData_GetSelectedSettingSevenByteRecord(){return (char *)fixtureHeap+0x7100;}
char *RallyData_GetCountrySettingSevenByteRecord(){return (char *)fixtureHeap+0x7200;}
char *RallyData_GetCountrySevenByteRecord(){return (char *)fixtureHeap+0x7000;}
void Car_BindModel(int index,Car *car){assert(index==slot && car==g_pCurrentCar);}
void Car_SetRideHeight(int){}
void Car_SetSteeringSwingTarget(int){}
void Car_SetBrakeBias(int){}
void Car_SetDriveSplit(int){}
void Car_SetSurfaceDragLevel(int){}
void Car_SelectGearSpeedTable(unsigned int){}
void Car_SetDifficultyHandling(int){}
void Car_UpdateCorners(Car *car){assert(car==g_pCurrentCar);}
'''
MAIN = r'''
int main(int argc,char **argv) {
 assert(argc==9);mode=atoi(argv[1]);flag25=atoi(argv[2]);network=atoi(argv[3]);players=atoi(argv[4]);
 slot=atoi(argv[5]);special=atoi(argv[6]);int carType=atoi(argv[7]);cheats=atoi(argv[8]);
 static_assert(sizeof(void *)==8,"native pointer width");
 static_assert(offsetof(Car,pSceneRoot)==0x720,"native first pointer");
 static_assert(offsetof(Car,collisionRadius)==offsetof(Car,pSceneRoot)+15*sizeof(void *),"runtime pointer span");
 static_assert(offsetof(SceneNode,current)!=0x98,"matrix follows native pointers");
 BYTE heap[65536];assert(fread(heap,1,sizeof(heap),stdin)==sizeof(heap));fixtureHeap=heap;
 Car car={};SceneNode wheels[4]={},bodyNode={},reference={};FixVector position;
 memcpy(&car,heap+0x1000,0x71c);memcpy(&car.collisionRadius,heap+0x1758,0xc24-0x758);
 for(int i=0;i<4;i++) {
  memcpy(&wheels[i].local,heap+0x3000+i*0x200+0x58,sizeof(FixMatrix));car.pWheelNodes[i]=&wheels[i];
  assert(reinterpret_cast<uintptr_t>(car.pWheelNodes[i])>UINT32_MAX);
 }
 memcpy(&bodyNode.current,heap+0x4000+0x98,sizeof(FixMatrix));car.pBodyNode=&bodyNode;
 memcpy(&reference.current,heap+0x8000+0x98,sizeof(FixMatrix));memcpy(&position,heap+0x9000,sizeof(position));
 assert(reinterpret_cast<uintptr_t>(&car)>UINT32_MAX && reinterpret_cast<uintptr_t>(&reference)>UINT32_MAX);
 assert(reinterpret_cast<uintptr_t>(fixtureHeap)>UINT32_MAX);
 for(int i=0;i<4096;i++)g_sqrtTable[i]=static_cast<unsigned short>(std::sqrt((8+16*i)*65536.0));
 Car_Spawn(&car,&reference,carType,&position,slot,1);
 assert(g_pCurrentCar==&car && car.pBodyNode==&bodyNode);
 for(int i=0;i<4;i++)assert(car.pWheelNodes[i]==&wheels[i]);
 assert(car.pWorld==&car.physicsMatrix && car.pBodyMatrix==&car.bodyMatrix);
 assert(car.pSceneRoot==nullptr && car.pAlternateBodyNode==nullptr);
 for(int i=0;i<4;i++)assert(car.pExtraNodes[i]==nullptr);
 assert(car.pViewNodeNear==nullptr && car.pViewNodeFar==nullptr);
 memcpy(heap+0x1000,&car,0x71c);memcpy(heap+0x1758,&car.collisionRadius,0xc24-0x758);
 // Original-address projection for the two intentionally assigned pointers.
 uint32_t world=0x10000000+0x1000,body=world+0x40;
 memcpy(heap+0x1750,&world,4);memcpy(heap+0x1754,&body,4);
 assert(fwrite(heap,1,sizeof(heap),stdout)==sizeof(heap));
 assert(fwrite(&g_gravityDir,1,sizeof(g_gravityDir),stdout)==sizeof(g_gravityDir));
 assert(fwrite(&g_gravityScale,1,sizeof(g_gravityScale),stdout)==sizeof(g_gravityScale));
}
'''


def main():
    oracle=OriginalSpawn(ROOT/'cmr2bin/CMR2.exe')
    oracle.square_roots=struct.pack('<4096H',*(math.isqrt((8+16*i)*65536) for i in range(4096)))
    source=ROOT/'CMR2Decomp'
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-car-spawn-') as directory:
        tmp=Path(directory)
        for name in ('Car.h','SceneNode.h','Sector.h','LayoutChecks.h'):
            shutil.copyfile(source/name,tmp/name)
        shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h')
        (tmp/'Graphics.h').write_text('struct Graphics;struct Texture;class CGraphics{public:static const double m_oneOver65536;};\n')
        (tmp/'Mesh.h').write_text('struct Mesh;\n')
        fixture=tmp/'spawn.cpp'
        fixed=(source/'FixedPoint.cpp').read_text(encoding='latin1')
        helpers='\n'.join(body(fixed,name) for name in ('FixMatrix_GetPosition','FixMatrix_GetRight','FixMatrix_GetUp','FixMatrix_GetForward'))
        fixture.write_text(PRELUDE+helpers+body((source/'Car.cpp').read_text(encoding='latin1'),'Car_Spawn')+MAIN)
        executable=tmp/'spawn'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast',
                        '-fsanitize=address,undefined',str(fixture),'-o',str(executable)],check=True)
        count=0
        for mode,flag25,network,players,slot,special in ((0,0,0,0,0,0),(2,1,1,1,1,0),
                                                       (4,1,0,1,1,1),(2,1,0,0,1,0),
                                                       (0,1,0,1,1,0),(4,0,1,0,0,0)):
            for car_type,cheats in itertools.product(range(14),(0,64,128,192)):
                args=(mode,flag25,network,players,slot,special,car_type,cheats)
                out,_=oracle.run(*args)
                result=subprocess.run([str(executable),*map(str,args)],input=oracle.native_input,
                                      capture_output=True,env=env)
                assert result.returncode==0,(args,result.stderr.decode())
                expected=out+oracle.read(oracle.addr(0x53cad0),16)
                if result.stdout!=expected:
                    at=next(i for i,(a,b) in enumerate(zip(result.stdout,expected)) if a!=b)
                    raise AssertionError((args,'native projection mismatch',hex(at)))
                count+=1
    print(f'PASS: actual Car_Spawn body, {count} x64 scenarios, native Car/SceneNode pointers above 4 GB; complete heap and gravity match original, ASan/UBSan')


if __name__=='__main__':main()
