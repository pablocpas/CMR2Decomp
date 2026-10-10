#!/usr/bin/env python3
"""Paired wheel surface builder: real tables/math, independent whole-heap model.

The original copies odd corner/wheel records to their preceding even record.
Guard bytes and the upper four corner records must remain untouched. No leaf
provider is intercepted. All 48 surfaces, seven drag groups and compression
branches are covered, as are the exact noise-smoothing boundaries.
"""
import itertools
import json
from pathlib import Path
import struct
import sys

from differential_car_info_records import Records, put, trunc
from differential_car_initial_state import CAR, get
from differential_menu_list import ROOT
from differential_stage_lighting import mul, signed

TABLES = {'grip':(0x51dfb8,96,'i'), 'grip2':(0x51e138,96,'i'),
          'soft':(0x51e378,96,'i'), 'drag':(0x51de5c,63,'i'),
          'drag_index':(0x51e918,48,'B'), 'effect':(0x51e7f8,96,'B'),
          'noise':(0x51e858,48,'B'), 'f0':(0x51e678,48,'i'),
          'f1':(0x51e738,48,'i'), 'f2':(0x51e2b8,48,'i'),
          'f3':(0x51e4f8,48,'i'), 'f4':(0x51e5b8,48,'i')}
COMPRESSIONS = ((0,0),(1,0),(65535,65536),(65536,1),(98304,98304),(0,65535))


def level(tables, ids):
    total = sum(tables['noise'][ids[i]]*2 for i in (3,1))
    return mul((total & 0xfffffffc) << 14, 0x28f)


def model(h,tables):
    h=bytearray(h)
    ids=[get(h,0xaae+2*i,'<h') for i in range(4)]
    for i in (3,1):
        id=ids[i]
        a,b=tables['grip'][id*2:id*2+2]
        c,d=tables['grip2'][id*2:id*2+2]
        comp=get(h,0x890+4*i)
        if comp:
            soft=mul(min(comp,65536),tables['soft'][id*2])
            aa,cc=signed(a+soft),signed(c+soft)
            bb=trunc(mul(a,b)<<16,aa)
            dd=trunc(mul(c,d)<<16,cc)
        else:aa,bb,cc,dd=a,b,c,d
        comp=get(h,0x8a0+4*i)
        if comp:
            soft=mul(min(comp,65536),tables['soft'][id*2+1])
            bb,dd=signed(bb+soft),signed(dd+soft)
        words=[aa,bb,cc,dd]+[tables['f'+str(j)][id] for j in range(5)]
        for j in (i,i-1):put(h,CAR+0x80+36*j,'<9i',*words)
        drag=mul(0x51e,tables['drag'][tables['drag_index'][id]+get(h,0xb29,'<B')*9])
        for j in (i,i-1):
            put(h,CAR+0x1a0+12*j,'<2B',*tables['effect'][2*id:2*id+2])
            put(h,CAR+0x1a0+12*j+4,'<2i',drag,0)
    value=level(tables,ids)
    before=get(h,0xa74)
    delta=signed(value-before)
    put(h,CAR+0xa78,'<i',value)
    put(h,CAR+0xa74,'<i',value if abs(delta)<=0x3333 else signed(before+(0x3333 if delta>0 else -0x3333)))
    return h


class Contacts(Records):
    def __init__(self,path,entities=None):
        super().__init__(path,entities)
        self.reset()
        self.tables={name:struct.unpack('<'+str(count)+fmt,self.read(self.addr(addr),count*struct.calcsize(fmt)))
                     for name,(addr,count,fmt) in TABLES.items()}

    def run(self,surface,compression,drag,previous_delta,seed):
        h=self.begin(seed)
        ids=[(surface+i*7)%48 for i in range(8)]
        put(h,CAR+0xaae,'<8h',*ids)
        put(h,CAR+0x890,'<4i',*compression)
        put(h,CAR+0x8a0,'<4i',*reversed(compression))
        put(h,CAR+0xb29,'<B',drag)
        put(h,CAR+0xa74,'<i',signed(level(self.tables,ids)+previous_delta))
        self.native_input=bytes(h)
        expected=model(h,self.tables)
        out=self.invoke_guarded(0x4786b0,[CAR,(seed*31337)&0xffffffff])
        assert out==expected,('paired surfaces/model/guards',surface,compression,drag,previous_delta)
        assert not self.trace
        return out


def cases():
    for seed,(surface,comp,drag,previous) in enumerate(itertools.product(
        range(48),COMPRESSIONS,range(7),(-65536,-0x3334,-0x3333,0,0x3333,0x3334,65536))):
        compression=tuple(comp[i%2] for i in range(4))
        yield surface,compression,drag,previous,seed


def main():
    entities=json.loads(Path(sys.argv[1]).read_text())
    original=Contacts(ROOT/'cmr2bin/CMR2.exe')
    rebuilt=Contacts(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities)
    assert original.tables==rebuilt.tables,'surface tables'
    count=0
    for case in cases():
        assert original.run(*case)==rebuilt.run(*case),('original/rebuilt',case)
        count+=1
    print(f'{count} paired surface cases: real helpers/tables; independent model, complete heap/ABI and noise thresholds match')


if __name__=='__main__':main()
