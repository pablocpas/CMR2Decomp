#!/usr/bin/env python3
"""Compare spawn tuning selection and complete car memory to the original.
Usage: differential_car_spawn.py entities.json [rebuilt.exe]
Scene/model/selection providers are controlled; the complete spawn routine and
fixed-point arithmetic execute independently in each image.
"""
import itertools,json,random,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_EIP,UC_X86_REG_ESP
from differential_menu_list import Drawing,ROOT,HEAP,STACK,STOP
class Spawn(Drawing):
 def __init__(self,path,entities=None):
  super().__init__(path,entities);self.callbacks={}
  signatures={0x405d80:0,0x405da0:0,0x405e00:0,0x406310:0,0x406320:0,0x4063f0:1,0x406890:0,0x4074f0:0,0x407630:1,0x407e50:0,0x407e60:0,0x407e70:0,0x407ea0:0,0x408500:1,0x4086b0:1,0x4086f0:1,0x40ee70:1,0x40ee80:1,0x40ee90:1,0x41b370:0,0x43dff0:1,0x43e160:1,0x43e190:1,0x43e1b0:1,0x43e1d0:1,0x43e1f0:1,0x43e530:1,0x43e5a0:2,0x43eef0:1,0x456ca0:0,0x4660f0:0,0x494a40:0,0x494a70:0}
  for a,n in signatures.items():self.callbacks[self.addr(a)]=(n*4,lambda a=a,n=n:self.provider(a,n))
 def provider(self,a,n):
  args=self.args(n);self.trace.append((a,args))
  value=self.settings.get(a,0)
  if a==0x4063f0:value=0
  if a==0x407630:value=HEAP+0x6000+(args[0]&255)*16
  if a==0x40ee90:value=args[0]&255
  self.u.reg_write(UC_X86_REG_EAX,value&0xffffffff)
 def run(self,mode,flag25,network,players,slot,special,car_type):
  self.u.mem_write(self.base,bytes(self.memory));self.u.mem_write(HEAP,bytes(0x10000));self.u.mem_write(STACK,bytes(0x10000))
  self.settings={0x405d80:mode,0x405da0:players,0x405e00:network,0x407e60:flag25,0x407ea0:special,0x406310:special,0x456ca0:2,0x4660f0:-1,0x408500:-1,0x4074f0:1,0x40ee80:1,0x406890:HEAP+0x7000,0x494a40:HEAP+0x7100,0x494a70:HEAP+0x7200}
  for i in range(4):
   self.put(HEAP+0x1000+0x738+i*4,'<I',HEAP+0x3000+i*0x200)
  self.put(HEAP+0x1000+0x720,'<I',HEAP+0x4000)
  self.put(HEAP+0x4000+0xc,'<I',HEAP+0x5000)
  # Distinct valid percentage tuning records make every setup branch observable.
  for j,base in enumerate([HEAP+0x6000,HEAP+0x6010,HEAP+0x7000,HEAP+0x7100,HEAP+0x7200]):
   self.u.mem_write(base,bytes([j%3,j%3,10+j*5,40+j,50-j,30+j,35+j]))
  self.put(HEAP+0x8000+0x98,'<12i',65536,0,0,0,0,65536,0,0,0,0,65536,0)
  self.put(HEAP+0x9000,'<3i',65536,131072,196608)
  sp=STACK+0xff00;self.put(sp,'<7I',STOP,HEAP+0x1000,HEAP+0x8000,car_type,HEAP+0x9000,slot,1)
  self.u.reg_write(UC_X86_REG_ESP,sp);self.trace=[]
  self.u.emu_start(self.addr(0x43c7f0),STOP,count=100000)
  assert self.u.reg_read(UC_X86_REG_EIP)==STOP
  return self.read(HEAP,0x10000),self.trace

def main():
 e=json.loads(Path(sys.argv[1]).read_text());a=Spawn(ROOT/'cmr2bin/CMR2.exe');b=Spawn(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e);cases=0
 for args in itertools.product([0,2,4],[0,1],[0,1],[0,1],[0,1],[0,1],[0,6,13]):
  aa,bb=a.run(*args),b.run(*args)
  if aa!=bb:
   print('FAIL spawn',args)
   for i,pair in enumerate(itertools.zip_longest(aa[1],bb[1])):
    if pair[0]!=pair[1]:print('provider',i,pair);break
   print('heap differences',[(hex(i),x,y) for i,(x,y) in enumerate(zip(aa[0],bb[0])) if x!=y][:10]);return 1
  cases+=1
 print(f'{cases} spawn cases: identical tuning providers, setup arguments and complete car state');return 0
if __name__=='__main__':sys.exit(main())
