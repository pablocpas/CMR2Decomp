#!/usr/bin/env python3
"""Whole original/rebuilt address walks: guarded tables, primitive models, ABI."""
import itertools,json,random,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX
from differential_car_info_records import Records,put
from differential_menu_list import ROOT,HEAP
LINE_ARRAY=HEAP+0x4000

def release_cases():return itertools.product(('replay','parts','volume'),(0,1,0x55,0x80,0xaa,0xff),(-1,0,1,8),(0,1),(5,31))
def flag_cases():return itertools.product(('ready','block'),range(32))
class Walks(Records):
 def addr(self,a):
  return self.member(a,0x588cd4) if a==0x588d14 else super().addr(a)
 def region(self,start,size):
  self.allowed.update((start+i,w) for w in (1,2,4,8) for i in range(size-w+1))
 def provider(self,a,n):
  args=tuple(self.args(n))
  if self.mode=='stage':
   self.trace.append((a,args));value=1 if a==0x456ca0 else 0
  elif self.mode=='controls':
   if a in (0x40bbc0,0x40bbe0):args=tuple(v&65535 for v in args)
   self.trace.append((a,args));value=0
   if a==0x40bbb0:value=LINE_ARRAY
   elif a==0x40bbc0:value=self.mapping[args[0]]
  elif self.mode=='championship':
   self.trace.append((a,args));value=0
   if a==0x40ce40:value=(6,4,3,2,1,0,0,0)[args[0]]
   elif a==0x406950:value=self.round
   elif a==0x40d520:
    assert args==(self.addr(0x5335b8),self.addr(0x5335b8)+88,0,8,0)
    self.u.mem_write(args[1],bytes(self.sorted))
  else:
   arg,=args
   self.trace.append(('free' if a==0x4aade0 else 'volume',arg));value=0x12345678
  self.u.reg_write(UC_X86_REG_EAX,value);self.u.reg_write(UC_X86_REG_ECX,0xa5a5a5a5);self.u.reg_write(UC_X86_REG_EDX,0x5a5a5a5a)
 def release(self,kind,mask,count,has_lines,seed):
  h=self.begin(seed);self.mode='release';self.intercept((0x4aade0,1),(0x4a25f0,1));trace=[];after=bytearray(h)
  def pointers(base,n,step):
   values=[HEAP+0x1000+i*step if mask&(1<<(i%8)) else 0 for i in range(n)]
   self.put(self.addr(base),'<'+str(n)+'I',*values)
   return values
  if kind=='replay':
   values=pointers(0x588e80,8,32)
   self.allowed.update((self.addr(0x588e80)+i*4,4) for i in range(8));self.allowed.update((self.addr(a),4) for a in (0x588d3c,0x588d14))
   for a in (0x588d3c,0x588d14):self.put(self.addr(a),'<I',0x81234567)
   target,args=0x46c500,[];trace=[('free',v) for v in values if v]
  elif kind=='parts':
   values=pointers(0x590d7c,4,32);self.put(self.addr(0x590c64),'<i',count);self.put(self.addr(0x590c6c),'<I',LINE_ARRAY if has_lines else 0)
   self.allowed.update((self.addr(0x590d7c)+i*4,4) for i in range(4));self.allowed.update((self.addr(a),4) for a in (0x590c64,0x590c6c))
   lines=[HEAP+0x2000+i*32 if mask&(1<<i) else 0 for i in range(8)];put(h,LINE_ARRAY,'<8I',*lines);after=bytearray(h)
   trace=[('free',v) for v in values if v]
   if has_lines:
    for i in range(max(count,0)):
     if lines[i]:trace.append(('free',lines[i]));put(after,LINE_ARRAY+i*4,'<I',0)
    trace.append(('free',LINE_ARRAY))
   target,args=0x480870,[]
  else:
   values=pointers(0x6e0d6c,32,32);self.allowed.add((self.addr(0x5210f4),4));self.put(self.addr(0x5210f4),'<i',12345)
   target,args=0x4b7950,[(-1,0,32768,65537)[count if count in (0,1) else 2 if count==8 else 3]]
   trace=[('volume',v) for v in values if v]
  assert self.invoke_guarded(target,args)==bytes(after),'whole heap and untouched records'
  assert self.trace==trace,'complete provider order and pointer identities'
  if kind in ('replay','parts'):
   assert self.read(self.addr(0x588e80 if kind=='replay' else 0x590d7c),len(values)*4)==bytes(len(values)*4)
   assert self.u.reg_read(UC_X86_REG_EAX)==1
   for a in ((0x588d3c,0x588d14) if kind=='replay' else (0x590c64,0x590c6c)):assert self.read(self.addr(a),4)==bytes(4)
  else:assert struct.unpack('<i',self.read(self.addr(0x5210f4),4))[0]==(args[0] if 0<=args[0]<=65536 else 12345)
  return bytes(after),trace
 def flags(self,kind,seed):
  h=self.begin(seed);self.mode='release';before=random.Random(seed).randbytes(1024);expected=bytearray(before)
  self.u.mem_write(self.addr(0x531778),before);rank_guard=random.Random(seed+13).randbytes(32);self.u.mem_write(self.addr(0x531b78),rank_guard)
  for i in range(8):
   at=128*i+4;value=struct.unpack_from('<I',before,at)[0];struct.pack_into('<I',expected,at,value&~256 if kind=='ready' else value|0x400000);self.allowed.add((self.addr(0x531778)+at,4))
  assert self.invoke_guarded(0x409bc0 if kind=='ready' else 0x40afd0,[])==bytes(h)
  actual=self.read(self.addr(0x531778),1024);assert actual==bytes(expected) and self.read(self.addr(0x531b78),32)==rank_guard,'8 player flags and full adjacent rank guards'
  return actual
 def timing(self,kind,seed):
  h=self.begin(seed);self.mode='release';base=0x533538 if kind=='stage' else 0x533638;size=128 if kind=='stage' else 100
  before=random.Random(seed+1).randbytes(size);expected=bytearray(before);self.u.mem_write(self.addr(base),before)
  for i in range(16):
   expected[i]=i;struct.pack_into('<i',expected,32+i*4,0);self.allowed.add((self.addr(base)+i,1));self.allowed.add((self.addr(base)+32+i*4,4))
   if kind=='stage':
    expected[16+i]=i;expected[96+i]=expected[112+i]=0
    self.allowed.update((self.addr(base)+o+i,1) for o in (16,96,112))
  assert self.invoke_guarded(0x40cf00 if kind=='stage' else 0x40d100,[])==bytes(h)
  actual=self.read(self.addr(base),size);assert actual==bytes(expected),'all 16 primitive time rows and unchanged footer/padding'
  return actual

 def paired(self,seed):
  h=self.begin(seed);self.mode='paired'
  before=random.Random(seed).randbytes(100);expected=bytes(64)+before[64:]
  self.u.mem_write(self.addr(0x588cd4),before);pending=random.Random(seed+1).randbytes(64)
  self.u.mem_write(self.addr(0x588bb4),pending);self.region(self.addr(0x588cd4),64);self.region(self.addr(0x588bb4),32)
  assert self.invoke_guarded(0x46b710,[])==bytes(h)
  actual=self.read(self.addr(0x588cd4),100),self.read(self.addr(0x588bb4),64)
  assert actual==(expected,bytes(32)+pending[32:]),'16 levels, first 8 pending flags; scalar count and last 8 flags untouched'
  return actual
 def controls(self,enter,back,seed):
  h=self.begin(seed);self.mode='controls';self.mapping=[(seed*7919+i*8251)&65535 for i in range(8)]
  copy=random.Random(seed).randbytes(6*0x2f0);table=random.Random(seed+1).randbytes(len(copy))
  slots=random.Random(seed+2).randbytes(18);expected_slots=struct.pack('<8H',*self.mapping)+slots[16:] if enter else slots
  self.u.mem_write(self.addr(0x829448),copy);self.u.mem_write(self.addr(0x82a7c8),slots)
  h[LINE_ARRAY-HEAP:LINE_ARRAY-HEAP+len(table)]=table;after=bytearray(h)
  self.intercept((0x40c050,0),(0x40bbc0,1),(0x40bbe0,2),(0x40bbb0,0),(0x4fcb30,0))
  if enter:self.region(self.addr(0x82a7c8),16);self.region(self.addr(0x829448),len(copy))
  if not enter and back==0:after[LINE_ARRAY-HEAP:LINE_ARRAY-HEAP+len(copy)]=copy
  assert self.invoke_guarded(0x4fc9b0 if enter else 0x4fc970,[HEAP+0x800,back])==bytes(after)
  assert self.read(self.addr(0x82a7c8),18)==expected_slots
  actual=self.read(self.addr(0x829448),len(copy));assert actual==(table if enter else copy)
  expected_trace=([(0x40c050,())]+[(0x40bbc0,(i,)) for i in range(8)]+[(0x40bbb0,()),(0x4fcb30,())] if enter else [(0x40bbe0,(i,v)) for i,v in enumerate(struct.unpack('<8H',slots[:16]))]+[(0x40bbb0,())] if back==0 or enter else [])
  assert self.trace==expected_trace,(self.trace,expected_trace)
  return actual,bytes(after),self.trace
 def stage(self,seed):
  h=self.begin(seed);self.mode='stage';tables=[(0x5429c8,32),(0x542904,64),(0x5428c4,64),(0x542884,64),(0x542944,64),(0x542984,64)]
  scene=random.Random(seed).randbytes(16*36);expected=bytearray(scene)
  for base,size in tables:self.u.mem_write(self.addr(base),bytes([0xa3])*size);self.region(self.addr(base),size)
  self.u.mem_write(self.addr(0x542630),scene)
  for i in range(16):expected[i*36+4:i*36+12]=bytes(8);self.region(self.addr(0x542630)+i*36+4,8)
  entries=[(0x456ca0,0),(0x4287c0,1),(0x4667c0,1),(0x480900,1),(0x494b50,1),(0x45e5b0,1),(0x42b740,2),(0x42b5b0,2),(0x433840,1),(0x466630,1),(0x42b7e0,0),(0x405e00,0),(0x407e30,0),(0x407e80,0),(0x40b1c0,0),(0x457000,1)]
  self.intercept(*entries)
  assert self.invoke_guarded(0x456d90,[])==bytes(h) and self.u.reg_read(UC_X86_REG_EAX)==0
  actual=self.read(self.addr(0x542630),len(scene));assert actual==bytes(expected),'exactly 16 body/root pairs; all other scene members retained'
  for base,size in tables:assert self.read(self.addr(base),size)==bytes(size)
  expected_trace=[(a,() if n==0 else ((0,1) if n==2 else (0x190000,) if a in (0x433840,0x466630) else (0,) if a==0x457000 else (1,))) for a,n in entries]
  assert self.trace==expected_trace,(self.trace,expected_trace)
  return actual,self.trace

 def championship(self,round,seed):
  h=self.begin(seed);self.mode='championship';self.round=round;rng=random.Random(seed)
  before=bytearray(rng.randbytes(120));expected=bytearray(before);positions=list(range(8));rng.shuffle(positions)
  totals=[rng.choice((0,65536,131072,262144)) for i in range(8)];points=[rng.randrange(-1000,1000) for i in range(8)];earned=[rng.randrange(-1000,1000) for i in range(8)]
  wins=[rng.randrange(0,9) for i in range(8)]
  for i in range(8):struct.pack_into('<i',before,i*4,totals[i]);struct.pack_into('<i',before,56+i*4,points[i]);before[112+i]=wins[i]
  expected=bytearray(before);scores=(6,4,3,2,1,0,0,0)
  wins[positions[0]]+=1
  for i,car in enumerate(positions):totals[car]+=scores[i]*65536;expected[32+car*3+round]=scores[i]
  # Controlled sort starts from an order consistent with totals. The wrapper's
  # real tie handling still executes and is checked against a separate model.
  self.sorted=sorted(range(8),key=lambda i:-totals[i]);order=list(self.sorted);rank=[0]*8
  for i,car in enumerate(order):rank[car]=i
  for i in range(8):
   for j in range(i+1,8):
    car,other=order[i],order[j]
    if totals[car]!=totals[other]:break
    if wins[other]>wins[car] or wins[other]==wins[car] and other==0:
     rank[car],rank[other]=j,i;order[i],order[j]=other,car
  for i in range(8):struct.pack_into('<i',expected,i*4,totals[i]);struct.pack_into('<i',expected,56+i*4,points[i]+earned[i]);expected[112+i]=wins[i]
  expected[88:96]=bytes(order);expected[96:104]=bytes(rank)
  self.u.mem_write(self.addr(0x5335b8),bytes(before));self.region(self.addr(0x5335b8),120)
  put(h,HEAP+0x200,'<8b',*positions);put(h,HEAP+0x240,'<8i',*earned)
  self.put(self.addr(0x5112e0),'<d',65536.0)
  self.intercept((0x40ce40,1),(0x406950,0),(0x40d520,5))
  assert self.invoke_guarded(0x40ccd0,[HEAP+0x200,HEAP+0x240])==bytes(h)
  actual=self.read(self.addr(0x5335b8),120);assert actual==bytes(expected),'totals, points, round bytes, ties, all untouched fields'
  expected_trace=[entry for i in range(8) for entry in ((0x40ce40,(i,)),(0x406950,()))]+[(0x40d520,(self.addr(0x5335b8),self.addr(0x5335b8)+88,0,8,0))]
  assert self.trace==expected_trace
  return actual

def main():
 e=json.loads(Path(sys.argv[1]).read_text());a=Walks(ROOT/'cmr2bin/CMR2.exe');b=Walks(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e);n=0
 for args in release_cases():assert a.release(*args)==b.release(*args),args;n+=1
 for args in flag_cases():assert a.flags(*args)==b.flags(*args);n+=1
 for args in itertools.product(('stage','overall'),range(32)):assert a.timing(*args)==b.timing(*args);n+=1
 for seed in range(32):assert a.paired(seed)==b.paired(seed);n+=1
 for args in itertools.product((0,1),(-1,0,1,127),range(8)):assert a.controls(*args)==b.controls(*args);n+=1
 for seed in range(32):assert a.stage(seed)==b.stage(seed);n+=1
 for args in itertools.product(range(3),range(16)):assert a.championship(*args)==b.championship(*args);n+=1
 print(f'PASS: {n} complete table walks; independent free/volume/flag/time models, null/mixed slots, signed counts, full heap/global guards, pointer identities/provider order, stdcall/callee-saved ABI')
if __name__=='__main__':main()
