#!/usr/bin/env python3
"""Whole particle rendering/initialization/setter and race restore bodies.

Independent primitive models cover animation, visibility, matrix attachment,
callback pointer identities, initializer data and every untouched input byte.
Draw callbacks, billboard submission, random and route-reset leaves are controlled.
"""
import itertools,json,random,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX
from differential_car_info_records import Records,put
from differential_stage_lighting import signed,mul
from differential_menu_list import ROOT,HEAP
P,T,F,M,C,D=tuple(HEAP+n for n in (0x1000,0x2000,0x3000,0x4000,0x5000,0x6000))
CUSTOM=HEAP+0xf000

def draw_cases():return itertools.product(range(16),(-1,0,1,8),(0,1,3,7,8))
class Particles(Records):
 def provider(self,a,n):
  args=tuple(self.args(n));value=0
  if a==CUSTOM:self.trace.append(('callback',*args))
  elif a==0x4b11c0:
   definition,texture=args;raw=bytearray(self.read(definition,36));raw[35]&=3;self.trace.append(('billboard',bytes(raw),texture))
  elif a==0x4c6676:value=self.random_value;self.trace.append(('rand',value))
  elif a==0x420820:self.trace.append(('routes',))
  else:raise AssertionError(hex(a))
  self.u.reg_write(UC_X86_REG_EAX,value);self.u.reg_write(UC_X86_REG_ECX,0xa5a5a5a5);self.u.reg_write(UC_X86_REG_EDX,0x5a5a5a5a)
 def draw(self,seed,count,view):
  h=self.begin(seed);rng=random.Random(seed);want=[];self.intercept((0x4b11c0,2));self.callbacks[CUSTOM]=(12,lambda:self.provider(CUSTOM,3))
  self.put(self.addr(0x6a2cdc),'<I',P);self.put(self.addr(0x6a2cd4),'<i',count)
  for i in range(8):
   p,t,f,m=P+i*104,T+i*112,F+i*16,M+i*64;active=(seed+i)%3!=0;visible=(seed*17+i*31)&255;custom=(seed+i)%5==0;animated=(seed+i)%4!=0
   flags=0x20 if (seed+i)%2 else 0;direction=(seed+i)%16;life=0x50000;elapsed=(-65536,0,65536,131072,393216,1048576)[(seed+i)%6];delay=65536;step=(0,32768,65536)[(seed+i)%3]
   textures=[HEAP+0xd000+i*32,0,HEAP+0xe000+i*32];direct_texture=textures[(seed+i)%3]
   pos=tuple(rng.randrange(-0x300000,0x300000) for _ in range(3));offset=tuple(rng.randrange(-0x300000,0x300000) for _ in range(3));attachment=m if (seed+i)%2 else 0;size=(0,16384,65536,98304)[(seed+i)%4];rect=(-49152,-65536,81920,98304);angle=(seed*193+i*71)&4095;colour=bytes(rng.randrange(256) for _ in range(3));alpha=(seed*23+i*31)&255;shade=(seed*13+i*7)&255
   put(h,p,'<I',t);put(h,p+0x1c,'<3i',*pos);put(h,p+0x40,'<Ii',attachment,life-elapsed);put(h,p+0x4c,'<ih',size,angle);put(h,p+0x53,'<2B',alpha,shade);put(h,p+0x55,'<B',visible);put(h,p+0x57,'<B',active);h[p+0x59-HEAP:p+0x5c-HEAP]=colour;put(h,p+0x60,'<I',direct_texture)
   put(h,t,'<i',life);put(h,t+0x35,'<2B',flags,direction);put(h,t+0x3c,'<4i',*rect);put(h,t+0x4c,'<4I',f if animated else 0,3,delay,step);put(h,t+0x5c,'<I',CUSTOM if custom else 0);put(h,f,'<3I',*textures);put(h,m+0x30,'<3i',*offset)
   if i>=max(count,0) or not active or not visible&((1<<view)&255):continue
   if custom:want.append(('callback',p,t,C));continue
   texture=direct_texture
   if animated:
    frame=0
    if elapsed>delay and step>0:
     frame=int(((elapsed-delay)*65536)//step)>>16
     if direction&8:frame+=(p&65535)%3
     frame=2 if frame>=3 and not direction&4 else frame%3
    texture=textures[frame]
   if not texture:continue
   bounds=rect if size==65536 else tuple(mul(v,size) for v in rect);position=tuple(signed(a+b) for a,b in zip(pos,offset)) if attachment else pos
   definition=struct.pack('<3i4i3BBhBB',*position,*bounds,*colour,alpha,angle if flags&32 else 0,shade,direction&2)
   assert len(definition)==36;want.append(('billboard',definition,texture))
  assert self.invoke_guarded(0x4b0480,[C,view])==bytes(h),'full input heap stays unchanged'
  assert self.trace==want,(seed,count,view,self.trace,want)
  return want
 def init(self,kind,value,seed):
  h=self.begin(seed);after=bytearray(h);self.random_value=(seed*19937)&32767;self.intercept((0x4c6676,0));trace=[]
  if kind=='owner':put(h,D,'<i',value);put(after,D,'<i',value);put(after,P+0x64,'<H',value&65535);target=0x45de80
  elif kind=='glass':
   put(h,D+0xb1a,'<B',value&255);put(after,D+0xb1a,'<B',value&255);put(h,D+0xa70,'<i',value*257);put(after,D+0xa70,'<i',value*257)
   put(after,P+0x64,'<H',((value&255)<<8)+self.random_value%10);put(after,P+0x4c,'<i',value*257);target=0x499710;trace=[('rand',self.random_value)]
  else:
   put(h,D,'<2i',value*257,signed(value<<16));put(after,D,'<2i',value*257,signed(value<<16));put(after,P+0x64,'<H',(((self.random_value%30)<<8)+value)&65535);put(after,P+0x4c,'<i',value*257);target=0x499a80;trace=[('rand',self.random_value)]
  assert self.invoke_guarded(target,[P,T,D])==bytes(after);assert self.trace==trace;return bytes(after),trace
 def setter(self,selected,seed):
  h=self.begin(seed);after=bytearray(h);self.put(self.addr(0x6a2ce0),'<I',T if selected else 0)
  callbacks=(CUSTOM,CUSTOM+16,CUSTOM+32,CUSTOM+48);args=(*callbacks,seed*919)
  if selected:put(after,T+0x5c,'<4I',*callbacks);put(after,T+0x6c,'<i',args[-1])
  assert self.invoke_guarded(0x4aff10,args)==bytes(after);return bytes(after)
 def restore(self,seed):
  h=self.begin(seed);before=random.Random(seed).randbytes(192);expected=bytearray(before);self.u.mem_write(self.addr(0x538aa8),before)
  for i in range(8):
   base=i*24;expected[base:base+4]=before[base+4:base+8];expected[base+8:base+16]=bytes(8);expected[base+20:base+22]=bytes(2)
   self.allowed.update((self.addr(0x538aa8)+base+o,w) for o,w in ((0,4),(8,4),(12,4),(20,2)))
  self.intercept((0x420820,0));assert self.invoke_guarded(0x4207f0,[])==bytes(h);actual=self.read(self.addr(0x538aa8),192);assert actual==bytes(expected) and self.trace==[('routes',)];return actual

def main():
 e=json.loads(Path(sys.argv[1]).read_text());a=Particles(ROOT/'cmr2bin/CMR2.exe');b=Particles(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e);n=0
 for args in draw_cases():assert a.draw(*args)==b.draw(*args);n+=1
 for args in itertools.product(('owner','glass','debris'),(-32768,-1,0,1,127,255,32767),range(8)):assert a.init(*args)==b.init(*args);n+=1
 for args in itertools.product((0,1),range(32)):assert a.setter(*args)==b.setter(*args);n+=1
 for seed in range(64):assert a.restore(seed)==b.restore(seed);n+=1
 print(f'PASS: {n} whole draw/initializer/setter/route-restore scenarios, independent primitive models, animation/attachment/visibility/null/custom paths, complete heap/record guards, callbacks and stdcall/callee-saved ABI')
if __name__=='__main__':main()
