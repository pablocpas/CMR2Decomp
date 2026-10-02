#!/usr/bin/env python3
"""Compare complete pre-stage menu records and reject overlap with globals.
Usage: differential_service_menus.py entities.json [rebuilt.exe]
Menu building helpers execute for real; only selection/preview providers are
controlled. Record pointers are translated to original addresses for comparison.
"""
import bisect,json,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_EIP,UC_X86_REG_ESP
from differential_menu_list import Drawing,ROOT,HEAP,STACK,STOP
class Service(Drawing):
 def __init__(self,path,entities=None):
  super().__init__(path,entities);self.callbacks={}
  for a in [0x501230,0x405d80,0x4ff4e0,0x50a080,0x50a3c0]:
   self.callbacks[self.addr(a)]=(0,lambda a=a:self.provider(a))
  if entities:
   self.rev=sorted((r[0],int(o,16)) for o,r in entities.items());self.keys=[r[0] for r in self.rev]
 def provider(self,a):self.u.reg_write(UC_X86_REG_EAX,self.mode if a==0x405d80 else (4 if a==0x501230 else 0))
 def canon(self,ptr):
  if not self.entities or not self.base<=ptr<self.base+self.size:return ptr
  i=bisect.bisect_right(self.keys,ptr)-1;r,o=self.rev[i];return o+ptr-r
 def run(self,root,base,mode):
  self.u.mem_write(self.base,bytes(self.memory));self.u.mem_write(STACK,bytes(0x10000));self.mode=mode
  addr=self.addr(base)
  if self.entities:
   # A complete Menu must own its items and callback footer; constructing it
   # must never overwrite an independently allocated global.
   overlaps=[(o,r[1]) for o,r in self.entities.items() if int(o,16)>=0x511000 and
             addr<r[0]<addr+0x1e0 and not base<int(o,16)<base+0x1e0]
   assert not overlaps, 'menu storage overlaps other globals: '+str(overlaps[:5])
  sp=STACK+0xff00;self.put(sp,'<I',STOP);self.u.reg_write(UC_X86_REG_ESP,sp);self.trace=[]
  self.u.emu_start(self.addr(root),STOP,count=100000);assert self.u.reg_read(UC_X86_REG_EIP)==STOP
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
 print(f'{cases} service menu cases: identical complete records and callbacks, no overlapping globals');return 0
if __name__=='__main__':sys.exit(main())
