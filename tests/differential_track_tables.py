#!/usr/bin/env python3
"""Original collision-block indexing and triangle readers, with integer oracles.

Whole heaps and a guarded 1 MB vertex allocation are compared. Includes signed
triangle offsets, unsigned indices 32768/65535 and overlapping output records.
No arithmetic/geometry providers are intercepted; only release registration is
controlled in the loader. Existing track_geometry covers quadtree traversal.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_ECX,UC_X86_REG_EDX
from differential_car_info_records import Records,put
from differential_menu_list import ROOT,HEAP

BLOCK,TRIANGLES,INDICES,OUTPUT=HEAP+0x1000,HEAP+0x5000,HEAP+0x6100,HEAP+0x6000
VERTICES=HEAP+0x20000
VERTEX_BYTES=0x100000
SLOTS=(0x591b30,0x591af8,0x591b00,0x591b04,0x591b08,0x591b0c,0x591b10,
       0x591b24,0x591b14,0x591afc,0x591af0,0x591b34,0x591b18,0x591b20,0x591b1c)
MODES=((0,1,2),(1,1,0),(65535,32768,0),(65535,65535,65535))


class Tables(Records):
    def __init__(self,*args):
        super().__init__(*args)
        self.u.mem_map(VERTICES,VERTEX_BYTES)

    def slot(self,address):
        if 0x591b00<=address<=0x591b14:
            return self.member(address,0x591b00)
        return self.addr(address)

    def provider(self,address,count):
        args=tuple(self.args(count))
        assert address==0x49c0a0 and args==(self.addr(0x4918c0),0)
        self.trace.append(('register',0x4918c0,0))
        self.u.reg_write(UC_X86_REG_EAX,0x12345678)
        self.u.reg_write(UC_X86_REG_ECX,0xa5a5a5a5)
        self.u.reg_write(UC_X86_REG_EDX,0x5a5a5a5a)

    def block(self,counts,vertices,triangles,lists,seed):
        h=self.begin(seed)
        self.intercept((0x49c0a0,2))
        put(h,BLOCK,'<2i5hH',-65536,131072,*counts,0xa55a)
        cursor=BLOCK+20
        pointers={0x591b30:BLOCK,0x591af8:BLOCK}
        for i,count in enumerate(counts):
            pointers[0x591b00+i*4]=cursor
            cursor+=count*8
        pointers[0x591b24]=cursor;put(h,cursor,'<HH',vertices,0xfffe);cursor+=4
        pointers[0x591b14]=cursor;cursor+=vertices*12
        pointers[0x591afc]=cursor;put(h,cursor,'<HH',triangles,0x7fff);cursor+=4
        pointers[0x591af0]=cursor;cursor+=triangles*8
        pointers[0x591b34]=cursor;put(h,cursor,'<i',lists);cursor+=4
        pointers[0x591b18]=cursor;cursor+=lists*2
        pointers[0x591b20]=cursor;put(h,cursor,'<hh',4,3);cursor+=4
        pointers[0x591b1c]=cursor-2
        for a in SLOTS:self.allowed.add((self.slot(a),4))
        for i in range(5):self.allowed.add((self.addr(0x591b38)+i*4,4))
        self.native_input=bytes(h)
        actual=self.invoke_guarded(0x490d50,[BLOCK])
        assert actual==bytes(h),'indexing must not modify serialized bytes'
        values=tuple(struct.unpack('<I',self.read(self.slot(a),4))[0] for a in SLOTS)
        assert values==tuple(pointers[a] for a in SLOTS),('independent table pointer model',counts,vertices,triangles,lists)
        node_counts=self.read(self.addr(0x591b38),20)
        assert node_counts==struct.pack('<5i',*counts),'signed node counts'
        assert self.trace==[('register',0x4918c0,0)]
        return actual,values,node_counts,tuple(self.trace)

    def reader(self,kind,tri,mode,alias,seed):
        h=self.begin(seed);rnd=random.Random(seed)
        v=bytearray([0xd3])*VERTEX_BYTES
        for i in set(mode+(0,1,2,32768,65535)):
            struct.pack_into('<3i',v,i*12,*(rnd.randrange(-2147483648,2147483648) for _ in range(3)))
        for i in range(-4,4):
            put(h,TRIANGLES+i*8,'<4H',*mode,(rnd.randrange(512)<<7)|rnd.randrange(128))
        put(h,INDICES,'<3H',*mode)
        self.put(self.slot(0x591b14),'<I',VERTICES)
        self.put(self.slot(0x591af0),'<I',TRIANGLES)
        out=OUTPUT if alias==0 else VERTICES+mode[0]*12 if alias==1 else INDICES if kind=='copy' else TRIANGLES+tri*8
        initial_h,initial_v=bytes(h),bytes(v)
        def read(address,fmt):
            memory,offset=(v,address-VERTICES) if address>=VERTICES else (h,address-HEAP)
            return struct.unpack_from(fmt,memory,offset)[0]
        def write(address,fmt,value):
            memory,offset=(v,address-VERTICES) if address>=VERTICES else (h,address-HEAP)
            struct.pack_into(fmt,memory,offset,value)
        source=INDICES if kind=='copy' else TRIANGLES+tri*8
        if kind=='face':
            for i in range(3):write(out+i*2,'<H',read(source+i*2,'<H'))
            write(out+8,'<H',read(source+6,'<H')&127)
            write(out+6,'<H',0)
            address,args=0x491790,[tri&65535,out,out+2,out+4,out+6,out+8]
        else:
            for i in range(3):
                for k in range(3):
                    index=read(source+i*2,'<H')
                    write(out+i*12+k*4,'<i',read(VERTICES+index*12+k*4,'<i'))
            address,args=(0x490e00,[out,tri&65535]) if kind=='get' else (0x4917f0,[out,INDICES,HEAP+0x6900])
        expected_h,expected_v=bytes(h),bytes(v)
        self.heap=bytearray(initial_h);self.u.mem_write(VERTICES,initial_v)
        self.native_input=initial_h+initial_v
        actual=self.invoke_guarded(address,args)
        vertices_actual=self.read(VERTICES,VERTEX_BYTES)
        assert actual==expected_h and vertices_actual==expected_v,('independent reader model/guards',kind,tri,mode,alias,seed)
        if kind=='get':assert self.u.reg_read(UC_X86_REG_EAX)==1
        return actual,vertices_actual


def main():
    entities=json.loads(Path(sys.argv[1]).read_text())
    original=Tables(ROOT/'cmr2bin/CMR2.exe')
    rebuilt=Tables(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities)
    count=0
    for args in itertools.product(((0,0,0,0,0),(1,2,3,4,5),(5,0,1,0,2),(-1,2,0,0,0)),(0,1,17),(0,1,9),(0,7),(0,31)):
        assert original.block(*args)==rebuilt.block(*args),('serialized index',args);count+=1
    blocks=count
    for kind in ('get','copy','face'):
        for rest in itertools.product((0,) if kind=='copy' else (-4,-1,0,1,3),MODES,(0,1,2),(5,23)):
            args=(kind,*rest)
            assert original.reader(*args)==rebuilt.reader(*args),('reader',args);count+=1
    print(f'PASS: {blocks} complete serialized-index scenarios, {count-blocks} triangle-reader scenarios; independent pointer/copy models, full heap and 1 MB guards, unsigned high indices and aliases, ABI')


if __name__=='__main__':main()
