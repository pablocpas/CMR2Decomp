#!/usr/bin/env python3
"""Both original triangle queues: independent record models, layer bounds and ABI."""
import itertools,json,random,struct,sys
from pathlib import Path
from differential_car_info_records import Records,put
from differential_menu_list import ROOT,HEAP

LAYERS=(0x7dd168,0x803168,0x730fd0,0x7acfd0)
COUNTS=(0x81616c,0x816170,0x816174,0x816178)
CAPS=(1024,512,2048,1)
SOURCE,DEST,TEXTURE=HEAP+0x1000,HEAP+0x7000,HEAP+0x8000

def cases():
 for kind,flags,status,overflow,seed in itertools.product(('copy','fixed'),(8,16,32,64,0x78,0xe,0x26),range(5),(0,1),(3,19)):
  k=next(i for i,mask in enumerate((8,16,32,64)) if flags&mask)
  yield kind,flags,(0,CAPS[k]-1,CAPS[k],CAPS[k]+1,0xffffffff)[status],overflow,seed
 for kind,flags,overflow,seed in itertools.product(('copy','fixed'),(DEST,DEST|1,DEST|7),(0,1),(3,19)):
  yield kind,flags,17,overflow,seed

def input_data(kind,seed):
 r=random.Random(seed)
 if kind=='copy':return r.randbytes(144)
 values=(-2147483648,2147483647,-65537,-1,0,1,65535,65536,123456789)
 return b''.join(struct.pack('<3i4s2i',*(values[(seed+i*3+j)%len(values)] for j in range(3)),r.randbytes(4),*(values[(seed+i+j+5)%len(values)] for j in range(2))) for i in range(3))

def model(kind,before,source,texture,flags):
 out=bytearray(before)
 struct.pack_into('<2I',out,144,texture,flags)
 if kind=='copy':out[:144]=source
 else:
  for i in range(3):
   x,y,z,c,u,v=struct.unpack_from('<3i4s2i',source,i*24)
   for offset,value in ((0,x),(4,y),(8,z),(32,u),(36,v)):struct.pack_into('<f',out,i*48+offset,value/65536)
   struct.pack_into('<2I',out,i*48+24,(c[3]<<24)|(c[0]<<16)|(c[1]<<8)|c[2],0xff000000)
 return bytes(out)

class Queues(Records):
 def run(self,kind,flags,count,overflow,seed):
  h=self.begin(seed);source=input_data(kind,seed);h[0x1000:0x1000+len(source)]=source
  k=next((i for i,m in enumerate((8,16,32,64)) if flags&m),None)
  over=0x81618d if kind=='copy' else 0x81618c
  self.put(self.addr(over),'<B',overflow);self.allowed.add((self.addr(over),1))
  initial_counts=[13,11,7,0];initial_counts[k if k is not None else 0]=count
  for a,n in zip(COUNTS,initial_counts):self.put(self.addr(a),'<I',n);self.allowed.add((self.addr(a),4))
  before=[self.read(self.addr(a),n*152) for a,n in zip(LAYERS,CAPS)]
  expected=[bytearray(b) for b in before]
  full=count>=CAPS[k] if k is not None else False
  record=flags if k is None else self.addr(LAYERS[k])+min(count,CAPS[k]-1)*152
  poison=random.Random(seed+99).randbytes(152)
  if k is None:
   h[record-HEAP:record-HEAP+152]=poison
   expected_heap=bytearray(h);expected_heap[record-HEAP:record-HEAP+152]=model(kind,poison,source,TEXTURE,flags)
  else:
   # Paint only a valid record; capped calls must leave even that guard intact.
   self.u.mem_write(record,poison)
   start=(record-self.addr(LAYERS[k]));expected[k][start:start+152]=poison
   if not full:
    expected[k][start:start+152]=model(kind,poison,source,TEXTURE,flags)
    self.allowed.update((record+i,4) for i in range(0,152,4));initial_counts[k]+=1
   expected_heap=h
  args=[SOURCE,TEXTURE,flags] if kind=='copy' else [0x12345678,SOURCE,SOURCE+24,SOURCE+48,TEXTURE,flags]
  actual=self.invoke_guarded(0x4bbc60 if kind=='copy' else 0x4bba00,args)
  assert actual==bytes(expected_heap),'full guarded input/output heap'
  assert all(self.read(self.addr(a),n*152)==bytes(b) for a,n,b in zip(LAYERS,CAPS,expected)),'complete layer records and guards'
  assert tuple(struct.unpack('<I',self.read(self.addr(a),4))[0] for a in COUNTS)==tuple(initial_counts),'unsigned counts and priority'
  actual_over=self.read(self.addr(over),1)
  assert actual_over==bytes([1 if full else overflow]),'separate sticky overflow flag'
  result=actual[flags-HEAP:flags-HEAP+152] if k is None else self.read(record,152)
  return result,tuple(initial_counts),actual_over

def main():
 e=json.loads(Path(sys.argv[1]).read_text());a=Queues(ROOT/'cmr2bin/CMR2.exe');b=Queues(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e)
 count=0
 for args in cases():assert a.run(*args)==b.run(*args),args;count+=1
 print(f'PASS: {count} complete queue scenarios; independent vertex/copy models, four complete guarded layers, direct destinations, unsigned capacity bounds, mask priority, separate sticky overflow flags, stdcall/callee-saved ABI')
if __name__=='__main__':main()
