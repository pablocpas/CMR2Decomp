#!/usr/bin/env python3
"""Real Send/SetLocal bodies with checked COM pointer arguments and status models."""
import itertools,json,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX
from differential_car_info_records import Records,put
from differential_menu_list import ROOT,HEAP
OBJECT,TABLE,STUB,DATA=HEAP+0x9000,HEAP+0xa000,HEAP+0xf000,HEAP+0x1000
STATUSES=(0,1,0x88770082,0x88770096,0x88770168,0x8877010e,0x88770816,0x80004005,0x7fffffff)

def cases():return itertools.product(('send','local'),(0,1),(-1,0,1,2),(0,1,16,257),STATUSES)
class Transport(Records):
 def provider(self,address,n):
  if address==0x4aad40:value=OBJECT if self.present else 0
  else:
   args=tuple(self.args(n))
   expected=(OBJECT,0x87654321,0xfedcba98,int(self.guaranteed==1),DATA,self.length) if self.kind=='send' else (OBJECT,0x87654321,DATA,self.length,2)
   assert args==expected,('COM payload/id/flags argument widths',self.kind,args,expected)
   self.trace.append((self.kind,self.read(DATA,self.length),self.guaranteed==1 if self.kind=='send' else 2));value=self.status
  self.u.reg_write(UC_X86_REG_EAX,value);self.u.reg_write(UC_X86_REG_ECX,0xa5a5a5a5);self.u.reg_write(UC_X86_REG_EDX,0x5a5a5a5a)
 def run(self,kind,present,guaranteed,length,status):
  h=self.begin(37);self.kind,self.present,self.guaranteed,self.length,self.status=kind,present,guaranteed,length,status
  self.intercept((0x4aad40,0));self.callbacks[STUB]=(24 if kind=='send' else 20,lambda:self.provider(STUB,6 if kind=='send' else 5))
  put(h,OBJECT,'<I',TABLE);put(h,TABLE+(26 if kind=='send' else 29)*4,'<I',STUB)
  self.put(self.addr(0x66521c),'<I',OBJECT if present else 0);self.put(self.addr(0x5a1ea0),'<I',0x87654321)
  args=[0xfedcba98,guaranteed,DATA,length] if kind=='send' else [DATA,length]
  assert self.invoke_guarded(0x4a1c50 if kind=='send' else 0x4a1cb0,args)==bytes(h),'whole message/object/dispatch heap guards'
  result=self.u.reg_read(UC_X86_REG_EAX)&255;assert result==int(present and status==0),'HRESULT model/char result'
  expected=[(kind,bytes(h[DATA-HEAP:DATA-HEAP+length]),guaranteed==1 if kind=='send' else 2)] if present else []
  assert self.trace==expected;return result,self.trace

def main():
 e=json.loads(Path(sys.argv[1]).read_text());a=Transport(ROOT/'cmr2bin/CMR2.exe');b=Transport(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e);n=0
 for args in cases():assert a.run(*args)==b.run(*args);n+=1
 print(f'PASS: {n} complete Send/SetLocal scenarios; COM payload/id/size/flags argument models, HRESULT/null paths, full heap guards, stdcall/callee-saved ABI')
if __name__=='__main__':main()
