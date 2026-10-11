#!/usr/bin/env python3
"""Actual control indicator body with native Car fields and numeric DeviceInfo.

The DeviceInfo declaration is copied verbatim from production Input.h; platform
queries/camera leaves are controlled. Whole Car guards, device merging and trace
outputs are compared with original x86 plus its independent signed model.
"""
import itertools,os,re,shutil,struct,subprocess,tempfile
from pathlib import Path
from check_64bit_car_contacts import body
from differential_player_indicators import Indicators,CAR,DEVICE,SOURCE,ROOT,HEAP

PRELUDE=r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;typedef char CHAR;typedef long long __int64;
#define MAX_PATH 260
#include "Car.h"
#include "DeviceInfo.h"
#include "CallbackState.h"
Car car,*g_controlCar=&car;DeviceInfo device,source;CallbackStateMachine machine;CallbackStateRecord records[3];
BYTE g_carAxisInputEnabled[3];unsigned short g_carButtonMasks[9];
int player,state,multiplayer,camera,finished,inverted,cheat,axes[4];BYTE image[65536];std::vector<BYTE> trace;
void put(uint32_t v){auto *p=(BYTE *)&v;trace.insert(trace.end(),p,p+4);}
void event(uint32_t address,std::initializer_list<int> args={}){put(address);put(args.size());for(int a:args)put((uint32_t)a);}
struct CGameInfo{static int IsInRaceMenuOpen(){event(0x404f20);return 0;}static int GetGameModeOptionBit19(){event(0x405e00);return 0;}static int IsActiveCheatEnabled(int n){event(0x4063f0,{n});return cheat;}};
struct CInput{
 static DeviceInfo *GetAvailableDeviceRecord(int n){event(0x49ead0,{n});return n==3?&device:&source;}
 static unsigned int GetEnabledControllerAxisBinding(int n,int axis){event(0x40c210,{n,axis});return axes[axis];}
 static DWORD GetControllerField124(short n){event(0x40be60,{n});return inverted;}
 static DWORD GetControllerField120(int n){event(0x40be30,{n});return inverted;}
};
BYTE RallyDataState(){event(0x4074f0);return 4;}
void StageObject_ReadCarControllerMapping(int n){event(0x47bca0,{n});}
BYTE *StageUI_GetRaceResultTable(){event(0x41b390);return (BYTE *)&machine;}
int GameMenu_IsPauseHeaderActive(){event(0x44a130);return 1;}
int Race_IsMultiplayerRecordMode10(){event(0x41f3a0);return multiplayer;}
int View_GetActiveCameraMode(BYTE n){event(0x422f50,{n});return camera;}
int Race_GetPlayerRecordField4(BYTE n){event(0x41f3d0,{n});return finished;}
void View_UpdateDriverCameraCycle(int n){event(0x47b970,{n});}
int StageObject_CycleDriverCameraSelection(int buttons,int n){event(0x47bad0,{buttons,n});return 0;}
int word(int at){int n;memcpy(&n,image+at,4);return n;}
void read(void *p,size_t n){assert(fread(p,1,n,stdin)==n);}
'''
MAIN=r'''
int main(int argc,char **argv){
 assert(argc==14 && sizeof(void *)==8);player=atoi(argv[1]);state=atoi(argv[2]);multiplayer=atoi(argv[3]);camera=atoi(argv[4]);finished=atoi(argv[5]);
 int kind=atoi(argv[6]);axes[0]=atoi(argv[7]);axes[1]=-1;axes[2]=atoi(argv[8]);axes[3]=atoi(argv[9]);inverted=atoi(argv[10]);cheat=atoi(argv[11]);
 g_carAxisInputEnabled[0]=atoi(argv[12]);g_carAxisInputEnabled[1]=atoi(argv[13]);read(image,sizeof(image));
 static_assert(sizeof(DeviceInfo)==0x50c,"pointer-free device record");static_assert(offsetof(Car,steeringInputMode)!=0xb88,"native Car offsets");
 assert((uintptr_t)&car>UINT32_MAX && (uintptr_t)&device>UINT32_MAX && (uintptr_t)&machine>UINT32_MAX);
 memcpy(&car,image+0x1000,0x1e8);car.index=player;car.automaticGearbox=word(0x1000+0xb9c);
 car.steeringInputMode=word(0x1000+0xb88);car.analogThrottle=word(0x1000+0xb8c);car.analogBrake=word(0x1000+0xb90);
 memcpy(&device,image+0x3000,sizeof(device));memcpy(&source,image+0x4000,sizeof(source));assert((int)device.field_0x0==kind);
 machine.records=records;records[player].packedState=state;for(int i=0;i<8;i++)g_carButtonMasks[i]=1<<i;
 Car before=car;auto oldSource=source;StageObject_UpdatePlayerControlIndicators(player,3);
 // Permit exactly the fields written by the original control dispatch.
 memcpy(before.flag0x1d0,car.flag0x1d0,4);before.field_0x1d4[0]=car.field_0x1d4[0];before.handbrake=car.handbrake;
 before.steeringInputMode=car.steeringInputMode;before.analogThrottle=car.analogThrottle;before.analogBrake=car.analogBrake;
 assert(!memcmp(&car,&before,sizeof(car)) && !memcmp(&source,&oldSource,sizeof(source)));
 memcpy(image+0x1000+0x1d0,car.flag0x1d0,4);image[0x1000+0x1d4]=car.field_0x1d4[0];memcpy(image+0x1000+0x1d8,&car.handbrake,4);
 memcpy(image+0x1000+0xb88,&car.steeringInputMode,4);memcpy(image+0x1000+0xb8c,&car.analogThrottle,4);memcpy(image+0x1000+0xb90,&car.analogBrake,4);memcpy(image+0x3000,&device,sizeof(device));
 assert(fwrite(image,1,sizeof(image),stdout)==sizeof(image));assert(fwrite(trace.data(),1,trace.size(),stdout)==trace.size());
}
'''
def main():
 src=ROOT/'CMR2Decomp';fn=body((src/'StageObjects.cpp').read_text(),'StageObject_UpdatePlayerControlIndicators');input_header=(src/'Input.h').read_text();fragment=input_header[input_header.index('struct JoystickBinding'):input_header.index('struct Unk0x0059f8cc')]
 with tempfile.TemporaryDirectory(prefix='cmr2-native-controls-') as directory:
  tmp=Path(directory)
  for h in ('FloatToInt.h','Car.h','SceneNode.h','Sector.h','Mesh.h','CallbackState.h'):shutil.copyfile(src/h,tmp/h)
  shutil.copyfile(ROOT.parent/'OpenCMR2/game/FixedPoint.h',tmp/'FixedPoint.h');(tmp/'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;\n');(tmp/'Graphics.h').write_text('struct Graphics;class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0;};\n');(tmp/'DeviceInfo.h').write_text(fragment)
  cpp=tmp/'control.cpp';cpp.write_text(PRELUDE+fn+MAIN);exe=tmp/'control';env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
  subprocess.run(['clang++','-include',str(ROOT/'CMR2Decomp/FloatToInt.h'),'-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-I',str(tmp),'-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast','-fsanitize=address,undefined',str(cpp),'-o',str(exe)],check=True)
  oracle=Indicators(ROOT/'cmr2bin/CMR2.exe');cases=[]
  for seed,kind,axes,inverted,finished,enabled in itertools.product(range(3),(1,2,3),[(0,-1,1,2),(0,-1,1,1),(-1,-1,-1,-1)],range(2),range(2),[(1,1),(0,1),(1,0)]):
   raw=(-65536,32769,0x7fffffff,-0x80000000)[seed:]+(-65536,32769,0x7fffffff,-0x80000000)[:seed]
   cases.append((seed%3,seed%2*10,seed%2,(4,8,10)[seed],finished,kind,axes,raw,inverted,(0,127,128)[seed],seed%2,seed,enabled))
  for case in cases:
   # run() contains the independent signed model and full x86 guards. Capture
   # its actual pre-call fixture by wrapping the invocation for this one call.
   captured=[];invoke=oracle.checked_invoke
   def record(target,args):captured.append(oracle.read(HEAP,0x10000));return invoke(target,args)
   oracle.checked_invoke=record
   try:expected,trace=oracle.run(*case)
   finally:oracle.checked_invoke=invoke
   player,state,multi,camera,finished,kind,axes,raw,inverted,buttons,cheat,seed,enabled=case
   args=(player,state,multi,camera,finished,kind,axes[0],axes[2],axes[3],inverted,cheat,*enabled)
   encoded=bytearray(expected)
   for address,values in trace:encoded.extend(struct.pack('<2I',address,len(values)));encoded.extend(struct.pack('<'+'I'*len(values),*[v&0xffffffff for v in values]))
   p=subprocess.run([str(exe),*map(str,args)],input=captured[0],capture_output=True,env=env)
   assert not p.returncode,(case,p.stderr.decode());assert p.stdout==encoded,(case,[(hex(i),a,b) for i,(a,b) in enumerate(zip(p.stdout,encoded)) if a!=b][:12])
 print(f'PASS: {len(cases)} native player-indicator cases vs original x86 and independent signed model; actual body, production Car/DeviceInfo, full-car guards, complete heap/provider traces, pointers above 4GB, ASan/UBSan')
if __name__=='__main__':main()
