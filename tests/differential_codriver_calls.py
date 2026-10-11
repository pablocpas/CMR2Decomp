#!/usr/bin/env python3
"""Original/rebuilt co-driver loading, numeric call queue and sprite outputs.

Actual producer, decoder, queue, release and drawing bodies execute. Resource,
menu queries, final audio dispatch and rendering are controlled leaves. Checks
complete guarded heap/records, pointer output, provider arguments and x86 ABI;
the decoder must also agree with an independent numeric model.
"""
import itertools,json,math,random,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ESP,UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT,HEAP,STACK

RECTS=[(i*13,i*7,32+i*2,24+i) for i in range(8)]
SINES=tuple(int(math.sin(i*math.pi/2048)*65536) for i in range(4096))
REGS=(UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP)
def icon_model(id,cheat):
 t=id&15;b1=(id>>4)&3;b2=(id>>17)&15;b3=(id>>21)&3
 result=int(b1!=0 or t not in (0,9) or b2 in (14,2,1) or b3!=0)
 flag=int(b1==1)^cheat;colour=bytes([255]*4);rect=RECTS[{2:5,3:4,4:3,5:2,6:2,7:1,8:0}.get(t,6)]
 if b2 and not b1:
  flag=0;rect=RECTS[7]
  if b2 in (2,14):colour=bytes([255,255,13,255])
  if b2==1:colour=bytes([255,0,0,255])
 if b3:
  flag=0;rect=RECTS[7];colour=bytes([255,255 if b3==1 else 0,13 if b3==1 else 0,255])
 return result,struct.pack('<4h',*rect),flag,colour

class Calls(Lighting):
 def __init__(self,path,entities=None):
  super().__init__(path,entities);self.callbacks={}
  for a,n,name in [(0x4063f0,1,'cheat'),(0x417760,1,'car-blocked'),(0x411880,0,'head-to-head'),(0x405dc0,0,'split'),(0x404f20,0,'menu'),(0x4054b0,1,'editing'),(0x41b370,0,'event-count'),(0x408a00,1,'status'),(0x477f70,2,'grid'),(0x418000,1,'assign'),(0x417e70,6,'message'),(0x41f900,0,'name'),(0x4552c0,0,'archive'),(0x49c0a0,2,'register'),(0x416670,0,'reset')]:
   self.callbacks[self.addr(a)]=(4*n,lambda a=a,n=n,name=name:self.provider(a,n,name))
  self.callbacks[self.addr(0x4a3290)]=(36,self.sprite)
  self.callbacks[self.addr(0x4a3c60)]=(4,lambda:self.provider(0x4a3c60,1,'text'))
  self.callbacks[self.addr(0x405620)]=(0,self.format)
  self.callbacks[self.addr(0x4a9df0)]=(20,self.file)
 def provider(self,a,n,name):
  args=self.args(n);value=0
  if name=='cheat':assert args==(2,);value=self.cheat
  elif name=='car-blocked':value=self.case%2
  elif name=='head-to-head':value=(self.case//2)%2
  elif name=='split':value=(self.case//4)%2
  elif name=='menu':value=(self.case//8)%2
  elif name=='editing':value=(self.case//16)%2
  elif name=='status':value=HEAP+0x2000
  elif name=='event-count':value=1
  elif name=='name':value=HEAP+0x7000
  elif name=='archive':value=HEAP+0x7100
  elif name=='text':assert args==(0x57,);value=HEAP+0x7200
  elif name=='register':assert args==(self.addr(0x418550),0);args=()
  elif name=='message':
   text,colour,player,shadow,x,y=args;assert self.read(text,10)==b'wrong way\0';args=(self.read(colour,4),player,shadow,x,y)
  self.trace.append((name,args));self.u.reg_write(UC_X86_REG_EAX,value)
 def sprite(self):
  src,dst,texture,layer,angle,centre,unused,colour,flags=self.args(9)
  assert texture==HEAP+0x6000 and layer==2 and not angle and not centre and not unused
  self.trace.append(('sprite',self.read(src,8),self.read(dst,8),self.read(colour,4),flags))
 def format(self):
  dest,fmt=self.args(2);raw=self.read(fmt,10)
  if raw.startswith(b'%s.cod\0'):
   assert self.args(3)[2]==HEAP+0x7000;self.u.mem_write(dest,b'route.cod\0')
  else:assert raw==b'wrong way\0';self.u.mem_write(dest,raw)
  self.trace.append(('format',))
 def file(self):
  archive,name,a,size,b=self.args(5);assert archive==HEAP+0x7100 and not a and not size and not b and self.read(name,10)==b'route.cod\0'
  self.trace.append(('file',));self.u.reg_write(UC_X86_REG_EAX,0 if self.case%2 else HEAP+0x3000)
 def initialize(self,case):
  self.reset();self.case=case;self.cheat=case%2;self.u.mem_write(HEAP,random.Random(case).randbytes(0x10000))
  self.put(self.addr(0x520b74),'<I',HEAP+0x1000);self.put(HEAP+0x1000,'<2I',640+(case%3)*160,480+(case%3)*120)
  self.put(self.addr(0x537350),'<i',-1 if case%3 else 2);self.put(self.addr(0x52f2ac),'<I',((case//32)%2<<24)|(case%2<<26))
  self.put(self.addr(0x537248),'<2i',case%3==1,case%3==2)
  self.put(self.addr(0x537358),'<I',HEAP+0x3000);self.put(self.addr(0x537094),'<I',HEAP+0x4000)
  self.put(self.addr(ARROW_RECTS),'<I',HEAP+0x5000);self.put(self.addr(ARROW_TEXTURE),'<I',HEAP+0x6000)
  for i,rect in enumerate(RECTS):self.put(HEAP+0x5000+i*8,'<4h',*rect)
  self.put(self.addr(0x6e2ef4),'<4096i',*SINES)
  self.u.mem_write(HEAP+0x7000,b'route\0');self.u.mem_write(HEAP+0x7200,b'wrong way\0');self.put(HEAP+0x2000,'<B',case%3)
  for i in range(256):self.put(HEAP+0x3000+i*4,'<I',(i%16)|((i%4)<<4)|((i%16)<<17)|((i%4)<<21))
  for i in range(10):self.put(self.addr(0x5371d0)+i*12,'<3I',(i%9+1)|((case%3)<<4),(2<<17) if i%2 else 0,0x100|(case%51))
  self.trace=[];return self.read(HEAP,0x10000)
 def checked(self,target,args):
  for i,r in enumerate(REGS):self.u.reg_write(r,0x1212a000+i)
  self.invoke(target,args);assert self.u.reg_read(UC_X86_REG_ESP)==STACK+0xff04+len(args)*4
  assert all(self.u.reg_read(r)==0x1212a000+i for i,r in enumerate(REGS));return self.u.reg_read(UC_X86_REG_EAX)
 def decode(self,id,cheat):
  before=self.initialize(cheat);out=HEAP+0x8000;result=self.checked(0x418580,[id,out,out+8,out+16,out+20])
  model=icon_model(id,cheat);assert result==model[0] and self.read(out+8,8)==model[1] and self.read(out+16,4)==struct.pack('<I',model[2]) and self.read(out+20,4)==model[3]
  assert self.read(out,4)==struct.pack('<I',HEAP+0x6000),'texture output'
  after=self.read(HEAP,0x10000);assert after[:0x8000]==before[:0x8000] and after[0x8018:]==before[0x8018:]
  return after,self.trace
 def run(self,kind,case,player=0,callId=0,previous=255,free=0):
  before=self.initialize(case)
  if kind=='queue':
   for i in range(5):self.put(self.addr(0x5371d0)+(player*5+i)*12+8,'<I',0x100 if i<free else 0x200)
   self.checked(0x4174e0,[player,callId,previous,77])
  elif kind=='draw':
   if case%5==0:self.put(self.addr(0x5371d0)+player*60+8,'<I',case%51)
   self.checked(0x4177d0,[player,0x1234])
  elif kind=='load':self.checked(0x416720,[])
  elif kind=='release':assert self.checked(0x418550,[])==1
  assert self.read(HEAP,0x10000)==before,'immutable guarded heap'
  return self.read(self.addr(0x5371d0),120),self.read(self.addr(0x537358),4),self.read(self.addr(0x537094),4),self.trace

# Resolved original globals; producer annotations in StageUI.cpp are authoritative.
ARROW_RECTS=0x537088
ARROW_TEXTURE=0x537098

def main():
 e=json.loads(Path(sys.argv[1]).read_text());a=Calls(ROOT/'cmr2bin/CMR2.exe');b=Calls(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e);number=0
 for t,b1,b2,b3,cheat in itertools.product(range(16),range(4),[0,1,2,3,14],range(4),range(2)):
  id=t|(b1<<4)|(b2<<17)|(b3<<21);assert a.decode(id,cheat)==b.decode(id,cheat);number+=1
 for args in itertools.product(range(4),range(2),[255,0,1,4,7,11],[255,0,3,5],range(6)):
  assert a.run('queue',*args)==b.run('queue',*args),('queue',args);number+=1
 for kind,case,player in itertools.product(['draw','load','release'],range(128),range(2)):
  assert a.run(kind,case,player)==b.run(kind,case,player),('calls',kind,case,player);number+=1
 print(f'PASS: {number} co-driver decoder/queue/draw/load/release scenarios; independent icon model, guarded memory, pointer outputs, complete queue, provider traces and ABI')
if __name__=='__main__':main()
