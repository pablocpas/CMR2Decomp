#!/usr/bin/env python3
"""Check the complete width wrapper, context forwarding and return contract.

Only Sprite_FillRect is controlled. Its existing clipping mismatch is outside
this API conversion and is documented, rather than silently changing behavior.
The wrapper executes its real fixed-point operations and preserves all inputs.
"""
import itertools
import json
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX
from differential_car_info_records import Records, put, short
from differential_stage_lighting import signed, mul
from differential_menu_list import ROOT, HEAP

RECT, COLOUR, GRAPHICS = HEAP+0x1000, HEAP+0x1100, HEAP+0x2000


class Rectangles(Records):
    def provider(self,address,nargs):
        assert address==0x4a5e40
        context,rect,colour,layer=self.args(nargs)
        self.trace.append((context,self.rect(rect),self.read(colour,4),layer))
        self.u.reg_write(UC_X86_REG_EAX,self.result)

    def run(self,rectangle,layer,scale,seed):
        h=self.begin(seed)
        self.intercept((0x4a5e40,4))
        put(h,RECT,'<4h',*rectangle)
        colour=bytes(((seed*17+3*i)&255 for i in range(4)))
        h[COLOUR-HEAP:COLOUR-HEAP+4]=colour
        self.result=(0x81000000+seed*97)&0xffffffff
        actual=self.invoke_guarded(0x475970,[scale,GRAPHICS+0x150,RECT,COLOUR,layer])
        assert actual==h,'heap/input guards'
        assert self.u.reg_read(UC_X86_REG_EAX)==self.result,'return forwarded'
        x,y,w,height=rectangle
        expected=[(GRAPHICS+0x150,(x,y,short(mul(signed(w<<16),scale)>>16),height),colour,layer)]
        assert self.trace==expected,('width/context model',rectangle,scale,self.trace,expected)
        return self.trace,self.result


def cases():
    rectangles=[(0,0,1,1),(-20,-30,50,70),(639,479,2,2),(0,0,640,480),
                (700,510,25,30),(32767,32767,32767,32767),(-32768,-32768,32767,32767),
                (-1,-1,1,1),(0,0,0,7),(10,20,-1,7),(0,0,7,-1),(10,20,7,0),
                (-20,30,19,10),(-20,30,20,10),(30,-20,10,19),(30,-20,10,20),
                (0,0,32767,32767),(1,1,10,10),(0,479,20,10),(639,0,10,20)]
    for i,args in enumerate(itertools.product(rectangles,(0,1,2,3,4,0xffffffff),
                                             (0,1,32768,65536,98304,131072,-65536,0x7fffffff))):
        yield (*args,i)


def main():
    entities=json.loads(Path(sys.argv[1]).read_text())
    original=Rectangles(ROOT/'cmr2bin/CMR2.exe')
    rebuilt=Rectangles(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities)
    count=0
    for args in cases():
        assert original.run(*args)==rebuilt.run(*args),('width rectangle',args)
        count+=1
    print(f'{count} width wrapper cases: independent signed fixed-point model, context/rect/colour/layer trace, return/input/global/ABI guards; Sprite_FillRect controlled')


if __name__=='__main__':main()
