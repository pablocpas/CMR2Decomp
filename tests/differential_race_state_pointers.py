#!/usr/bin/env python3
"""Complete race-state clients with controlled external scene/network leaves.

Original/rebuilt inputs include poisoned machine records, independent state
queries, full guarded heaps, global write widths, provider arguments and ABI.
Provider cleanup comes from the original callee's RET, never from rebuilt code.
"""
import bisect,csv,itertools,json,random,struct,sys
from pathlib import Path
from capstone import Cs,CS_ARCH_X86,CS_MODE_32,CS_OP_IMM
from unicorn import UC_HOOK_MEM_WRITE
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX,UC_X86_REG_ESP,UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT,HEAP,STACK
MACHINE,RECORDS,CAR=HEAP+0x1000,HEAP+0x2000,HEAP+0x4000
TARGETS={0x41b060:0,0x401000:2,0x4010a0:2,0x401150:2,0x501350:2,0x41bf80:2,0x41c0e0:2,0x41e5c0:2,0x41d2b0:2,0x41d7a0:2,0x41db10:2,0x41e350:2,0x41e6b0:3,0x41f3a0:0,0x423970:2,0x41b390:0}
REGS=(UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP)
META={int(r['addr'],16):r for r in csv.DictReader((ROOT/'scripts/functions.tsv').open(),delimiter='\t')}
DECODER=Cs(CS_ARCH_X86,CS_MODE_32);DECODER.detail=True

def insns(image,a):return list(DECODER.disasm(image.memory[a-image.base:a-image.base+int(META[a]['size'])],a))

class States(Lighting):
 def __init__(self,path,entities=None):
  super().__init__(path,entities);self.callbacks={};self.active=False;self.u.hook_add(UC_HOOK_MEM_WRITE,self.capture)
  original=Lighting(ROOT/'cmr2bin/CMR2.exe')
  callees=set()
  for target in TARGETS:
   for i in insns(original,target):
    if i.mnemonic=='call' and i.operands[0].type==CS_OP_IMM:callees.add(i.operands[0].imm)
  self.providers={}
  for a in callees:
   if a not in META:raise AssertionError(('unidentified controlled leaf',hex(a)))
   returns=[int(i.op_str or '0',0) for i in insns(original,a) if i.mnemonic=='ret']
   assert len(set(returns))<=1,(hex(a),returns)
   self.providers[a]=(returns[0] if returns else 0,META[a]['name'])
  self.reverse={self.addr(a):a for a in META if entities is None or hex(a) in entities}
  if entities:
   self.reverse.update({v[0]:int(k,16) for k,v in entities.items()})
  else:
   self.reverse.update({int(k,16):int(k,16) for k in json.loads((ROOT/'CMR2PROGRESS/entities.json').read_text())})
  self.data_addresses=sorted(a for a,original in self.reverse.items() if original>=0x511000)
 def norm(self,v):
  if v in self.reverse:return ('entity',self.reverse[v])
  if STACK<=v<STACK+65536:return ('stack',)
  if self.data_addresses and self.data_addresses[0]<=v<self.base+self.size:
   base=self.data_addresses[bisect.bisect_right(self.data_addresses,v)-1]
   return ('member',self.reverse[base],v-base)
  return v
 def capture(self,u,access,address,size,value,data):
  if self.active and self.base<=address<self.base+self.size:
   base=self.data_addresses[bisect.bisect_right(self.data_addresses,address)-1]
   self.writes.append(((self.reverse[base],address-base),size,self.norm(value) if size==4 else value))
 def leaf(self,a,n,name):
  args=list(self.args(n//4));value=self.values.get(name,0)
  if name=='StageUI_GetRaceResultTable':value=MACHINE
  elif name=='Car_Get':value=CAR+(args[0]&255)*0xc24
  elif name=='View_IsModeAvailable':value=self.available
  if name=='CGame::InitializeCallbackStateMachine':args[1]&=255
  if name in ('CGame::InitializeCallbackStateRecord','CGame::PromoteCallbackEntryByRule'):
   args[1]&=255;args[2]&=255
  self.trace.append((name,tuple(self.norm(v) for v in args)))
  self.u.reg_write(UC_X86_REG_EAX,value&0xffffffff)
  self.u.reg_write(UC_X86_REG_ECX,0xa5a5a5a5);self.u.reg_write(UC_X86_REG_EDX,0x5a5a5a5a)
 def run(self,target,seed,count,state,index,network,event):
  self.active=False;self.reset();self.callbacks={};self.writes=[]
  h=bytearray(random.Random(seed).randbytes(65536))
  struct.pack_into('<B3x3I',h,MACHINE-HEAP,count,RECORDS,0,0)
  for i in range(2):struct.pack_into('<Ii',h,RECORDS-HEAP+i*8,((seed*0x193700)&0xffffff00)|((state+i*3)&255),seed*11-i)
  self.u.mem_write(HEAP,bytes(h))
  self.u.mem_write(STACK,bytes([seed&255])*65536)
  self.values={'RallyDataState':count,'CGameInfo::GetConfiguredPlayerCount':count,'CGameInfo::GetConfiguredGameMode':10 if network else 0,'CGameInfo::GetGameModeOptionBit19':network,'CMain::GetFrameDelta':64+seed,'StageTiming_GetCheckpointField1A':event,'GameInfo_GetFrontendSessionFlag':event,'View_IsModeAvailable':seed%2}
  self.available=seed%2
  for a,(n,name) in self.providers.items():
   if a==target:continue
   self.callbacks[self.addr(a)]=(n,lambda a=a,n=n,name=name:self.leaf(a,n,name))
  # Cold runtime globals are independent of the original/rebuilt image layout.
  self.put(self.addr(0x82a938),'<i',seed%2)
  self.put(self.addr(0x82b0a4),'<I',seed)
  self.put(self.addr(0x538100),'<B',network)
  self.put(self.addr(0x537ef4),'<B',event)
  self.put(self.addr(0x537ef5),'<B',0)
  self.put(self.addr(0x537ef8),'<i',0)
  args=[MACHINE,index] if TARGETS[target]==2 else []
  if target==0x423970:args=[index,2]
  if target==0x41e6b0:args=[MACHINE,index,event]
  saved={r:0x12345678+i*0x11111111 for i,r in enumerate(REGS)}
  for r,v in saved.items():self.u.reg_write(r,v)
  self.active=True
  try:self.invoke(target,args)
  except Exception as exc:raise AssertionError((hex(target),seed,count,state,index,network,event,self.trace[-5:])) from exc
  finally:self.active=False
  assert self.u.reg_read(UC_X86_REG_ESP)==STACK+0xff04+4*len(args),'stdcall cleanup'
  assert all(self.u.reg_read(r)==v for r,v in saved.items()),'callee-saved registers'
  actual=self.read(HEAP,65536)
  assert actual==bytes(h),'machine and guarded input heap remain untouched'
  result=self.norm(self.u.reg_read(UC_X86_REG_EAX)) if target in (0x41f3a0,0x423970,0x41b390) else None
  if target==0x41f3a0:assert result==int(count>1 and state==10)
  if target==0x41b060 and not event:
   setup=[args for name,args in self.trace if name=='CGame::InitializeCallbackStateMachine']
   assert setup==[(('entity',0x537dd0),2 if count>1 else 1,('member',0x537dd0,16),('entity',0x5190b0),('entity',0x519120))],setup
  if target==0x423970:assert result==int(((state+index*3)&255)!=10 and self.available!=0)
  return self.trace,self.writes,result

def main():
 entities=json.loads(Path(sys.argv[1]).read_text());a=States(ROOT/'cmr2bin/CMR2.exe');b=States(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities);total=0
 for target in TARGETS:
  for seed,count,state,index,network,event in itertools.product(range(2),(1,2),(7,8,10,13,255),(0,1),(0,1),(0,1)):
   if index>=count:continue
   x=a.run(target,seed,count,state,index,network,event);y=b.run(target,seed,count,state,index,network,event)
   assert x==y,(hex(target),seed,count,state,index,network,event,x,y)
   total+=1
 print(f'PASS: {total} complete race-state client scenarios; independent queried-state models, guarded records/heaps, global write widths, controlled provider identities/order/arguments, stdcall and preserved registers')
if __name__=='__main__':main()
