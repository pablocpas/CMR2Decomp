#!/usr/bin/env python3
"""Signed TM tuning and both split banks against independent arithmetic models.

Route length, split count, RNG and final sorting are controlled leaves. Execute
the complete blending/randomization/interpolation bodies with guarded memory.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX, UC_X86_REG_FPCW
from differential_car_info_records import Records, put, trunc
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import add, mul, signed

PROFILE, SPREAD, OUTPUT = HEAP+0x1000, HEAP+0x3000, HEAP+0x4000
BLENDS = ((0,0),(65536,65536),(0,65536),(16384,49152),(-16384,81920))


def cases():
    for seed,blend in itertools.product(range(4),BLENDS):
        yield 'blend',(seed,*blend,0)
        for splits in (-1,0,1,4,9):
            yield 'interpolate',(seed,*blend,splits)
    for seed,splits,count in itertools.product(range(4),(0,4,9),(0,1,8,16)):
        yield 'randomize',(seed,splits,count,0)


def inputs(seed):
    rng=random.Random(71003+seed)
    heap=bytearray(rng.randbytes(0x10000))
    edges=(-128,-127,-17,-1,0,1,23,126,127)
    tuning=[tuple(edges[(seed+driver*3+field*2)%len(edges)] for field in range(5)) for driver in range(16)]
    for driver,row in enumerate(tuning):put(heap,PROFILE+driver*5,'<5b',*row)
    banks=[]
    for bank in range(2):
        words=[signed((-1 if (i+seed+bank)%3==0 else 1)*(i*17013+bank*97003+seed*333)) for i in range(160)]
        if seed==3:words[::17]=[signed(0x7fffffff+i*977) for i in range(len(words[::17]))]
        put(heap,PROFILE+80+bank*640,'<160i',*words);banks.append(words)
    weights=[rng.randrange(-16384,81921) for _ in range(40)]
    put(heap,SPREAD,'<40i',*weights)
    return heap,tuning,banks,weights


def blend_model(tuning,start,end):
    span=10000*65536
    before,after=trunc((span//8)<<16,span),trunc((span-span*7//8)<<16,span)
    middle=signed(65536-after-before)
    half=trunc(add(65536-start,65536-end),2)
    output=[]
    for base,_,_,b0,b1 in tuning:
        b0,b1=b0*65536,b1*65536
        terms=(mul(before,mul(65536-start,b0)),mul(before,mul(start,b1)),
               mul(after,mul(65536-end,b0)),mul(after,mul(b1,end)),
               mul(middle,mul(half,b0)),mul(middle,mul(b1,65536-half)),base*65536)
        output.append(signed(sum(terms)))
    return output


def interpolate_model(banks,weights,start,end,splits):
    result=[]
    for split in range(1,max(splits,0)+1):
        t=trunc(add(add(start,mul(end-start,weights[split])),add(start,mul(end-start,weights[split-1]))),2)
        for driver in range(16):
            a,b=(bank[split*16+driver] for bank in banks)
            terms=(mul(mul(weights[10+split],65536-start),a),mul(mul(weights[10+split],start),b),
                   mul(mul(weights[30+split],65536-end),a),mul(mul(weights[30+split],end),b),
                   mul(65536-t,mul(weights[20+split],a)),mul(t,mul(weights[20+split],b)))
            result.append(signed(sum(terms)))
    return result


class TimeProfile(Records):
    def provider(self,address,nargs):
        value=0
        if address==0x421420:
            self.trace.append(('route',));value=10000
        elif address==0x4583C0:
            self.trace.append(('splits',));value=self.splits
        elif address==0x4C6676:
            value=self.draws[len([e for e in self.trace if e[0]=='rand'])]
            self.trace.append(('rand',value))
        elif address==0x40D520:
            times,order,direction,count,initialize=self.args(nargs)
            assert (times,order,direction,count,initialize)==(OUTPUT,self.addr(0x541F98)+0x14+self.splits*16,0,self.count,1)
            self.trace.append(('sort',tuple(struct.unpack('<16i',self.read(times,64))),self.splits,count))
        else:raise AssertionError(hex(address))
        self.u.reg_write(UC_X86_REG_EAX,value & 0xFFFFFFFF)
        self.u.reg_write(UC_X86_REG_ECX,0xA5A5A5A5);self.u.reg_write(UC_X86_REG_EDX,0x5A5A5A5A)

    def run_case(self,kind,args):
        seed,a,b,splits=args
        self.begin(71003+seed);h,tuning,banks,weights=inputs(seed)
        self.heap=h;self.splits=splits;self.count=0
        self.u.reg_write(UC_X86_REG_FPCW,0x007f)
        self.put(self.addr(0x54241C),'<I',PROFILE)
        expected=h[:]
        if kind=='randomize':
            self.splits,self.count=a,b
            self.put(self.addr(0x541F98),'<i',self.count)
            self.draws=[(0,1,16383,16384,32766,32767)[(seed+i)%6] for i in range(16)]
            initial=[signed(0x7ffffff0+i*1234567) for i in range(16)];put(h,OUTPUT,'<16i',*initial)
            expected=h[:]
            # high = -randomSubtract; the affine span is -low-high.
            values=[add(value,add(mul(trunc((draw*65536)<<16,0x7fff0000),(-row[2]+row[1])*65536),row[2]*65536))
                    for value,row,draw in zip(initial,tuning,self.draws)]
            trace=[('splits',)]+[('rand',d) for d in self.draws]+[('sort',tuple(values),self.splits,self.count)]
            target=0x456110
        else:
            span=10000*65536;put(h,SPREAD+160,'<4i',span//8,span*7//8,a,b);expected=h[:]
            if kind=='blend':values=blend_model(tuning,a,b);trace=[('route',)];target=0x455F00
            else:values=interpolate_model(banks,weights,a,b,splits);trace=[('splits',)];target=0x4564D0
        if values:put(expected,OUTPUT,'<'+str(len(values))+'i',*values)
        self.initial=bytes(h)
        self.intercept((0x421420,0),(0x4583C0,0),(0x40D520,5),(0x4C6676,0))
        actual=self.invoke_guarded(target,[OUTPUT] if kind=='randomize' else [SPREAD,OUTPUT])
        assert actual==expected,(kind,args,'complete heap/model')
        assert self.trace==trace,(kind,args,self.trace,trace)
        return actual,self.trace


def main():
    entities=json.loads(Path(sys.argv[1]).read_text())
    images=(TimeProfile(ROOT/'cmr2bin/CMR2.exe'),TimeProfile(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities))
    count=0
    for kind,args in cases():
        assert images[0].run_case(kind,args)==images[1].run_case(kind,args),(kind,args);count+=1
    print(f'PASS: {count} TM profile cases; signed tuning, endpoint/extrapolated blends, both time banks, '
          'zero/negative/nine splits, random endpoints, sort arguments, full file/workspace/heap guards and ABI')


if __name__=='__main__':main()
