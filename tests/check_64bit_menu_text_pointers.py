#!/usr/bin/env python3
"""Actual runtime Menu/Texture/Graphics and both complete draws with native text."""
import os,re,struct,subprocess,tempfile
from pathlib import Path
from check_64bit_car_contacts import ROOT,body
from differential_menu_text_pointers import cases,oracle
PRELUDE=r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
using BYTE=unsigned char;using DWORD=uint32_t;using WORD=uint16_t;using USHORT=uint16_t;using BOOL=int;
struct IDirectDrawSurface7;struct IDirectDraw7;using LPDIRECTDRAW7=IDirectDraw7*;
'''
SUPPORT=r'''
Graphics *g_pGraphics;Texture *g_unk0x0052aa60,*g_unk0x0052aa68;
int g_unk0x00516074=(int)0xfffafafau,g_unk0x00516078=(int)0xffdbaca7u,g_unk0x0051608c=(int)0xbfae8072u;
char localized[32][32],literal[7][32];
void emit(uint32_t k,uint32_t a=0,uint32_t b=0,uint32_t c=0,uint32_t d=0,uint32_t e=0,uint32_t f=0,uint32_t g=0,uint32_t h=0) {
 uint32_t v[]={k,a,b,c,d,e,f,g,h};assert(fwrite(v,sizeof(v),1,stdout)==1);
}
unsigned canonical(char *p) {
 assert((uintptr_t)p>UINT32_MAX);
 for(unsigned i=0;i<32;i++)if(p==localized[i])return 0x10008000+i*32;
 for(unsigned i=0;i<7;i++)if(p==literal[i])return 0x10009000+i*32;
 assert(false);return 0;
}
struct CFrontend {static char *GetTextString(int i) {emit(4,i);return localized[(unsigned)i&31];}};
void Font_SetBlendMode(int i){emit(1,i);}
void InRaceMenu_DrawHeaderBar(){emit(3);}
void Sprite_FillRect(BYTE *ctx,short *r,BYTE *c,int l) {
 assert(ctx==&g_pGraphics->field309_0x150 && (uintptr_t)ctx>UINT32_MAX);
 assert(!memcmp(c,&g_unk0x0051608c,4));emit(2,r[0],r[1],r[2],r[3],l);
}
void Font_DrawText(int font,char *text,int x,int y,int *colour,int flags){emit(5,font,canonical(text),x,y,(uint32_t)*colour,flags);}
void Sprite_Queue(SpriteRect *s,SpriteRect *d,Texture *t,int l,short angle,int *centre,SpriteRect *uv,BYTE *c,int flags) {
 assert((uintptr_t)t>UINT32_MAX && !angle && !centre && !uv);
 assert(s==(SpriteRect*)&t->field_0x11c && (t==g_unk0x0052aa60 || t==g_unk0x0052aa68));
 uint32_t colour;memcpy(&colour,c,4);
 emit(6,(uint16_t)d->x|((uint32_t)(uint16_t)d->y<<16),(uint16_t)d->w|((uint32_t)(uint16_t)d->h<<16),
 (uint16_t)s->x|((uint32_t)(uint16_t)s->y<<16),(uint16_t)s->w|((uint32_t)(uint16_t)s->h<<16),colour,l,flags);
}
'''
MAIN=r'''
int main() {
 static_assert(sizeof(void*)==8 && sizeof(MenuItem)==32 && offsetof(Menu,items)==32,"native menu stride");
 static_assert(offsetof(Texture,width)!=0x120 && offsetof(Graphics,field309_0x150)!=0x150,"native record fields move after pointers");
 Graphics gfx;Texture a,b;Menu m;g_pGraphics=&gfx;g_unk0x0052aa60=&a;g_unk0x0052aa68=&b;
 assert((uintptr_t)&m>UINT32_MAX);int in[6];
 while(fread(in,sizeof(in),1,stdin)==1) {
  memset(&gfx,0xa5,sizeof(gfx));memset(&a,0xa5,sizeof(a));memset(&b,0xa5,sizeof(b));memset(&m,0xa5,sizeof(m));
  gfx.resX=in[1];gfx.resY=in[2];m.itemCount=in[3];m.cursor=in[4];
  a.field_0x11c=1;a.field_0x11e=2;a.width=211;a.height=27;
  b.field_0x11c=3;b.field_0x11e=4;b.width=109;b.height=19;
  for(int i=0;i<7;i++){m.items[i].id=i*3-1;m.items[i].text=(in[5]==1 || (in[5]==2 && i%2))?literal[i]:nullptr;}
  Menu before=m;Texture oldA=a,oldB=b;Graphics oldGfx=gfx;
  if(!in[0])InRaceMenu_DrawOptionList(&m);else InRaceMenu_DrawMenuRows(&m);
  assert(!memcmp(&m,&before,sizeof(m)) && !memcmp(&a,&oldA,sizeof(a)) && !memcmp(&b,&oldB,sizeof(b)) && !memcmp(&gfx,&oldGfx,sizeof(gfx)));
 }
}
'''
def encoded(trace):
 out=bytearray()
 for entry in trace:
  kind,*a=entry
  if kind=='blend':v=[1,*a]
  elif kind=='header':v=[3]
  elif kind=='text':v=[4,*a]
  elif kind=='fill':v=[2,*a[0],a[1]]
  elif kind=='font':v=[5,*a]
  elif kind=='sprite':
   dst,src,col,layer,flags=a;pack=lambda r:[(r[0]&65535)|((r[1]&65535)<<16),(r[2]&65535)|((r[3]&65535)<<16)]
   v=[6,*pack(dst),*pack(src),col,layer,flags]
  out+=struct.pack('<9I',*([x&0xffffffff for x in v]+[0]*(9-len(v))))
 return out

def main():
 src=ROOT/'CMR2Decomp'
 extract=lambda filename,name:re.search(r'struct '+name+r'\s*\{.*?\n\};',(src/filename).read_text(encoding='latin1'),re.S).group()+'\n'
 code=PRELUDE+extract('Graphics.h','Graphics')+extract('Texture.h','Texture')+(src/'Menu.h').read_text()+extract('Sprite.h','SpriteRect')+SUPPORT
 for n in ('InRaceMenu_DrawOptionList','InRaceMenu_DrawMenuRows'):code+=body((src/'GameInfo.cpp').read_text(encoding='latin1'),n)
 code+=MAIN;data=bytearray();expected=bytearray();n=0
 for kind,res,count,cursor,mode in cases():
  data+=struct.pack('<6i',kind=='rows',*res,count,cursor,mode);expected+=encoded(oracle(kind,res,count,cursor,mode));n+=1
 with tempfile.TemporaryDirectory(prefix='cmr2-native-menu-text-') as d:
  p=Path(d);(p/'menu.cpp').write_text(code)
  subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-fsanitize=address,undefined',str(p/'menu.cpp'),'-o',str(p/'menu')],check=True)
  r=subprocess.run([str(p/'menu')],input=data,capture_output=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
  assert r.returncode==0,r.stderr.decode();assert r.stdout==expected,'complete primitive drawing model and native text identity'
 print(f'PASS: {n} actual x64 menu draw scenarios; real Menu/Texture/Graphics headers, texts and records above 4 GB, changed field offsets/strides, complete primitive call models, whole input guards, ASan/UBSan; rendering/header/text providers controlled')
if __name__=='__main__':main()
