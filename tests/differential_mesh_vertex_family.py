#!/usr/bin/env python3
"""Whole vertex consumers vs original x86, with real fixed-point/CRT helpers.

Only clone allocation and final mesh upload/part construction are controlled.
Complete guarded heaps, source immutability, allocation sizes and stdcall ABI
are compared. Colour writers and vertex readers also have independent models.
"""
import itertools,json,math,random,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ESP,UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP,UC_X86_REG_FPCW
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT,HEAP,STACK
from differential_object_matrix import identity
from differential_deform_mesh_records import SQRT

SAVED=(UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP)
SRC,OBJECT,VERTICES,TRIANGLES,WORK,LEVELS,DEST=(HEAP+x for x in (0x1000,0x1400,0x2000,0x3000,0x4000,0x5000,0x6000))
class Vertices(Lighting):
 def __init__(self,path,entities=None):
  super().__init__(path,entities);self.callbacks={}
 def leaf(self,address,n,fn):self.callbacks[self.addr(address)]=(n*4,fn)
 def checked(self,address,args):
  for i,r in enumerate(SAVED):self.u.reg_write(r,0x1234a000+i)
  self.invoke(address,args)
  assert self.u.reg_read(UC_X86_REG_ESP)==STACK+0xff04+4*len(args),'stdcall'
  assert all(self.u.reg_read(r)==0x1234a000+i for i,r in enumerate(SAVED)),'callee registers'
 def begin(self,seed):
  self.reset();self.u.mem_write(HEAP,random.Random(seed).randbytes(0x10000));self.u.reg_write(UC_X86_REG_FPCW,0x27f)
 def allocate(self):
  n,=self.args(1);p=self.cursor;self.cursor=(p+n+31)&~15;assert self.cursor<HEAP+0xf000
  self.allocations.append((p,n));self.u.reg_write(UC_X86_REG_EAX,p)
 def mesh(self):
  self.put(self.addr(0x67f224),'<i',18);self.put(DEST+0x118,'<i',0);self.u.reg_write(UC_X86_REG_EAX,DEST)
 def final(self,kind):
  p,=self.args(1);assert p==DEST;self.trace.append((kind,self.read(DEST,0x120)))
 def clone_initial(self,seed,n,tri,rotation):
  self.begin(seed);self.callbacks={};self.cursor=HEAP+0x8000;self.allocations=[]
  self.put(self.addr(0x6e0ef4),'<4096H',*struct.unpack('<4096H',SQRT))
  self.put(self.addr(0x67f224),'<i',17);self.put(self.addr(0x67f22c),'<i',2)
  self.put(self.addr(0x667364),'<4h',-12,-13,-14,-15)
  self.put(self.addr(0x677124),'<4I',0,0,0,0)
  self.put(SRC+0xc,'<Ii',VERTICES,n);self.put(SRC+0x20,'<IIi',WORK,TRIANGLES,tri);self.put(SRC+0x34,'<I',LEVELS)
  m=identity();m[12:15]=[3*65536,-2*65536,65536]
  if rotation==1:m[0:3],m[4:7],m[8:11]=[0,0,65536],[0,65536,0],[-65536,0,0]
  if rotation==2:m[0:3],m[4:7],m[8:11]=[-65536,0,0],[0,0,65536],[0,65536,0]
  if rotation==3:m[0:3],m[4:7],m[8:11]=[0,0,0],[0,2*65536,0],[0,0,-3*65536]
  self.put(OBJECT+0x18,'<16i',*m)
  for i in range(n):self.put(VERTICES+i*48,'<6f',(i-3)*.25,(i+2)*.5,-i*.125,.25,-.5,1.)
  self.initial_heap=self.read(HEAP,0x10000)
  self.leaf(0x4ab8a0,0,self.mesh);self.leaf(0x4aad70,1,self.allocate)
  self.leaf(0x4b1ac0,1,lambda:self.final('parts'));self.leaf(0x4b1d00,1,lambda:self.final('upload'))
 def clone(self,seed,n,tri,rotation):
  self.clone_initial(seed,n,tri,rotation);self.checked(0x4abaf0,[SRC,OBJECT])
  assert self.u.reg_read(UC_X86_REG_EAX)==DEST
  expected_requests=[n*48,tri*76,tri*20,n*4]
  assert [n for _,n in self.allocations]==expected_requests
  assert [k for k,_ in self.trace]==['parts','upload'] and self.trace[0][1]==self.trace[1][1]
  header=self.read(DEST,0x120)
  assert header[:12]==self.initial_heap[0x1000:0x100c]
  assert header[0x108:0x10c]==self.initial_heap[0x1108:0x110c]
  assert header[0x10c:0x111]==bytes(5)
  assert header[0x111:0x118]==self.initial_heap[0x1111:0x1118]
  assert header[0x118:0x120]==bytes(8)
  assert self.read(SRC,0x5000)==self.initial_heap[0x1000:0x6000],'immutable source and object'
  for (at,size),source in zip(self.allocations,[VERTICES,TRIANGLES,WORK,LEVELS]):
   if source!=VERTICES:assert self.read(at,size)==self.read(source,size)
   assert self.read(at-16,16)==self.initial_heap[at-HEAP-16:at-HEAP]
   assert self.read(at+size,16)==self.initial_heap[at-HEAP+size:at-HEAP+size+16]
  assert self.read(self.addr(0x667364),8)==struct.pack('<4h',-12,-13,17,-15)
  assert self.read(self.addr(0x677124),16)==struct.pack('<4I',0,0,DEST,0)
  return self.read(HEAP,0x10000),self.trace,self.canonical_clone(n,tri)
 def canonical_clone(self,n,tri):
  h=self.read(DEST,0x120)
  fields=h[:12]+h[0x10:0x14]+h[0x28:0x34]+h[0x104:0x105]+h[0x108:0x120]
  return fields+b''.join(self.read(at,size) for at,size in self.allocations)
 def colours(self,seed,n,kind,alpha,missing):
  self.begin(seed);self.callbacks={};self.put(HEAP+0x7000,'<2I',0x7f123456,0xa5abcdef)
  specs=[(0x5920e8,0x592140),(0x5920f0,0x592142),(0x5920f8,0x592144),(0x5920d0,0x59213c),(0x5920d8,0x592138),(0x5920e0,0x59213a)]
  # g_groundReferenceMesh is a separate global; resolve it by annotation.
  for j,(ptr,count) in enumerate(specs):
   self.put(self.addr(ptr),'<I',0 if missing and j in (2,5) else SRC+j*0x120)
   self.put(self.addr(count),'<h',n);self.put(SRC+j*0x120+0xc,'<I',VERTICES+j*0x200)
   for i in range(n):self.put(VERTICES+j*0x200+i*48,'<3f',i*.25,-i*.5,i*.125)
  self.put(self.addr(0x592147),'<B',alpha);self.put(self.addr(0x592146),'<B',alpha)
  self.put(self.addr(0x592108),'<3i',0,0,0)
  for a in (0x5920fc,0x592100,0x592104):self.put(self.addr(a),'<i',0)
  before=self.read(HEAP,0x10000);expected=bytearray(before)
  address=[0x4923d0,0x492470,0x492520,0x4920d0,0x492220][kind]
  js=[[0],[1],[2],[3],[4,5]][kind]
  for j in js:
   if missing and j in (2,5):continue
   for i in range(n):
    value=(0x56<<16)|(0x34<<8)|0x12|(0x7f<<24)
    if kind>=3 and i==0 and j!=5:value=(0xef<<16)|(0xcd<<8)|0xab|(0xa5<<24)
    spec=(255 if kind==0 else alpha)<<24
    struct.pack_into('<II',expected,0x2000+j*0x200+i*48+24,value,spec)
  self.checked(address,[HEAP+0x7000]+([HEAP+0x7004] if kind>=3 else []))
  assert self.read(HEAP,0x10000)==expected,('colour model and whole guarded heap',seed,n,kind,alpha,missing)
  return self.read(HEAP,0x10000)
 def height_colours(self,seed,n,alpha,blend):
  self.begin(seed);self.callbacks={}
  self.put(self.addr(0x5920c8),'<I',SRC);self.put(self.addr(0x59213e),'<h',n)
  self.put(SRC+0xc,'<I',VERTICES)
  self.put(self.addr(0x592114),'<3i',0,0,0)
  self.put(self.addr(0x592120),'<i',-65536);self.put(self.addr(0x592124),'<i',32768)
  self.put(self.addr(0x592147),'<B',alpha)
  low,high,ref=[10,20,30],[170,160,150],[210,180,100]
  self.u.mem_write(HEAP+0x7000,bytes(low+high+ref))
  for i in range(n):self.put(VERTICES+i*48,'<3f',i*.25,-i*.5,i*.125)
  expected=bytearray(self.read(HEAP,0x10000))
  delta=[(b-a)*65536 for a,b in zip(low,high)]
  reference=[a+((d*32768)>>32) for a,d in zip(low,delta)]
  reference=[a+(((b-a)*blend)>>16) for a,b in zip(reference,ref)]
  for i in range(n):
   t=max(0,min(65536,((int(-i*.5*65536)+65536)*32768)>>16))
   rgb=reference if i==0 else [max(0,min(255,a+((d*t)>>32))) for a,d in zip(low,delta)]
   value=((0xffffff00|rgb[0])<<16|(rgb[1]<<8)|(rgb[2]&255))&0xffffffff
   struct.pack_into('<II',expected,0x2000+i*48+24,value,alpha<<24)
  self.checked(0x491d40,[HEAP+0x7000,HEAP+0x7003,HEAP+0x7006,blend&0xffffffff])
  assert self.read(HEAP,0x10000)==expected,('height colour model and whole heap',seed,n,alpha,blend)
  assert self.read(self.addr(0x592148),4)==struct.pack('<i',1)
  return self.read(HEAP,0x10000)
 def read_vertex(self,seed,mesh,index):
  self.begin(seed);self.callbacks={};self.put(HEAP+0x7000,'<3I',SRC,SRC+0x120,SRC+0x240)
  for j in range(3):
   self.put(SRC+j*0x120+0xc,'<I',VERTICES+j*0x200)
   for i in range(8):self.put(VERTICES+j*0x200+i*48,'<3f',(i-j)*.25,(j+i)*-.5,(i+3)*.125)
  before=self.read(HEAP,0x10000);expected=bytearray(before)
  pos=struct.unpack_from('<3f',before,0x2000+mesh*0x200+index*48)
  struct.pack_into('<3i',expected,0x7100,*(int(v*65536) for v in pos))
  self.checked(0x46b440,[HEAP+0x7000,mesh,index,HEAP+0x7100])
  assert self.read(HEAP,0x10000)==expected,'fixed vertex model and complete heap'
  return self.read(HEAP,0x10000)

def main():
 e=json.loads(Path(sys.argv[1]).read_text());a=Vertices(ROOT/'cmr2bin/CMR2.exe');b=Vertices(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e)
 count=0
 for args in itertools.product(range(4),(0,1,4,8),(0,1,3),range(4)):
  assert a.clone(*args)==b.clone(*args),('clone',args);count+=1
 for args in itertools.product(range(4),(0,1,4,8),range(5),(0,37,255),(0,1)):
  assert a.colours(*args)==b.colours(*args),('colour',args);count+=1
 for args in itertools.product(range(4),range(3),range(8)):
  assert a.read_vertex(*args)==b.read_vertex(*args),('read',args);count+=1
 for args in itertools.product(range(4),(0,1,4,8),(0,37,255),(-65536,0,65536,98304)):
  assert a.height_colours(*args)==b.height_colours(*args),('height colours',args);count+=1
 print(f'PASS: {count} whole clone, colour and fixed-vertex consumers vs original x86; allocation order, complete heaps/source guards, independent colour/position models and callee ABI')
if __name__=='__main__':main()
