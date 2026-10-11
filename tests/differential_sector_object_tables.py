#!/usr/bin/env python3
"""Sector entry lifecycle against original x86 with complete guarded memory.

Allocation, cloning, scene ownership and final contact leaves are controlled;
entry publication, bounds/projection construction, flags, mesh classification,
moving-object setup/eviction, interpolation publication and collision traversal
execute their actual bodies. Providers preserve stdcall ABI and call order.
"""
import itertools,json,math,random,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ESP,UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP
from differential_stage_lighting import Lighting,signed
from differential_menu_list import ROOT,HEAP,STACK
from differential_object_matrix import identity

GLOBALS=(0x58c938,0x58c94c,0x58c958,0x58ca6c,0x58c940,0x58c960,0x58ca78,0x58c93c,0x58c964,0x58c954,0x58c944,0x58c948,0x58ca70,0x58c95c,0x58c928,0x58c92c,0x58c930)
OWNER=0x5894e0
PRESERVED=(UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP)
class Tables(Lighting):
 def __init__(self,path,entities=None):
  super().__init__(path,entities);self.callbacks={}
  self.leaf(0x4aad70,1,self.allocate);self.leaf(0x4ac6f0,1,self.scene)
  self.leaf(0x4ab9c0,0,lambda:self.emit('reset-clones'))
  self.leaf(0x4abaf0,2,self.clone);self.leaf(0x411060,0,lambda:self.emit('scene-root',result=HEAP+0x3c00))
  self.leaf(0x49c0a0,2,self.register);self.leaf(0x4074f0,0,lambda:self.emit('mode',result=self.mode))
  self.leaf(0x42b5f0,1,self.car);self.leaf(0x490c90,5,self.ground)
  self.leaf(0x4b8aa0,1,self.remove);self.leaf(0x4b8b10,1,self.insert)
  self.leaf(0x4b5320,2,self.action);self.leaf(0x406930,1,lambda:self.emit('validate',*self.args(1)))
  self.leaf(0x42b700,0,lambda:self.emit('car-count',result=2))
  self.leaf(0x487b80,4,self.spheres)
  self.leaf(0x486c30,4,lambda:self.emit('view-box'))
  self.leaf(0x487f60,4,lambda:self.emit('turned-contact',result=self.contact))
  self.leaf(0x489750,3,lambda:self.emit('box-contact',self.args(3)[2],result=self.contact))
  self.leaf(0x48be20,4,lambda:self.emit('obstacle-contact',result=1))
  self.leaf(0x487e50,2,lambda:self.emit('shadow'))
 def leaf(self,address,n,callback):self.callbacks[self.addr(address)]=(n*4,callback)
 def emit(self,name,*args,result=0):self.trace.append((name,*args));self.u.reg_write(UC_X86_REG_EAX,result&0xffffffff)
 def allocate(self):
  size,=self.args(1);p=self.cursor;self.cursor=(p+size+15)&~15;assert self.cursor<=HEAP+0x10000
  self.allocations.append((p,size));self.emit('allocate',size,result=p)
 def scene(self):
  root,=self.args(1);assert root==HEAP+0x3c00;p=HEAP+0x4000+self.created*0x18c;self.created+=1
  self.emit('scene',self.created-1,result=p)
 def clone(self):
  mesh,source=self.args(2);index=(source-HEAP-0x1000)//0xa0
  assert mesh==HEAP+0x2000+index*0x120
  self.emit('clone',index,result=0 if index==self.failed else mesh)
 def register(self):
  callback,param=self.args(2);assert param==0
  name=next(name for address,name in [(0x472720,'entries'),(0x46f500,'meshes')] if self.addr(address)==callback)
  self.emit('register',name)
 def car(self):index,=self.args(1);self.emit('car',index,result=HEAP+0xc000+index*0xc24)
 def ground(self):
  pos,normal,triangle,surface,flag=self.args(5);assert flag==0
  self.put(normal,'<3i',0,65536,0);self.put(triangle,'<h',7);self.put(surface,'<H',3);self.emit('ground',result=0)
 def remove(self):node,=self.args(1);self.put(node+0x24,'<h',-1);self.emit('remove',node)
 def insert(self):node,=self.args(1);self.put(node+0x24,'<h',2);self.emit('insert',node)
 def action(self):node,value=self.args(2);self.emit('action',node,signed(value),result=value+1)
 def spheres(self):
  a,b,left,right=self.args(4);self.emit('spheres',a,b,self.read(left,12),self.read(right,12),result=self.overlap)
 def checked(self,target,args):
  for i,reg in enumerate(PRESERVED):self.u.reg_write(reg,0x1234a000+i)
  self.invoke(target,[v&0xffffffff for v in args])
  assert self.u.reg_read(UC_X86_REG_ESP)==STACK+0xff00+4+len(args)*4,(hex(target),'stdcall')
  assert all(self.u.reg_read(reg)==0x1234a000+i for i,reg in enumerate(PRESERVED))
  return self.u.reg_read(UC_X86_REG_EAX)
 def initialize(self,seed,count,mode,failed=-1):
  self.reset();self.u.mem_write(HEAP,random.Random(seed).randbytes(0x10000));self.trace=[]
  self.mode=mode;self.failed=failed;self.cursor=HEAP+0x9000;self.allocations=[];self.created=0
  self.put(self.addr(0x58ca68),'<i',0);self.put(self.addr(0x58c950),'<i',count)
  self.put(self.addr(0x72d568),'<I',3)
  self.put(self.addr(OWNER),'<'+str(0x3448)+'s',bytes(0x3448));self.u.mem_write(self.addr(0x5894b8),bytes(40))
  for a in GLOBALS:self.put(self.addr(a),'<I',0)
  for sector in range(3):
   at=HEAP+0x800+sector*0x88;self.u.mem_write(at,bytes(0x88));self.put(self.addr(0x71f608)+sector*4,'<I',at)
   members=list(range(sector,count,3));self.put(at+0x14,'<I',HEAP+0x1000+members[0]*0xa0 if members else 0)
  for index in range(count):
   obj=HEAP+0x1000+index*0xa0;mesh=HEAP+0x2000+index*0x120;record=HEAP+0x3000+index*0xb8
   self.u.mem_write(obj,bytes(0xa0));self.u.mem_write(mesh,bytes(0x120));self.u.mem_write(record,bytes(0xb8))
   self.put(obj,'<3iI',index*65536,12345+index*123,(-index)*32768,mesh)
   self.put(obj+0x10,'<IB',0x2001000 if index&1 else 0,0xff)
   self.put(obj+0x18,'<16i',*identity());self.put(obj+0x98,'<I',obj+3*0xa0 if index+3<count else 0)
   self.put(self.addr(0x669364)+index*4,'<I',obj)
   for r in range(2):
    matrix=identity()
    if index==2:matrix[:8]=[0,65536,0,0,-65536,0,0,0]
    at=record+r*0x5c;self.put(at,'<B',(index+r)%2);self.put(at+4,'<16i',*matrix)
    self.put(at+0x44,'<3i',(index+1)*32768,98304+index*16384,65536+index*65536)
    self.put(at+0x58,'<I',record+0x5c if r==0 else 0)
   self.put(mesh+0x10c,'<IB',record,2)
   self.put(mesh+0xc,'<Ii',HEAP+0x3800+index*0x60,2)
   for vertex in range(2):self.put(HEAP+0x3800+index*0x60+vertex*0x30,'<3f',(-1 if vertex else 1)*(index+1),float(index+2),0.25 if index%2 else 4.0)
  for i in range(40):
   node=HEAP+0x4000+i*0x18c;self.u.mem_write(node,bytes(0x18c));self.put(node+0x24,'<h',-1)
  for i in range(2):
   car=HEAP+0xc000+i*0xc24;self.u.mem_write(car,bytes(0xc24));self.put(car+0x408,'<3i',32768+i*65536,0,-49152)
   self.put(car+0x778,'<i',65536 if seed%2 else 16384)
  self.put(self.addr(0x6e0ef4),'<4096H',*[int(math.sqrt(8+16*i)*256)&65535 for i in range(4096)])
 def snapshot(self):
  owner=bytearray(self.read(self.addr(OWNER),0x3448))
  for index in range(40):
   at=index*0x128+0xc8;pointer=struct.unpack_from('<I',owner,at)[0]
   if self.addr(OWNER)<=pointer<self.addr(OWNER)+0x2e40:struct.pack_into('<I',owner,at,OWNER+pointer-self.addr(OWNER))
  return self.read(HEAP,0x10000),tuple(self.read(self.addr(a),4) for a in GLOBALS),bytes(owner),self.trace,self.read(self.addr(0x5894b8),40)
 def pipeline(self,seed,count,mode,failed):
  self.initialize(seed,count,mode,failed);self.checked(0x471dd0,[])
  assert self.created==40
  self.produced=self.snapshot();self.produced_summary=self.canonical(count)
  entries=self.get(0x58c94c);n=self.get(0x58ca6c)
  for sector in (0,1,2,3,0xffffffff):
   for target in (0x471b30,0x471b80):self.checked(target,[sector,HEAP+0x7f00])
  for index in range(n):
   element=entries+index*8;self.put(HEAP+0x7f10,'<I',element)
   self.checked(0x471d80,[HEAP+0x7f10,index%2,0]);self.checked(0x471d40,[HEAP+0x7f10,index%2])
   self.checked(0x471cc0,[HEAP+0x7f20,HEAP+0x7f10]);self.checked(0x470240,[element,index%2])
  self.checked(0x471bf0,[0]);self.checked(0x471bf0,[1])
  if n and self.read(self.addr(OWNER)+0x2e40,1)!=b'\x00':
   self.checked(0x46fe70,[entries,1,0]);self.checked(0x471a60,[0])
   if seed>=2 and count==5:
    for i in range(40):self.checked(0x46fe70,[entries+(i%n)*8,i%3,i%2])
   self.checked(0x471a60,[0])
  self.lifecycle_summary=self.canonical(count)
  for i in range(n):
   element=entries+i*8;bounds=struct.unpack('<I',self.read(element+4,4))[0];projection=struct.unpack('<I',self.read(bounds+4,4))[0]
   if projection:
    self.put(HEAP+0x8000+0x28,'<I',0);self.checked(0x487c40,[HEAP+0x8000,element,HEAP+0x7f20]);self.checked(0x487c40,[HEAP+0x8000,element,HEAP+0x7f20])
   else:self.checked(0x487e00,[HEAP+0x7f20,element])
  self.checked(0x4702a0,[])
  for offset in [-8,n*8,(n+1)*8,0x7fffff00]:
   element=(entries+offset)&0xffffffff;self.put(HEAP+0x7f10,'<I',element)
   self.checked(0x471d40,[HEAP+0x7f10,0]);self.checked(0x471d80,[HEAP+0x7f10,0,1])
  return self.snapshot()
 def collide(self,seed,count,mode,overlap,contact,moving_only):
  self.pipeline(seed,count,mode,-1);self.overlap=overlap;self.contact=contact
  car=HEAP+0xc000;self.put(car+0xb00,'<4h',0,1,2,-1);self.put(car+0xb1a,'<b',0);self.put(car+0xc18,'<i',moving_only)
  before=self.read(HEAP,0x10000);result=self.checked(0x4878a0,[car])
  globals=[self.read(self.addr(a),size) for a,size in [(0x591490,4),(0x591494,4),(0x591498,12),(0x5914d8,4)]]
  box=self.read(self.addr(0x5915f8),0x90);pointers=struct.unpack('<2I',self.read(self.addr(0x5915f8)+0x90,8))
  center=self.read(pointers[1],12) if pointers[1] else b''
  return self.snapshot(),result,globals,box,pointers[0],center
 def canonical(self,count):
  out=bytearray()
  def put(*values):out.extend(struct.pack('<'+'I'*len(values),*[v&0xffffffff for v in values]))
  a,b=self.get(0x58c95c),self.get(0x58ca6c);put(a,b,self.read(self.addr(OWNER)+0x2e40,1)[0],self.read(self.addr(OWNER)+0x3444,1)[0])
  for letter,index_address,count_address,entries_address,table_address,total in [('a',0x58c940,0x58c960,0x58ca78,0x58c948,a),('b',0x58c93c,0x58c964,0x58c954,0x58c94c,b)]:
   indexes=self.get(index_address);counts=self.get(count_address);spans=self.get(entries_address);table=self.get(table_address)
   for sector in range(3):
    index=struct.unpack('<h',self.read(indexes+sector*2,2))[0];put(index)
    put(0,0) if index<0 else put(struct.unpack('<H',self.read(counts+index*2,2))[0],(struct.unpack('<I',self.read(spans+index*4,4))[0]-table)//8)
   for i in range(total):
    object,bounds=struct.unpack('<2I',self.read(table+i*8,8));radius,projection=struct.unpack('<2I',self.read(bounds,8));put((object-HEAP-0x1000)//0xa0,(bounds-self.get(0x58ca70))//8,radius,int(projection!=0));out.extend(self.read(projection,12) if projection else bytes(12))
  for index in range(count):
   object=HEAP+0x1000+index*0xa0;out.extend(self.read(object,12));out.extend(self.read(object+0x10,5))
  n=b;out.extend(self.read(self.get(0x58c938),n) if n else b'');out.extend(self.read(self.get(0x58c958),n*4) if n else b'')
  if n:out.extend(self.read(self.get(0x58c928),n))
  mesh_count=self.read(self.addr(OWNER)+0x2e40,1)[0]
  for i in range(mesh_count):
   mesh=struct.unpack('<I',self.read(self.get(0x58c92c)+i*4,4))[0];put((mesh-HEAP-0x2000)//0x120,struct.unpack('<I',self.read(self.get(0x58c930)+i*4,4))[0])
  for i in range(40):
   state=self.addr(OWNER)+i*0x128;entry,node=struct.unpack('<2I',self.read(state,8));matrix=struct.unpack('<I',self.read(state+0xc8,4))[0]
   put((entry-self.get(0x58c94c))//8 if entry else -1,(node-HEAP-0x4000)//0x18c if node else -1,1 if matrix==state+8 else 0)
   out.extend(self.read(state+8,192));out.extend(self.read(state+0xcc,92))
   node=HEAP+0x4000+i*0x18c;mesh=struct.unpack('<I',self.read(node+0xc,4))[0]
   put((mesh-HEAP-0x2000)//0x120 if mesh else -1,struct.unpack('<h',self.read(node+0x24,2))[0]);out.extend(self.read(node+0x98,128));out.extend(self.read(node+0x178,5))
  out.extend(self.read(self.addr(OWNER)+0x2f44,1280));out.extend(self.read(self.addr(0x5894b8),40))
  return bytes(out)
 def get(self,address):return struct.unpack('<I',self.read(self.addr(address),4))[0]

def main():
 entities=json.loads(Path(sys.argv[1]).read_text());a=Tables(ROOT/'cmr2bin/CMR2.exe');b=Tables(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities);n=0
 for args in itertools.product(range(4),[0,1,2,5], [1,2,3],[-1,1]):
  x=a.pipeline(*args);y=b.pipeline(*args);assert x==y,(args,[(i,x[i] if i==3 else 'memory',y[i] if i==3 else 'memory') for i in range(len(x)) if x[i]!=y[i]]);n+=1
 for args in itertools.product(range(2),[0,1,2,5],[1,2,3],[0,1],[0,1],[0,1]):
  assert a.collide(*args)==b.collide(*args),('collision',args);n+=1
 print(f'PASS: {n} sector-object original/rebuilt lifecycle scenarios; full guarded heap, runtime globals/owner, provider order and stdcall ABI')
if __name__=='__main__':main()
