#!/usr/bin/env python3
"""Execute both actual queue bodies with real native records and >4 GB pointers."""
import os,struct,subprocess,tempfile
from pathlib import Path
from check_64bit_car_contacts import ROOT,body
from differential_quad_queue import cases,input_data,model,DEST,TEXTURE,CAPS

PRELUDE=r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <type_traits>
using BYTE=unsigned char;using DWORD=uint32_t;using UINT_PTR=uintptr_t;
struct D3DTLVERTEX {float sx,sy,sz,rhw;DWORD color,specular;float tu,tv;};
struct Texture {int guard;};
struct CGraphics {static const float m_oneOver65536;};
const float CGraphics::m_oneOver65536=1.f/65536.f;
#define RGBA_MAKE(r,g,b,a) (((DWORD)(a)<<24)|((DWORD)(r)<<16)|((DWORD)(g)<<8)|(b))
'''
GLOBALS=r'''
Quad2D g_quad2DLayerA[1024],g_quad2DLayerB[512],g_quad2DLayerC[2048],g_quad2DLayerD[1];
unsigned g_quad2DCountA,g_quad2DCountB,g_quad2DCountC,g_quad2DCountD;
BYTE g_quad2DOverflow,g_quad2DConvertOverflow;
alignas(128) Quad2D direct;
Quad2D expectedA[1024],expectedB[512],expectedC[2048],expectedD[1];
'''
MAIN=r'''
int main() {
 static_assert(sizeof(void*)==8 && sizeof(Quad2D)==160,"native render record");
 static_assert(offsetof(Quad2D,pTexture)==144 && offsetof(Quad2D,flags)==152,"native pointer followed by flags");
 static_assert(std::is_same<decltype(&Quad2D_Queue),void(*)(Quad2DVertices*,Texture*,UINT_PTR)>::value,"mixed native carrier");
 Quad2D *layers[]={g_quad2DLayerA,g_quad2DLayerB,g_quad2DLayerC,g_quad2DLayerD};
 Quad2D *expected[]={expectedA,expectedB,expectedC,expectedD};
 unsigned *counts[]={&g_quad2DCountA,&g_quad2DCountB,&g_quad2DCountC,&g_quad2DCountD};
 unsigned caps[]={1024,512,2048,1};
 Texture texture{0x12345678};assert((uintptr_t)&texture>UINT32_MAX && (uintptr_t)&direct>UINT32_MAX);
 unsigned input[5];unsigned scenarios=0;
 while(fread(input,sizeof(input),1,stdin)==1) {
  unsigned kind=input[0],flags=input[1],count=input[2],overflow=input[3],k=input[4];
  alignas(16) BYTE source[144];BYTE oracle[152];
  assert(fread(source,144,1,stdin)==1 && fread(oracle,152,1,stdin)==1);
  for(unsigned i=0;i<4;i++) {
   memset(layers[i],0xd3,caps[i]*sizeof(Quad2D));memcpy(expected[i],layers[i],caps[i]*sizeof(Quad2D));
   assert((uintptr_t)layers[i]>UINT32_MAX);
   *counts[i]=i==0?13:i==1?11:i==2?7:0;
  }
  *counts[k==4?0:k]=count;g_quad2DOverflow=g_quad2DConvertOverflow=overflow;
  memset(&direct,0xd3,sizeof(direct));Quad2D expectedDirect=direct;
  bool full=k<4 && count>=caps[k];unsigned slot=k==4?0:(count<caps[k]?count:caps[k]-1);
  Quad2D *target=k==4?&direct:&layers[k][slot];Quad2D *expectedTarget=k==4?&expectedDirect:&expected[k][slot];
  UINT_PTR destination=k==4?(UINT_PTR)&direct:flags;
  if(!full) {
   memcpy(&expectedTarget->verts,oracle,144);expectedTarget->pTexture=&texture;
   expectedTarget->flags=k==4?(unsigned)destination:flags;
  }
  if(kind==0)Quad2D_Queue((Quad2DVertices*)source,&texture,destination);
  else Quad2D_QueueFixedTriangle(0x12345678,(Quad2DInputVertex*)source,(Quad2DInputVertex*)(source+24),(Quad2DInputVertex*)(source+48),&texture,destination);
  for(unsigned i=0;i<4;i++) {
   assert(!memcmp(layers[i],expected[i],caps[i]*sizeof(Quad2D)));
   unsigned initial=i==0?13:i==1?11:i==2?7:0;
   if(i==(k==4?0:k))initial=count;
   if(i==k && !full)++initial;
   assert(*counts[i]==initial);
  }
  assert(!memcmp(&direct,&expectedDirect,sizeof(direct)));
  assert(g_quad2DOverflow==(kind==0 && full?1:overflow));
  assert(g_quad2DConvertOverflow==(kind==1 && full?1:overflow));
  if(!full)assert(target->pTexture==&texture && texture.guard==0x12345678);
  ++scenarios;
 }
 printf("PASS: %u actual x64 queue scenarios; current native records, guarded full layers, 8-byte texture/address carriers above 4 GB, capacity/priority/overflow and primitive models, ASan/UBSan\n",scenarios);
}
'''

def main():
 src=ROOT/'CMR2Decomp';header=(src/'Sprite.h').read_text(encoding='latin1')
 header='\n'.join(line for line in header.splitlines() if not line.startswith('#include'))
 code=PRELUDE+header+GLOBALS+body((src/'Sprite.cpp').read_text(encoding='latin1'),'Quad2D_Queue')+body((src/'Sprite.cpp').read_text(encoding='latin1'),'Quad2D_QueueFixedTriangle')+MAIN
 data=bytearray()
 for kind,flags,count,over,seed in cases():
  # Native typed destinations require their natural alignment. x86 also tests
  # the legacy unaligned form separately; it is not advertised as native safe.
  if flags>=DEST and flags!=DEST:continue
  k=next((i for i,m in enumerate((8,16,32,64)) if flags&m),4)
  source=input_data(kind,seed);data+=struct.pack('<5I',kind=='fixed',flags,count,over,k)+source.ljust(144,b'\0')+model(kind,bytes([0xd3])*152,source,TEXTURE,flags)
 with tempfile.TemporaryDirectory(prefix='cmr2-native-quad-') as d:
  p=Path(d);(p/'queue.cpp').write_text(code)
  subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-fsanitize=address,undefined',str(p/'queue.cpp'),'-o',str(p/'queue')],check=True)
  result=subprocess.run([str(p/'queue')],input=data,capture_output=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
  assert result.returncode==0,result.stderr.decode();print(result.stdout.decode(),end='')
if __name__=='__main__':main()
