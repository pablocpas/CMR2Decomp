#!/usr/bin/env python3
"""Guarded sound lookups and AI route publication against original x86.

Independent models cover range boundaries, integer interpolation/truncation,
the zero-step path, signed route point counts and coordinate wraparound.
"""
import itertools,json,random,struct,sys
from pathlib import Path
from unicorn import UC_HOOK_MEM_WRITE
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ESP,UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP
from differential_stage_lighting import Lighting,signed
from differential_menu_list import ROOT,HEAP,STACK

CURVES=[(20,2000,3000),(126,2200,8500),(193,0,160),(37,-20,-3),(1,-1,0),(7,-50,47)]
PRESERVED=(UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP)

class Lookup(Lighting):
 def __init__(self,path,entities=None):
  super().__init__(path,entities);self.callbacks={self.addr(0x421530):(8,self.origin),self.addr(0x421420):(0,self.points)};self.active=False
  self.u.hook_add(UC_HOOK_MEM_WRITE,self.write,begin=self.base,end=self.base+self.size-1)
 def origin(self):
  index,out=self.args(2);assert index==0;self.put(out,'<3i',*self.route_origin);self.trace.append(('origin',index))
 def points(self):
  self.u.reg_write(UC_X86_REG_EAX,self.point_count&0xffffffff);self.trace.append(('points',))
 def write(self,u,access,address,size,value,data):
  if self.active:
   table=self.addr(0x592744);assert size==4 and table+4<=address<table+36
   self.writes.append(((address-table)//4,value))
 def invoke_checked(self,target,args):
  for i,reg in enumerate(PRESERVED):self.u.reg_write(reg,0xa1b20000+i)
  self.invoke(target,[v&0xffffffff for v in args])
  assert self.u.reg_read(UC_X86_REG_ESP)==STACK+0xff00+4+4*len(args)
  assert all(self.u.reg_read(reg)==0xa1b20000+i for i,reg in enumerate(PRESERVED))
  return self.u.reg_read(UC_X86_REG_EAX)
 def curve(self,seed,config,value,target):
  self.reset();r=random.Random(seed);self.u.mem_write(HEAP,r.randbytes(0x10000))
  count,lo,hi=config;curve,bytes_at,words_at=HEAP+0x800,HEAP+0x1000,HEAP+0x2000
  self.put(curve,'<3i2I',count,lo,hi,words_at,bytes_at)
  self.initial=self.read(HEAP,0x10000);expected=self.initial
  if target==0x427ab0:model=int(lo<=value<=hi)
  else:
   span=hi-lo;i=value-lo;index=count*i//span;step=span//count
   samples=list(self.read(bytes_at,512)) if target==0x427ad0 else struct.unpack('<256H',self.read(words_at,512))
   frac=(i%step)*100//step if step else 0
   model=((100-frac)*samples[index])//100+(samples[index+1]*frac)//100 if step else samples[index]
  self.active=True;self.writes=[]
  result=self.invoke_checked(target,[value,curve]);self.active=False
  if target==0x427ab0:result&=255  # bool is returned in AL; upper EAX is unspecified.
  observed=(self.read(HEAP,0x10000),result)
  assert observed==(expected,model),(hex(target),config,value,result,model)
  assert not self.writes and not self.trace
  return observed
 def route(self,seed,count,points):
  self.reset();r=random.Random(seed);self.u.mem_write(HEAP,r.randbytes(0x10000));table=self.addr(0x592744)
  self.point_count=points;self.route_origin=(signed(0x7ffffff0+seed*37),-123,signed(0x80000010-seed*53))
  slots=[HEAP+0xf000+i*4 for i in range(9)];self.put(table,'<9I',*slots)
  self.initial=self.read(HEAP,0x10000);expected=bytearray(self.initial);self.writes=[];expected_writes=[]
  for entry in range(max(count,0)):
   slots[entry+1]=HEAP+0x1000+entry*0x1720;expected_writes.append((entry+1,slots[entry+1]))
   for i in range(max(signed((points&0x7fff)-(points&0x8000)),0)):
    for offset,origin in [(0xb90,self.route_origin[0]),(0x1158,self.route_origin[2])]:
     at=0x1000+entry*0x1720+offset+i*4;old=struct.unpack_from('<i',expected,at)[0];struct.pack_into('<i',expected,at,signed(old+origin))
  self.active=True;result=self.invoke_checked(0x498590,[HEAP+0x1000,0,count]);self.active=False
  observed=(self.read(HEAP,0x10000),result,self.read(table,36))
  assert observed==(bytes(expected),HEAP+0x1000+max(count,0)*0x1720,struct.pack('<9I',*slots))
  assert self.trace==[('origin',0),('points',)] and self.writes==expected_writes
  return observed

def cases():
 for seed,config in itertools.product(range(4),CURVES):
  _,lo,hi=config
  for value in sorted({lo,lo+1,(lo+hi)//2,hi-1,hi}):
   for target in (0x427ad0,0x427b70):yield ('curve',(seed,config,value,target))
  for value in sorted({lo-1,lo,hi,hi+1}):yield ('curve',(seed,config,value,0x427ab0))
 for values in itertools.product(range(4),[-1,0,1,2,8],[-1,0,1,12,370]):yield ('route',values)

def main():
 entities=json.loads(Path(sys.argv[1]).read_text());a=Lookup(ROOT/'cmr2bin/CMR2.exe');b=Lookup(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities);count=0
 for kind,args in cases():
  assert getattr(a,kind)(*args)==getattr(b,kind)(*args),(kind,args);count+=1
 print(f'PASS: {count} original/rebuilt lookup and route scenarios; independent models, complete guarded heap, global pointer slots/write widths, provider order and callee ABI')

if __name__=='__main__':main()
