#!/usr/bin/env python3
"""Original/rebuilt session fields, ghost flags and replay state/pose recording.

Network read/push, colour uploads, car lookup and pose encoding are controlled
leaves. Complete changed bodies execute; models check all guarded memory,
provider traces, pointer arguments and x86 stack cleanup.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_ESP
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT, HEAP, STACK, STOP

SESSION = 0x5a0068
STREAM, NODE, MESH, CAR = HEAP+0x1000, HEAP+0x2000, HEAP+0x3000, HEAP+0x4000

class Records(Lighting):
    def __init__(self,path,entities=None):
        super().__init__(path,entities)
        self.callbacks={}
        for address,n in ((0x4a0d60,0),(0x4a14e0,0),(0x4ab960,2),(0x4ab8f0,2),(0x42b5f0,1),(0x46c4e0,2)):
            self.callbacks[self.addr(address)]=(n*4,lambda a=address,n=n:self.leaf(a,n))
        self.callbacks[self.addr(0x4aad40)] = (0, self.direct_play)
        self.callbacks[STOP+0x100] = (24, self.enumerate_leaf)
    def direct_play(self):
        self.trace.append(('provider', []))
        self.u.reg_write(UC_X86_REG_EAX, HEAP+0x9000 if self.provider_present else 0)
    def enumerate_leaf(self):
        args = self.args(6)
        assert args == (HEAP+0x9000, self.addr(SESSION), 0, self.addr(0x4a12b0), 0, self.enum_flags)
        self.trace.append(('enumerate', [self.enum_flags]))
        self.u.reg_write(UC_X86_REG_EAX, self.enum_status & 0xffffffff)
    def enumerate_sessions(self, target, busy, present, status, password):
        self.prepare(status)
        self.provider_present, self.enum_status = present, status
        self.enum_flags = 0x51 if target == 0x4a12d0 else 0x20
        self.u.mem_write(self.addr(SESSION), bytes([0xa5])*0x50)
        self.u.mem_write(self.addr(0x59fa20), bytes([0xa5])*1600)
        self.put(self.addr(0x5a1814), '<I', busy)
        self.put(HEAP+0x9000, '<I', HEAP+0x9100)
        self.put(HEAP+0x9134, '<I', STOP+0x100)
        self.put(self.addr(0x511cd8), '<4I', 1, 0xdeadbeef, 3, 0xffffffff)
        before = self.read(HEAP, 0x10000)
        answer = self.call(target, [password] if target == 0x4a12d0 else [])
        expected = bytearray([0xa5]*0x50 if busy else bytes(0x50))
        trace = []
        if not busy:
            struct.pack_into('<I', expected, 0, 0x50)
            struct.pack_into('<4I', expected, 0x18, 1, 0xdeadbeef, 3, 0xffffffff)
            struct.pack_into('<I', expected, 0x30, self.addr(0x5a00b8))
            if target == 0x4a12d0: struct.pack_into('<I', expected, 0x34, password)
            trace = [('provider', [])] + ([('enumerate', [self.enum_flags])] if present else [])
            assert self.read(self.addr(0x59fa20), 1600) == bytes(1600)
            assert self.read(self.addr(0x5a0060), 4) == bytes(4)
            assert self.read(self.addr(0x5a1814), 4) == bytes(4)
            assert self.read(self.addr(0x5a01bc), 1) == bytes(1)
        else: assert self.read(self.addr(0x59fa20), 1600) == bytes([0xa5])*1600
        value = {0: 1, 0x8877015e: -1, 0x88770118: -2}.get(status, 0) if not busy and present else 0
        assert answer == value & 0xffffffff
        assert self.read(self.addr(SESSION), 0x50) == expected and self.trace == trace and self.read(HEAP, 0x10000) == before
        # Globals are independently relocated in rebuilt; normalize the name address.
        if not busy: struct.pack_into('<I', expected, 0x30, 0x5a00b8)
        return answer, expected, trace
    def leaf(self,address,n):
        args=list(self.args(n))
        if address==0x4ab960:args[1]&=255
        if address==0x4ab8f0:args[1]=self.read(args[1],4)
        if address==0x46c4e0:
            args[1]&=255
            self.u.mem_write(args[0],bytes(range(16)))
        self.trace.append((address,args))
        value=CAR if address==0x42b5f0 else 1
        self.u.reg_write(UC_X86_REG_EAX,value)
        self.u.reg_write(UC_X86_REG_ECX,0xa5a5a5a5);self.u.reg_write(UC_X86_REG_EDX,0x5a5a5a5a)
    def prepare(self,seed):
        self.reset();self.u.mem_write(HEAP,random.Random(seed).randbytes(0x10000))
    def call(self,address,args):
        self.invoke(address,args)
        assert self.u.reg_read(UC_X86_REG_ESP)==STACK+0xff04+4*len(args)
        return self.u.reg_read(UC_X86_REG_EAX)
    def session(self,target,index,value):
        self.prepare(index+value);self.u.mem_write(self.addr(SESSION),bytes([0xa5])*0x50)
        expected=bytearray(self.read(self.addr(SESSION),0x50));args=[];trace=[(0x4a0d60,[])]
        if target in (0x4a16c0,0x4a1680):
            off=0x40+4*(index if index<3 else 3);args=[index]
            if target==0x4a16c0:args+=[value&0xffffffff];struct.pack_into('<I',expected,off,value&0xffffffff)
        elif target in (0x4a1510,0x4a1530):
            off=0x30 if target==0x4a1510 else 0x34;args=[HEAP+0x6000];struct.pack_into('<I',expected,off,args[0])
        elif target==0x4a1550:args=[value&0xffffffff];struct.pack_into('<I',expected,0x28,value&0xffffffff)
        else:
            args=[index];flags=struct.unpack_from('<I',expected,4)[0];struct.pack_into('<I',expected,4,flags&~32 if index else flags|32)
        before=self.read(HEAP,0x10000);answer=self.call(target,args)
        if target!=0x4a1680:trace.append((0x4a14e0,[]))
        else:assert answer==struct.unpack_from('<I',expected,off)[0]
        assert self.trace==trace and self.read(self.addr(SESSION),0x50)==expected and self.read(HEAP,0x10000)==before
        return answer if target in (0x4a1680,0x4a1550) else 0,expected,self.trace
    def ghost(self,kind,missing,alpha,check):
        self.prepare(alpha+kind);self.put(NODE+0x178,'<i',kind);self.put(NODE+12,'<I',0 if missing else MESH)
        expected=bytearray(self.read(HEAP,0x10000));trace=[]
        if kind==0 and not missing:
            at=MESH-HEAP+0x30;flags=struct.unpack_from('<I',expected,at)[0];struct.pack_into('<I',expected,at,flags|0x40008)
            trace=[(0x4ab960,[MESH,alpha&255])]
            if check:trace.append((0x4ab8f0,[MESH,b'\x80\x80\x80\0']))
        self.call(0x465ec0,[NODE,alpha&0xffffffff,check]);assert self.trace==trace and self.read(HEAP,0x10000)==expected
        return expected,trace
    def reset_stream(self,playing):
        self.prepare(playing);self.put(STREAM+4,'<i',playing)
        expected=bytearray(self.read(HEAP,0x10000))
        if playing:
            for off in (4,8,20):struct.pack_into('<i',expected,STREAM-HEAP+off,0)
        answer=self.call(0x46d2a0,[STREAM]);assert answer==bool(playing) and self.read(HEAP,0x10000)==expected
        return answer,expected
    def record_pose(self,kind,recording,steps,phase,count,capacity):
        self.prepare(count+phase);self.u.mem_write(self.addr(0x588d40),bytes(64));self.put(self.addr(0x588d40),'<I',HEAP+0x8000);self.put(HEAP+0x8000,'<I',STREAM)
        self.put(STREAM+12,'<i',recording);self.put(STREAM+28,'<i',kind);self.put(STREAM+32,'<B',1)
        self.put(STREAM+0xf8,'<B',phase);self.put(STREAM+0xfe,'<h',capacity);self.put(STREAM+0x100,'<h',1)
        self.put(STREAM+0x104,'<I',HEAP+0x7000);self.put(HEAP+0x7002,'<h',count);self.put(STREAM+0x40,'<I',HEAP+0x6000)
        self.put(CAR+0xb43,'<B',steps);expected=bytearray(self.read(HEAP,0x10000));trace=[]
        if recording and kind==2:
            trace.append((0x42b5f0,[1]))
            if steps:
                if phase==0:
                    if count<capacity:
                        pointer=HEAP+0x6000+16*(capacity+count);trace.append((0x46c4e0,[pointer,1]));expected[pointer-HEAP:pointer-HEAP+16]=bytes(range(16));struct.pack_into('<h',expected,0x7002,count+1)
                    else:
                        struct.pack_into('<i',expected,0x100c,0);struct.pack_into('<i',expected,0x1010,0);struct.pack_into('<i',expected,0x1018,0);struct.pack_into('<h',expected,0x1100,1)
                expected[0x10f8]=phase+1 if phase<2 else 0
        self.call(0x46d510,[]);assert self.trace==trace and self.read(HEAP,0x10000)==expected,(kind,recording,steps,phase,count,capacity)
        return expected,trace

def main():
    entities=json.loads(Path(sys.argv[1]).read_text());exe=Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe'
    a,b=Records(ROOT/'cmr2bin/CMR2.exe'),Records(exe,entities);n=0
    for target,index,value in itertools.product((0x4a16c0,0x4a1680,0x4a1510,0x4a1530,0x4a1550,0x4a1570),(0,1,2,3,255),(-2147483648,-1,0,1,2147483647)):
        assert a.session(target,index,value)==b.session(target,index,value);n+=1
    for case in itertools.product((0,1,3),(0,1),(-1,0,100,255,256),(0,1)):
        assert a.ghost(*case)==b.ghost(*case);n+=1
    for playing in (-1,0,1):assert a.reset_stream(playing)==b.reset_stream(playing);n+=1
    for case in itertools.product((0,1,2),(0,1),(0,1,2),(0,1,2),(0,1,2),(2,3)):
        assert a.record_pose(*case)==b.record_pose(*case);n+=1
    for case in itertools.product((0x4a13b0, 0x4a12d0), (0,1), (0,1), (0,0x8877015e,0x88770118,0x8877005a,0x88770082,0x887700aa,0x88770140,0x12345678), (0,HEAP+0x6000)):
        assert a.enumerate_sessions(*case)==b.enumerate_sessions(*case);n+=1
    print(f'PASS {n} original/rebuilt session, ghost and replay runtime cases; full memory models, traces, sample destinations and ABI')
    return 0
if __name__=='__main__':raise SystemExit(main())
