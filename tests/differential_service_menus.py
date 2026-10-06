#!/usr/bin/env python3
"""Compare complete pre-stage menu records and reject overlap with globals.
Usage: differential_service_menus.py entities.json [rebuilt.exe]
Menu building helpers execute for real; only selection/preview providers are
controlled. Record pointers are translated to original addresses for comparison.
"""
import bisect,itertools,json,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_EIP,UC_X86_REG_ESP
from differential_menu_list import Drawing,ROOT,HEAP,STACK,STOP
class Service(Drawing):
 def __init__(self,path,entities=None):
  super().__init__(path,entities);self.callbacks={}
  for a in [0x501230,0x405d80,0x4ff4e0,0x50a080,0x50a3c0]:
   self.callbacks[self.addr(a)]=(0,lambda a=a:self.provider(a))
  for a in [0x407e50,0x407ea0,0x4174d0,0x411880]:
   self.callbacks[self.addr(a)]=(0,lambda a=a:self.selection(a))
  if entities:
   self.rev=sorted((r[0],int(o,16)) for o,r in entities.items());self.keys=[r[0] for r in self.rev]
 def provider(self,a):self.u.reg_write(UC_X86_REG_EAX,self.mode if a==0x405d80 else (4 if a==0x501230 else 0))
 def selection(self,a):
  self.trace.append(a);self.u.reg_write(UC_X86_REG_EAX,self.flags[a])
 def canon(self,ptr):
  if not self.entities or not self.base<=ptr<self.base+self.size:return ptr
  i=bisect.bisect_right(self.keys,ptr)-1;r,o=self.rev[i];return o+ptr-r
 def run(self,root,base,mode,flags=None,poison=None):
  self.u.mem_write(self.base,bytes(self.memory));self.u.mem_write(STACK,bytes(0x10000));self.mode=mode
  self.flags=flags or {}
  addr=self.addr(base)
  if poison is not None:self.u.mem_write(addr-16,bytes([poison])*0x200)
  if self.entities:
   # A complete Menu must own its items and callback footer; constructing it
   # must never overwrite an independently allocated global.
   overlaps=[(o,r[1]) for o,r in self.entities.items() if int(o,16)>=0x511000 and
             addr<r[0]<addr+0x1e0 and not base<int(o,16)<base+0x1e0]
   assert not overlaps, 'menu storage overlaps other globals: '+str(overlaps[:5])
  sp=STACK+0xff00;self.put(sp,'<I',STOP);self.u.reg_write(UC_X86_REG_ESP,sp);self.trace=[]
  self.u.emu_start(self.addr(root),STOP,count=100000);assert self.u.reg_read(UC_X86_REG_EIP)==STOP
  assert self.u.reg_read(UC_X86_REG_ESP)==sp+4, 'menu builder return ABI'
  if poison is not None:
   assert self.read(addr-16,16)==bytes([poison])*16 and self.read(addr+0x1e0,16)==bytes([poison])*16, 'menu buffer guards'
  data=bytearray(self.read(addr,0x1e0))
  for off in [8,0x1cc,0x1d0,0x1d4,0x1d8,0x1dc]+[0x14+i*20+j for i in range(22) for j in [0,12,16]]:
   v,=struct.unpack_from('<I',data,off);struct.pack_into('<I',data,off,self.canon(v))
  return bytes(data)
def main():
 e=json.loads(Path(sys.argv[1]).read_text());a=Service(ROOT/'cmr2bin/CMR2.exe');b=Service(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e);cases=0
 for root,base in [(0x502310,0x82b668),(0x5024a0,0x82b848)]:
  for mode in range(8):
   try:aa,bb=a.run(root,base,mode),b.run(root,base,mode)
   except Exception as err:print('FAIL service menu',hex(root),mode,str(err));return 1
   if aa!=bb:
    print('FAIL service menu',hex(root),mode,[(hex(i),x,y) for i,(x,y) in enumerate(zip(aa,bb)) if x!=y][:12]);return 1
   cases+=1
 for flag24,flag28,optional,head,poison in itertools.product((0,1,128),(0,1,128),(0,1),(0,1),(0,0xa5,0xff)):
  flags={0x407e50:0x12340000|flag24,0x407ea0:0x12340000|flag28,
         0x4174d0:optional,0x411880:head}
  expected_calls=[0x407e50]+([0x407ea0] if flag24==0 else [])+[0x4174d0]+([0x411880] if optional else [])
  try:
   aa,bb=a.run(0x404000,0x52aa70,0,flags,poison),b.run(0x404000,0x52aa70,0,flags,poison)
   assert aa==bb, 'original/rebuilt menu records differ'
   assert a.trace==b.trace==expected_calls, 'selection provider order'
   for data in (aa,bb):
    assert data[6]==6+optional, 'car setup item count'
    ids=[0x30 if flag24 or flag28 else 0x64,0x65,0x66,0x67]+([0x68] if optional else [])
    for index,id_ in enumerate(ids):
     offset=0x14+index*20
     assert struct.unpack_from('<h',data,offset+4)[0]==id_, 'car setup option id'
     assert struct.unpack_from('<h',data,offset+8)[0]==index, 'car setup option value'
     assert struct.unpack_from('<I',data,offset+16)[0]==0, 'car setup option param'
     assert data[offset+10]==(3 if id_==0x68 and not head else 2), 'car setup choice count'
  except Exception as err:
   print('FAIL car setup menu',flag24,flag28,optional,head,poison,str(err));return 1
  cases+=1
 print(f'{cases} service/car setup menu cases: identical complete records, option indices, callbacks, guards and selection calls');return 0
if __name__=='__main__':sys.exit(main())
