#!/usr/bin/env python3
"""Complete in-race text drawing, literal/localized models and guarded records."""
import itertools,json,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX
from differential_car_info_records import Records,put,short,trunc
from differential_menu_list import ROOT,HEAP
MENU,GFX,TEX1,TEX2=HEAP+0x1000,HEAP+0x3000,HEAP+0x4000,HEAP+0x5000
LEAVES=((0x40bab0,1),(0x4a5e40,4),(0x401b60,0),(0x4a3c60,1),(0x40b880,6),(0x4a3290,9))
RESOLUTIONS=((640,480),(853,479),(1920,1080))

def cases():return itertools.product(('option','rows'),RESOLUTIONS,(-1,0,1,3,7),(-1,0,2,6),(0,1,2))
def text_address(index):return HEAP+0x8000+(index&31)*32

def oracle(kind,res,count,cursor,mode):
 x,yres=res;y=trunc(yres*0xaa,480);rect=[short(trunc(x*0x70,640)),0,211,27]
 trace=[('blend',2),('fill',(0,0,short(x),short(yres)),2),('header',)]
 for i in range(max(count,0)):
  rect[1]=short(y-trunc(yres*13,480))
  if kind=='option' and i==2:continue
  literal=HEAP+0x9000+i*32 if (mode==1 or mode==2 and i%2) else 0
  first=second=literal
  if not literal:
   first=text_address(i*3-1);second=text_address(i*3);trace += [('text',i*3-1),('text',i*3)]
  chosen=i==cursor;colour=0xfffafafa if chosen else 0xffdbaca7
  trace += [('font',1,first,trunc(x*0x86,640),y,colour,17),('font',0,second,trunc(x*0x86,640),trunc(yres*15,480)+y,colour,17),('sprite',tuple(rect),(1,2,211,27) if chosen else (3,4,109,19),colour,2,8)]
  y+=trunc(yres*0x36,480)
 trace.append(('blend',2));return trace

class TextMenus(Records):
 def provider(self,address,n):
  a=tuple(self.args(n));result=0
  if address==0x40bab0:self.trace.append(('blend',a[0]))
  elif address==0x401b60:self.trace.append(('header',))
  elif address==0x4a3c60:
   index=(a[0]+2147483648)%4294967296-2147483648;self.trace.append(('text',index));result=text_address(index)
  elif address==0x4a5e40:
   assert a[0]==GFX+0x150 and a[3]==2
   assert self.read(a[2],4)==struct.pack('<I',0xbfae8072)
   self.trace.append(('fill',self.rect(a[1]),a[3]))
  elif address==0x40b880:
   font,text,x,y,col,flags=a
   self.trace.append(('font',font,text,(x+2147483648)%4294967296-2147483648,(y+2147483648)%4294967296-2147483648,struct.unpack('<I',self.read(col,4))[0],flags))
  elif address==0x4a3290:
   assert a[2] in (TEX1,TEX2) and a[4:7]==(0,0,0)
   self.trace.append(('sprite',self.rect(a[1]),self.rect(a[0]),struct.unpack('<I',self.read(a[7],4))[0],a[3],a[8]))
  else:raise AssertionError(hex(address))
  self.u.reg_write(UC_X86_REG_EAX,result);self.u.reg_write(UC_X86_REG_ECX,0xa5a5a5a5);self.u.reg_write(UC_X86_REG_EDX,0x5a5a5a5a)
 def run(self,kind,res,count,cursor,mode):
  h=self.begin(17);self.intercept(*LEAVES)
  put(h,GFX,'<2i',*res);put(h,MENU+6,'<2b',count,cursor)
  for i in range(7):put(h,MENU+20+i*20,'<Ih',HEAP+0x9000+i*32 if (mode==1 or mode==2 and i%2) else 0,i*3-1)
  put(h,TEX1+0x11c,'<4h',1,2,211,27);put(h,TEX2+0x11c,'<4h',3,4,109,19)
  for address,value in ((0x520b74,GFX),(0x52aa60,TEX1),(0x52aa68,TEX2),(0x516074,0xfffafafa),(0x516078,0xffdbaca7),(0x51608c,0xbfae8072)):self.put(self.addr(address),'<I',value)
  assert self.invoke_guarded(0x401870 if kind=='option' else 0x401d20,[MENU])==bytes(h),'complete unchanged menu/texture heap'
  assert self.trace==oracle(kind,res,count,cursor,mode),('literal/localized drawing model',kind,res,count,cursor,mode,self.trace)
  return self.trace

def main():
 e=json.loads(Path(sys.argv[1]).read_text());a=TextMenus(ROOT/'cmr2bin/CMR2.exe');b=TextMenus(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e);n=0
 for args in cases():assert a.run(*args)==b.run(*args);n+=1
 print(f'PASS: {n} complete menu draw scenarios; independent literal/localized text, skipped row, dimensions/colours/provider order, full heap guards, ABI')
if __name__=='__main__':main()
