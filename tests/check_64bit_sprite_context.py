#!/usr/bin/env python3
"""Execute the actual width wrapper with native context addresses.

Graphics comes from the current header. DirectDraw pointer types are opaque;
Sprite_FillRect and FixMul are controlled leaves. Clipping behavior is outside
this type conversion; the x86 harness executes the real fixed-point operations.
"""
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile

from check_64bit_car_contacts import ROOT, body
from differential_sprite_rectangle_context import cases
from differential_car_info_records import short
from differential_stage_lighting import signed,mul

PRELUDE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <type_traits>
typedef unsigned char BYTE;
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef int BOOL;
struct IDirectDrawSurface7;
struct IDirectDraw7;
typedef IDirectDraw7 *LPDIRECTDRAW7;
'''
SUPPORT = r'''
Graphics *g_pGraphics;
int FixMul(int a,int b) {return static_cast<int>((static_cast<int64_t>(a)*b)>>16);}
unsigned calls;BYTE *forwarded;short actualRect[4];BYTE actualColour[4];int actualLayer,result;
int Sprite_FillRect(BYTE *context,short *rect,BYTE *colour,int layer) {
    ++calls;forwarded=context;memcpy(actualRect,rect,8);memcpy(actualColour,colour,4);actualLayer=layer;
    return result;
}
'''
MAIN = r'''
int main() {
    static_assert(sizeof(void *)==8,"native pointers");
    static_assert(offsetof(Graphics,field309_0x150)!=0x150,"context follows native surface pointer");
    static_assert(std::is_same<decltype(&Sprite_FillRect),int (*)(BYTE *,short *,BYTE *,int)>::value,
                  "native rectangle API");
    static_assert(std::is_same<decltype(&StageObject_FillWidthScaledRectangle),
                  int (*)(int,BYTE *,short *,BYTE *,int)>::value,"native wrapper API");
    Graphics nativeGraphics;g_pGraphics=&nativeGraphics;
    assert(reinterpret_cast<uintptr_t>(g_pGraphics)>UINT32_MAX);
    BYTE input[20];unsigned index=0;
    while(fread(input,1,sizeof(input),stdin)==sizeof(input)) {
        memset(&nativeGraphics,0xa5,sizeof(nativeGraphics));Graphics expected=nativeGraphics;
        short rect[4];memcpy(rect,input,8);
        short previous[4];memcpy(previous,rect,8);
        int layer,scale;memcpy(&layer,input+8,4);memcpy(&scale,input+12,4);
        BYTE colour[4];memcpy(colour,input+16,4);
        calls=0;forwarded=nullptr;result=static_cast<int>(0x81000000u+index*97u);++index;
        BYTE *context=&g_pGraphics->field309_0x150;
        assert(reinterpret_cast<uintptr_t>(context)>UINT32_MAX);
        int returned=StageObject_FillWidthScaledRectangle(scale,context,rect,colour,layer);
        assert(returned==result && calls==1 && forwarded==context);
        assert(!memcmp(&nativeGraphics,&expected,sizeof(expected)) && !memcmp(rect,previous,8));
        assert(!memcmp(colour,input+16,4));
        assert(fwrite(actualRect,8,1,stdout)==1 && fwrite(actualColour,4,1,stdout)==1);
        assert(fwrite(&actualLayer,4,1,stdout)==1 && fwrite(&returned,4,1,stdout)==1);
    }
}
'''


def main():
    source=ROOT/'CMR2Decomp'
    graphics=(source/'Graphics.h').read_text(encoding='latin1')
    graphics=re.search(r'struct Graphics\s*\{.*?\n\};',graphics,re.S).group()
    sprite=body((source/'Sprite.cpp').read_text(encoding='latin1'),'Sprite_FillRect')
    signature=sprite[:sprite.index('{')].strip()+';\n'
    wrapper=body((source/'StageObjects.cpp').read_text(encoding='latin1'),'StageObject_FillWidthScaledRectangle')
    inputs,expected=bytearray(),bytearray()
    for rectangle,layer,scale,seed in cases():
        colour=bytes(((seed*17+3*i)&255 for i in range(4)))
        inputs+=struct.pack('<4h2I4s',*rectangle,layer,scale&0xffffffff,colour)
        x,y,w,height=rectangle
        width=short(mul(signed(w<<16),scale)>>16)
        expected+=struct.pack('<4h4s2I',x,y,width,height,colour,layer,(0x81000000+seed*97)&0xffffffff)
    with tempfile.TemporaryDirectory(prefix='cmr2-native-sprite-context-') as directory:
        tmp=Path(directory);fixture=tmp/'rectangles.cpp';executable=tmp/'rectangles'
        fixture.write_text(PRELUDE+graphics+signature+SUPPORT+wrapper+MAIN)
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fwrapv','-fno-strict-aliasing',
                        '-fsanitize=address,undefined',str(fixture),'-o',str(executable)],check=True)
        actual=subprocess.run([str(executable)],input=inputs,capture_output=True,
                              env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
        assert actual.returncode==0,actual.stderr.decode()
        assert actual.stdout==expected,'native width/context model and return forwarding'
    print('PASS: actual width wrapper, current Graphics and rectangle API on x64; 960 scenarios, '
          'context/forwarding above 4 GB, shifted member offset, model/input guards, ASan/UBSan; drawing leaf controlled')


if __name__=='__main__':main()
