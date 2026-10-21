#!/usr/bin/env python3
"""Actual controller lookup/predicate bodies and record declarations on x64.

The lookup's binding query is a controlled leaf; the production predicate is
also tested directly for the nine declared keyboard entries. No native DirectInput.
"""
import os
from pathlib import Path
import subprocess
import tempfile

from check_64bit_address_loops import record
from check_64bit_car_part_tables import body
from differential_controller_mapping import Mappings, cases, ROOT

PRELUDE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using BYTE=unsigned char;using DWORD=uint32_t;using BOOL=int;
#define MAX_PATH 260
#define TRUE 1
#define FALSE 0
'''
API = r'''
int predicate,calls,calledAction=-1,calledProfile=-1;
class CInput {public:
 static ControllerData m_controllerInfo[6];static unsigned short m_unk0x005168f4[8];
 static short GetButtonMapping(unsigned short controllerSlot,int action);
 static BOOL RealIsControllerActionBound(int action,ControllerData *pController);
 static BOOL IsControllerActionBound(int action,ControllerData *pController) {
  assert((uintptr_t)pController>UINT32_MAX);++calls;calledAction=action;calledProfile=int(pController-m_controllerInfo);
  assert(calledProfile>=0 && calledProfile<6);return predicate;
 }
};
ControllerData CInput::m_controllerInfo[6];unsigned short CInput::m_unk0x005168f4[8];
'''
MAIN = r'''
int main(int argc,char **argv) {
 assert(argc==6 && sizeof(void *)==8);
 static_assert(sizeof(ControllerData)==0x2f0,"pointer-free controller record");
 static_assert(offsetof(ControllerData,actionMasks)==0x128,"mask offset");
 static_assert(sizeof(((ControllerData *)0)->actionMasks)==20,"ten masks");
 int slot=atoi(argv[2]),profile=atoi(argv[3]),action=atoi(argv[4]);predicate=atoi(argv[5]);
 assert(fread(CInput::m_controllerInfo,1,sizeof(CInput::m_controllerInfo),stdin)==sizeof(CInput::m_controllerInfo));
 assert(fread(CInput::m_unk0x005168f4,1,sizeof(CInput::m_unk0x005168f4),stdin)==sizeof(CInput::m_unk0x005168f4));
 BYTE before[sizeof(CInput::m_controllerInfo)];memcpy(before,CInput::m_controllerInfo,sizeof(before));
 unsigned short beforeSlots[8];memcpy(beforeSlots,CInput::m_unk0x005168f4,sizeof(beforeSlots));
 ControllerData &row=CInput::m_controllerInfo[profile];
 unsigned short *named[]={&row.steerLeftMask,&row.steerRightMask,&row.throttleMask,&row.brakeMask,&row.handbrakeMask,
                         &row.shiftUpMask,&row.shiftDownMask,&row.cameraCycleMask,&row.driverCycleMask,&row.backMask};
 for(int i=0;i<10;i++)assert(named[i]==row.actionMasks+i && (uintptr_t)named[i]>UINT32_MAX);
 int result;
 if(!strcmp(argv[1],"lookup"))result=CInput::GetButtonMapping((unsigned short)slot,action);
 else {assert(!strcmp(argv[1],"bound") && action>=0 && action<9);result=CInput::RealIsControllerActionBound(action,&row);}
 assert(!memcmp(before,CInput::m_controllerInfo,sizeof(before)) && !memcmp(beforeSlots,CInput::m_unk0x005168f4,sizeof(beforeSlots)));
 int output[]={result,calls,calledAction,calledProfile};assert(fwrite(output,sizeof(output),1,stdout)==1);
}
'''


def main():
    src = ROOT/'CMR2Decomp'
    header = (src/'Input.h').read_text()
    source = (src/'Input.cpp').read_text()
    oracle = Mappings(ROOT/'cmr2bin/CMR2.exe')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-controller-') as directory:
        tmp=Path(directory)
        cpp=tmp/'mapping.cpp';exe=tmp/'mapping'
        cpp.write_text(PRELUDE+record(header,'ControllerDataUnk0x210')+record(header,'ControllerData')+API+
                       body(source,'CInput::GetButtonMapping')+'\n'+
                       body(source,'CInput::IsControllerActionBound').replace('CInput::IsControllerActionBound','CInput::RealIsControllerActionBound')+MAIN)
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast','-fsanitize=address,undefined',str(cpp),'-o',str(exe)],check=True)
        import struct
        for i,(method,args) in enumerate(cases()):
            result,trace = getattr(oracle,method)(*args,11003+i)
            slot,profile,action,predicate = (args[0],args[1],args[2],args[4]) if method=='lookup' else (0,args[0],args[1],0)
            process=subprocess.run([str(exe),method,str(slot),str(profile),str(action),str(predicate)],input=oracle.native_input,
                capture_output=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
            assert process.returncode==0,(method,args,process.stderr.decode())
            expected=(result,len(trace),trace[0][0] if trace else -1,trace[0][1] if trace else -1)
            assert process.stdout==struct.pack('<4i',*expected),(method,args,'original / native result and dispatch')
    print(f'PASS: {len(cases())} native mapping cases; two production bodies, real ControllerData, indexed/named masks '
          'at identical offsets, pointers >4 GB, read-only guards, signed results and dispatch; ASan/UBSan')


if __name__=='__main__':
    main()
