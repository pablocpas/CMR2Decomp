#!/usr/bin/env python3
"""Compare sector-edge contact responses for every original surface type.

Usage: differential_surface_collision.py entities.json [rebuilt.exe]
Contact detection supplies valid face vertices. The complete response executes
with each image's own coefficient tables; impulses/effects are recorded leaves.
"""
import itertools
import json
import math
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP


class Collision(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for a,n in [(0x490570,0),(0x490640,0),(0x48e580,1),
                    (0x486c30,4),(0x490720,0),(0x48c870,4),
                    (0x48f470,3),(0x499750,6)]:
            self.callbacks[self.addr(a)] = (n*4,lambda a=a,n=n:self.provider(a,n))

    def provider(self,a,n):
        args=self.args(n)
        if a in [0x490570,0x490640,0x48e580]:
            self.u.reg_write(UC_X86_REG_EAX,1)
        elif a==0x48f470:
            self.trace.append(('impulse',self.read(args[0],12),self.read(args[1],12),args[2]))
        elif a==0x48c870:
            self.trace.append(('translate',args[0]&255,args[1]&255,self.read(args[2],12),args[3]))
        elif a==0x499750:
            self.trace.append(('debris',args[0],self.read(args[1],12),args[2],self.read(args[3],36),args[4:]))

    def run(self,surface,back,axis,velocity):
        self.u.mem_write(self.base,bytes(self.memory))
        self.u.mem_write(HEAP,bytes(0x10000));self.u.mem_write(STACK,bytes(0x10000))
        self.put(self.addr(0x6e0ef4),'<4096H',*(int(math.sqrt((8+16*i)/65536)*65536) for i in range(4096)))
        car,face,edge=HEAP+0x1000,HEAP+0x3000,HEAP+0x4000
        for a,v in [(0x5918dc,car),(0x591984,face),(0x59190c,edge),(0x59192c,back),(0x519c8c,0x10000)]:
            self.put(self.addr(a),'<I',v)
        direction=(65536,0,0) if axis==0 else (0,0,65536)
        self.put(self.addr(0x591910),'<3i',*direction)
        for a,v in [(0x591934,4),(0x591935,4),(0x591944,2),(0x591945,2)]:
            self.put(self.addr(a),'<B',v)
        self.put(self.addr(0x591988),'<4B',2,3,0,0)
        self.put(self.addr(0x59198c),'<4B',0,1,0,0)
        self.put(self.addr(0x5919a4),'<4i',0x8000,0x4000,-0x8000,-0x4000)
        self.put(edge+0x2d,'<B',1)
        self.put(car+0x2d0,'<3i',100*65536,0,200*65536)
        self.put(car+0x408,'<3i',velocity if axis==0 else 65536,0,velocity if axis==1 else 65536)
        for j in range(8):self.put(car+0x270+j*12,'<3i',100*65536+j*256,0,200*65536+j*256)
        for j in range(4):self.put(face+0x30+j*12,'<3i',100*65536+j*256,0,200*65536+j*256)
        sp=STACK+0xff00;self.put(sp,'<3I',STOP,surface,0)
        self.u.reg_write(UC_X86_REG_ESP,sp);self.trace=[]
        self.u.emu_start(self.addr(0x48fb80),STOP,count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP)==STOP
        return self.read(HEAP,0x10000),self.trace,self.u.reg_read(UC_X86_REG_EAX)


def main():
    e=json.loads(Path(sys.argv[1]).read_text())
    a=Collision(ROOT/'cmr2bin/CMR2.exe')
    b=Collision(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e)
    cases=0
    for args in itertools.product(range(26),[0,1],[0,1],[-0x40000,0x40000]):
        aa,bb=a.run(*args),b.run(*args)
        if aa!=bb:
            print('FAIL surface response',args,'original',aa[1],'rebuilt',bb[1]);return 1
        cases+=1
    print(f'{cases} surface contact cases: identical impulses, translated car/corners and effects')
    return 0


if __name__=='__main__':sys.exit(main())
