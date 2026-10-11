#!/usr/bin/env python3
"""Complete .cat loader, reset, call-node traversal and checkpoint bodies.

Only resource/scene/audio leaves are controlled. Checks immutable guarded route
records, full car/heap state, globals, argument traces and the x86 calling ABI.
Actual direction/ground-position helpers execute through the original code.
"""
import itertools,json,math,random,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ESP,UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT,HEAP,STACK

ROOTS=tuple(int(math.sqrt(8+16*i)*256)&65535 for i in range(4096))
REGS=(UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP)
class Nodes(Lighting):
 def __init__(self,path,entities=None):
  super().__init__(path,entities);self.callbacks={}
  self.leaf(0x41f900,0,lambda:self.emit('name',result=HEAP+0x7000))
  self.leaf(0x4552c0,0,lambda:self.emit('archive',result=HEAP+0x7100))
  self.callbacks[self.addr(0x405620)]=(0,self.format)
  self.leaf(0x4a9df0,5,self.file)
  self.leaf(0x420820,0,lambda:self.emit('reset-probes'))
  self.leaf(0x49c0a0,2,self.register)
  self.leaf(0x42b5f0,1,self.car)
  self.leaf(0x421370,1,lambda:self.emit('best',result=self.best))
  self.leaf(0x431d80,1,self.overlap)
  self.leaf(0x431ff0,2,self.place)
  for a,n,name in [(0x417780,1,'slots'),(0x421d80,1,'cameras'),(0x4aff80,0,'particles'),(0x463ce0,1,'weather'),(0x417660,1,'unused'),(0x4176b0,0,'playback')]:
   self.leaf(a,n,lambda n=n,name=name:self.emit(name,*self.args(n)))
  self.leaf(0x4074f0,0,lambda:self.emit('mode',result=2))
  self.leaf(0x458270,1,lambda:self.emit('checkpoint',*self.args(1),result=self.case%2))
  self.leaf(0x448110,0,lambda:self.emit('clock',result=80+self.case%20))
  self.leaf(0x41b390,0,lambda:self.emit('result',result=HEAP+0x7200))
  self.leaf(0x417760,1,lambda:self.emit('state-c10',*self.args(1),result=(self.case//3)%2))
  self.leaf(0x4174e0,4,lambda:self.emit('call',*self.args(4)))
  self.leaf(0x421470,1,lambda:self.emit('ratio',result=0x8000))
 def leaf(self,a,n,fn):self.callbacks[self.addr(a)]=(n*4,fn)
 def emit(self,name,*args,result=0):self.trace.append((name,*args));self.u.reg_write(UC_X86_REG_EAX,result&0xffffffff)
 def format(self):
  dest,fmt,name=self.args(3);assert self.read(fmt,7)==b'%s.cat\0' and name==HEAP+0x7000
  self.u.mem_write(dest,b'route.cat\0');self.emit('format')
 def file(self):
  archive,name,flags,size,unknown=self.args(5);assert archive==HEAP+0x7100 and not flags and not unknown
  assert self.read(name,10)==b'route.cat\0';self.put(size,'<I',self.count*44+7 if not self.missing else 0)
  self.emit('file',result=0 if self.missing else HEAP+0x3000)
 def register(self):
  callback,arg=self.args(2);assert callback==self.addr(0x458040) and arg==0;self.emit('register')
 def car(self):i,=self.args(1);assert i in (0,1);self.emit('car',i,result=HEAP+0x1000)
 def overlap(self):
  node,=self.args(1);result=int(self.overlaps>0);self.overlaps-=result;self.emit('overlap',node,result=result)
 def place(self):a,b=self.args(2);self.emit('place',self.read(a,12),self.read(b,12))
 def initialize(self,case,count=12,missing=False):
  self.reset();self.put(self.addr(0x6e0ef4),'<4096H',*ROOTS);self.u.mem_write(HEAP,random.Random(case).randbytes(0x10000));self.case=case;self.count=count;self.missing=missing;self.best=case%8;self.overlaps=case%3
  self.put(self.addr(0x52f2ac),'<I',(2<<14)|(case%2<<24)|((case//2)%2<<25)|((case//4)%2<<26)|((case//8)%2<<28)|(3<<16))
  self.put(self.addr(0x538a84),'<i',0 if missing else count);self.put(self.addr(0x538a88),'<i',count);self.put(self.addr(0x538a94),'<i',case%2)
  self.put(self.addr(0x538a9c),'<I',HEAP+0x3000);self.put(self.addr(0x538a8c),'<3h',-1,-1,-1);self.put(self.addr(0x538aa0),'<h',0)
  self.u.mem_write(self.addr(0x538b68),bytes(36));self.u.mem_write(self.addr(0x538aa8),bytes([0xa5])*192)
  for i in range(count):
   x=i*80*65536 if case%3 else (i%4)*5*65536;z=(i%3)*7*65536
   self.put(HEAP+0x3000+i*44,'<3i',x,12345,z)
   self.put(HEAP+0x3000+i*44+24,'<Bb',int(i%3==1)|int(i%4==2)*2,-1 if i%3!=1 else (i*31+case)%255-128)
  self.u.mem_write(HEAP+0x7000,b'route\0');self.put(HEAP+0x7204,'<I',HEAP+0x7300)
  self.put(HEAP+0x7300,'<4i',8 if case%4 else 9,0,8,0)
  self.put(HEAP+0x1000+0xb1a,'<b',case%2);self.put(HEAP+0x1000+0x3f0,'<3i',(-1 if case%2 else 1)*65536,0,0)
  self.put(HEAP+0x1000+0x2d0,'<3i',123,456,789);self.put(HEAP+0x1000+0x360,'<3i',65536,0,0)
  for a in (0x53708c,0x537198,0x537248):self.put(self.addr(a),'<2i',self.best+(case%5)-2,self.best+2)
  self.put(self.addr(0x5371a0),'<i',1);self.u.mem_write(self.addr(0x5371d0),bytes(120))
  for i in range(10):self.put(self.addr(0x5371d0)+i*12,'<3I',0x1234+i,0x5678+i,0x100|(case%30+1))
  self.trace=[];return self.read(HEAP,0x10000)
 def checked(self,a,args):
  for i,r in enumerate(REGS):self.u.reg_write(r,0x1212a000+i)
  self.invoke(a,args);assert self.u.reg_read(UC_X86_REG_ESP)==STACK+0xff00+4+4*len(args)
  assert all(self.u.reg_read(r)==0x1212a000+i for i,r in enumerate(REGS))
 def run(self,kind,case,count,missing):
  before=self.initialize(case,count,missing)
  if kind=='load':self.checked(0x420630,[])
  elif kind=='reset':self.checked(0x431c50,[case%2])
  elif kind=='calls':self.checked(0x417090,[case%2])
  elif kind=='splits':self.checked(0x458a00,[])
  elif kind=='get':
   for i in range(count):self.checked(0x421440,[i]);self.checked(0x421530,[i,HEAP+0x8000]);self.checked(0x4201d0,[HEAP+0x8100,i])
  after=self.read(HEAP,0x10000);assert after[0x3000:0x3000+count*44]==before[0x3000:0x3000+count*44],'immutable .cat records'
  permitted=[(0x1000,0x1c24)] if kind=='reset' else [(0x8000,0x8200)] if kind=='get' else []
  expected=bytearray(before)
  for a,b in permitted:expected[a:b]=after[a:b]
  assert after==expected,'guarded heap'
  globals=[self.read(self.addr(a),n) for a,n in [(0x538a84,4),(0x538a88,4),(0x538a94,4),(0x538a8c,6),(0x538aa0,2),(0x538b68,36),(0x538aa8,192),(0x538c94,4),(0x53708c,8),(0x537198,8),(0x537248,8),(0x5371d0,120),(0x537354,4),(0x537350,4),(0x542c70,4),(0x542cac,1),(0x542cae,1),(0x538a9c,4),(0x542c74,4),(0x542c78,4),(0x542c7c,48)]]
  return after,globals,self.trace

def mutation_probe():
 original=Nodes(ROOT/'cmr2bin/CMR2.exe');mutant=Nodes(ROOT/'cmr2bin/CMR2.exe')
 # Original ground-position load at 0x421559 reads node +8 (z).
 # Substituting +4 must fail the guarded full-output comparison.
 memory=bytearray(mutant.memory);at=0x421559-mutant.base
 assert memory[at:at+4]==bytes.fromhex('8b441008')
 memory[at+3]=4;mutant.memory=bytes(memory)
 assert original.run('get',1,12,False)!=mutant.run('get',1,12,False),'wrong route field mutation survived'

def main():
 entities=json.loads(Path(sys.argv[1]).read_text());a=Nodes(ROOT/'cmr2bin/CMR2.exe');b=Nodes(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities);number=0
 for kind,case,count,missing in itertools.product(['load','reset','calls','splits','get'],range(32),[12,16],[False,True]):
  if kind=='get' and missing:continue
  args=kind,case,count,missing;left=a.run(*args);right=b.run(*args)
  assert left==right,('route-node differential',args,[(i,x,y) for i,(x,y) in enumerate(zip(left[1],right[1])) if x!=y],left[2],right[2]);number+=1
 mutation_probe()
 print(f'PASS: {number} complete route-node scenarios; original/rebuilt memory, immutable disk records, cache, reset/call/checkpoint traces and ABI; wrong position field mutation rejected')
if __name__=='__main__':main()
