#!/usr/bin/env python3
"""Actual box construction/movement and sphere/box bodies with guarded memory.

No geometry or arithmetic helpers are intercepted. The split movement has an
independent integer model, including aliased geometry and nullable pointers.
Sphere fixtures cover both axes, four corners, zero direction and outside cases.
Construction covers the vertical-axis fallback, cached boxes with unreadable
inputs, preserved secondary points and an independent footprint model.
"""
import itertools
import json
import math
from pathlib import Path
import struct
import sys

from differential_car_info_records import Records, put
from differential_car_collision_access import Contacts, A, ENTRY, OWNER, BOX_A as CONTACT_BOX_A, BOX_B as CONTACT_BOX_B
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import mul, signed
from unicorn.x86_const import UC_X86_REG_EAX

BOX_A,BOX_B=HEAP+0x1000,HEAP+0x1200
CENTRE_A,CENTRE_B=HEAP+0x3000,HEAP+0x3100
VERTICES_A,VERTICES_B=HEAP+0x4000,HEAP+0x4100
DIRECTION,FACTORS=HEAP+0x6000,HEAP+0x6100
GLOBALS=((0x5914a8,12),(0x5914b8,12),(0x5915e8,12))


class Boxes(Records):
    def build_car(self,axes,built,seed):
        h=self.begin(seed)
        position=(seed*131-65536,seed*97+32768,-seed*73)
        put(h,CENTRE_A,'<3i',*position)
        put(h,DIRECTION,'<9i',*axes)
        corners=[]
        for i in range(8):
            corner=(position[0]+(65536 if i&1 else -98304),
                    position[1]+(131072 if i&4 else -32768),
                    position[2]+(49152 if i&2 else -65536))
            corners.append(corner)
            put(h,VERTICES_A+12*i,'<3i',*corner)
        put(h,BOX_A+0x28,'<i',built)
        self.put(self.addr(0x6e0ef4),'<4096H',
                 *(math.isqrt((8+16*i)*65536) for i in range(4096)))
        # A cached box must return before reading any of the input geometry.
        args=[BOX_A,DIRECTION,CENTRE_A,VERTICES_A] if not built else [BOX_A,0xdead0000,0xdead1000,0xdead2000]
        memory=self.invoke_guarded(0x486c30,args)
        if built:
            assert memory==bytes(h),'cached box touched memory'
            return memory
        before=BOX_A-HEAP
        expected=bytearray(h)
        # Only the published box fields and its first four points may change.
        # In particular the secondary flag and previous-frame points survive.
        for start,end in ((0,0x2c),(0x30,0x60),(0x90,0x98)):
            expected[before+start:before+end]=memory[before+start:before+end]
        assert memory==bytes(expected),'box construction guards/input geometry'
        width,length,top,bottom=struct.unpack_from('<4i',memory,before)
        axis_a=struct.unpack_from('<3i',memory,before+0x10)
        axis_b=struct.unpack_from('<3i',memory,before+0x1c)
        assert axis_a[1]==axis_b[1]==0 and axis_b==(axis_a[2],0,signed(-axis_a[0]))
        def projection(corner,axis):
            return signed(sum(mul(signed(a-b),v) for a,b,v in zip(corner,position,axis)))
        assert width==max(0,*(projection(c,axis_a) for c in corners))
        assert length==max(0,*(projection(c,axis_b) for c in corners))
        assert (top,bottom)==(max(c[1] for c in corners),min(c[1] for c in corners))
        assert struct.unpack_from('<i',memory,before+0x28)==(1,)
        assert struct.unpack_from('<2I',memory,before+0x90)==(VERTICES_A,CENTRE_A)
        return memory

    def footprint(self,axes,extents,seed):
        h=self.begin(seed)
        position=(seed*17-32768,seed*31+65536,-seed*43)
        put(h,CENTRE_A,'<3i',*position)
        put(h,BOX_A,'<4i',*extents,seed*101-65536,0x12345678)
        put(h,BOX_A+0x10,'<6i',*axes)
        expected=bytearray(h)
        a=tuple(mul(v,extents[0]) for v in axes[:3])
        b=tuple(mul(v,extents[1]) for v in axes[3:])
        for i,(sign_a,sign_b) in enumerate(((1,-1),(1,1),(-1,-1),(-1,1))):
            point=[signed(sign_a*x+sign_b*y+p) for x,y,p in zip(a,b,position)]
            point[1]=seed*101-65536
            put(expected,BOX_A+0x30+12*i,'<3i',*point)
        memory=self.invoke_guarded(0x486fc0,[BOX_A,CENTRE_A])
        assert memory==bytes(expected),'independent footprint model and guards'
        return memory

    def getter(self,index,seed):
        h=self.begin(seed)
        assert self.invoke_guarded(0x48ca40,[index])==bytes(h)
        assert self.u.reg_read(UC_X86_REG_EAX)==self.addr(0x590ed0)+index*0x98

    def provider(self,address,nargs):
        assert address==0x48df50 and tuple(self.args(nargs))==(HEAP+0x8000,)
        self.trace.append(('damping',HEAP+0x8000))
        from unicorn.x86_const import UC_X86_REG_ECX,UC_X86_REG_EDX
        self.u.reg_write(UC_X86_REG_EAX,0x12345678)
        self.u.reg_write(UC_X86_REG_ECX,0xa5a5a5a5)
        self.u.reg_write(UC_X86_REG_EDX,0x5a5a5a5a)

    def damping(self,x,z,radius,seed):
        h=self.fixture(seed)
        put(h,CENTRE_A,'<3i',0,0,0)
        self.put(self.addr(0x591498),'<3i',x,seed*100,z)
        self.put(self.addr(0x591490),'<i',radius)
        self.intercept((0x48df50,1))
        result=self.result(0x487e50,[BOX_A,HEAP+0x8000],h,True)
        expected=(not(abs(x)>65536 and abs(z)>32768) and abs(x)<=65536+radius and abs(z)<=32768+radius)
        assert result[0]==bytes(h) and len(self.trace)==int(expected),('damping overlap model',x,z,radius)
        return result,tuple(self.trace)

    def fixture(self,seed,nulls=0,alias=0):
        h=self.begin(seed)
        self.centres=(CENTRE_A,CENTRE_A if alias else CENTRE_B)
        self.vertices=(VERTICES_A,VERTICES_A if alias else VERTICES_B)
        for k,box in enumerate((BOX_A,BOX_B)):
            put(h,box,'<2i',65536,32768)
            put(h,box+8,'<2i',131072,-131072)
            put(h,box+0x10,'<6i',65536,0,0,0,0,65536)
            for i in range(8):
                put(h,box+0x30+12*i,'<3i',(1 if i%4<2 else -1)*65536,
                    131072 if i>=4 else -131072,(1 if i%2 else -1)*32768)
                put(h,self.vertices[k]+12*i,'<3i',i*111+seed,-i*123-seed,i*251+seed)
            put(h,self.centres[k],'<3i',k*8192,0,-k*4096)
            put(h,box+0x90,'<2I',0 if nulls&(1<<(2*k)) else self.vertices[k],
                0 if nulls&(2<<(2*k)) else self.centres[k])
        for j,(address,size) in enumerate(GLOBALS):
            self.put(self.addr(address),'<3i',1234+j*193,-2345-j*147,3456+j*357)
            self.allowed.update((self.addr(address)+i,4) for i in range(0,size,4))
        self.put(self.addr(0x6e0ef4),'<4096H',*(math.isqrt((8+16*i)*65536) for i in range(4096)))
        return h

    def result(self,address,args,heap,void=False):
        self.native_input=bytes(heap)
        actual=self.invoke_guarded(address,args)
        globals_=tuple(self.read(self.addr(a),n) for a,n in GLOBALS)
        return actual,globals_,None if void else self.u.reg_read(UC_X86_REG_EAX)&255

    def movement(self,direction,amount,scale,nulls,alias,seed):
        h=self.fixture(seed,nulls,alias)
        put(h,DIRECTION,'<3i',*direction)
        expected=bytearray(h)
        globals_=[list(struct.unpack('<3i',self.read(self.addr(a),n))) for a,n in GLOBALS]
        globals_[2]=list(direction)
        if amount>0:
            part=mul(amount,scale)
            for k,enabled,weight,global_index in ((1,scale!=0,part,1),(0,scale!=65536,signed(part-amount),0)):
                if not enabled:continue
                displacement=[mul(value,weight) for value in direction]
                globals_[global_index]=[signed(a+b) for a,b in zip(globals_[global_index],displacement)]
                box=(BOX_A,BOX_B)[k]
                array,centre=struct.unpack_from('<2I',expected,box-HEAP+0x90)
                if not (array and centre):continue
                # The original updates centre, eight corners, then four footprint
                # points; re-read the expected heap so shared storage accumulates.
                for address in [centre,*[array+12*i for i in range(8)],*[box+0x30+12*i for i in range(4)]]:
                    old=struct.unpack_from('<3i',expected,address-HEAP)
                    put(expected,address,'<3i',*(signed(a+b) for a,b in zip(old,displacement)))
        result=self.result(0x4894b0,[BOX_A,BOX_B,DIRECTION,amount&0xffffffff,scale&0xffffffff],h,True)
        assert result[0]==bytes(expected),('independent split heap',direction,amount,scale,nulls,alias,seed)
        assert result[1]==tuple(struct.pack('<3i',*g) for g in globals_),('independent split globals',direction,amount,scale,nulls,alias,seed)
        return result

    def sphere(self,x,z,scale,no_array,rotation,seed):
        h=self.fixture(seed)
        put(h,CENTRE_A,'<3i',0,0,0)
        if no_array:put(h,BOX_A+0x90,'<I',0)
        if rotation:
            put(h,BOX_A+0x10,'<6i',0,0,65536,-65536,0,0)
            for i in range(8):
                xx,yy,zz=struct.unpack_from('<3i',h,BOX_A-HEAP+0x30+12*i)
                put(h,BOX_A+0x30+12*i,'<3i',-zz,yy,xx)
        self.put(self.addr(0x591498),'<3i',x,seed*100,z)
        self.put(self.addr(0x591490),'<i',32768)
        put(h,FACTORS,'<2i',0x12345678,-0x1234567)
        result=self.result(0x489b20,[BOX_A,FACTORS,FACTORS+4,scale&0xffffffff],h)
        assert result[2] in (0,1,2,3)
        if result[2]==0:
            assert result[0]==bytes(h),('outside/zero direction heap',x,z,rotation)
        else:
            # The kernel publishes the same movement for centre, every external
            # corner and only the four local footprint points. Upper points and
            # padding stay untouched, as does geometry with a null array pointer.
            delta=struct.unpack('<3i',result[1][0])
            expected=bytearray(h)
            if not no_array:
                for address in [CENTRE_A,*[VERTICES_A+12*i for i in range(8)],*[BOX_A+0x30+12*i for i in range(4)]]:
                    old=struct.unpack_from('<3i',expected,address-HEAP)
                    put(expected,address,'<3i',*(signed(a+b) for a,b in zip(old,delta)))
            expected[FACTORS-HEAP:FACTORS-HEAP+8]=result[0][FACTORS-HEAP:FACTORS-HEAP+8]
            assert result[0]==bytes(expected),('sphere translation/guards',x,z,scale,no_array,rotation)
        return result


class Turned(Contacts):
    def provider(self,address,nargs):
        if address==0x488640:
            args=tuple(self.args(nargs))
            assert args[:2]==(CONTACT_BOX_A,CONTACT_BOX_B) and args[3] in (0,65536)
            self.trace.append(('overlap',self.read(args[2],12),args[3]))
            self.u.reg_write(UC_X86_REG_EAX,self.overlap)
            # The shared fixture clobbers caller-saved registers for every leaf.
            from unicorn.x86_const import UC_X86_REG_ECX,UC_X86_REG_EDX
            self.u.reg_write(UC_X86_REG_ECX,0xa5a5a5a5)
            self.u.reg_write(UC_X86_REG_EDX,0x5a5a5a5a)
        else:super().provider(address,nargs)

    def turned(self,hit,counts,nulls,solid,seed):
        h=self.fixture(seed,counts,nulls)
        self.overlap=hit
        put(h,OWNER+0x10,'<I',0x1000 if solid else 0)
        result=self.outcome(0x487f60,[A,ENTRY,CONTACT_BOX_A,CONTACT_BOX_B])
        assert result[3]==hit
        if not hit:assert result[0]==bytes(h) and len(result[2])==1
        frames=[call for call in result[2] if call[0]=='frame']
        assert len(frames)==int(hit and not solid and 0 in counts)
        return result


def main():
    entities=json.loads(Path(sys.argv[1]).read_text())
    original=Boxes(ROOT/'cmr2bin/CMR2.exe')
    rebuilt=Boxes(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities)
    count=0
    build_axes=((65536,0,0,0,65536,0,0,0,65536),
                (0,0,65536,0,65536,0,-65536,0,0),
                (32768,32768,49152,0,65536,0,0,0,65536),
                (0,65536,0,65536,0,0,0,0,65536),
                (0,-65536,0,0,0,-65536,65536,0,0),
                (0,0,0,0,0,0,0,0,0))
    threshold_axes=tuple((32768,y,0,0,0,65536,0,65536,0)
                         for y in (0xfd70,0xfd71,-0xfd70,-0xfd71))
    construction_count=0
    for args in itertools.product(build_axes+threshold_axes,(0,1,-1),(0,19)):
        assert original.build_car(*args)==rebuilt.build_car(*args),('build car box',args)
        construction_count+=1
    footprint_count=0
    footprint_axes=((65536,0,0,0,0,65536),(0,0,65536,-65536,0,0),
                    (32768,16384,-49152,-49152,8192,-32768),(0,0,0,0,0,0))
    for args in itertools.product(footprint_axes,((0,0),(65536,32768),(-32768,98304),(0x7fffffff,-1)),(0,19)):
        assert original.footprint(*args)==rebuilt.footprint(*args),('footprint',args)
        footprint_count+=1
    for index in range(8):
        original.getter(index,index);rebuilt.getter(index,index)
    directions=((0,0,0),(65536,0,0),(0,-65536,0),(0,0,65536),(-32768,8192,49152))
    for args in itertools.product(directions,(-1,0,1,65536,196609),(-32768,0,32768,65536,98304),(0,1,2,3,4,8,15),(0,1),(0,17)):
        assert original.movement(*args)==rebuilt.movement(*args),('split',args);count+=1
    split_count=count
    for args in itertools.product((-196608,-98304,-65536,-1,0,1,65536,98304,196608),
                                 (-131072,-65536,-32768,0,32768,65536,131072),
                                 (-32768,0,32768,65536),(0,1),(0,1),(3,)):
        assert original.sphere(*args)==rebuilt.sphere(*args),('sphere',args);count+=1
    damping_count=0
    for args in itertools.product((-98305,-98304,-65536,0,65536,98304,98305),
                                  (-65537,-65536,-32768,0,32768,65536,65537),(0,1,32768),(13,)):
        assert original.damping(*args)==rebuilt.damping(*args),('damping',args);damping_count+=1
    left,right=Turned(ROOT/'cmr2bin/CMR2.exe'),Turned(ROOT/'build/CMR2.exe',entities)
    turned_count=0
    for args in itertools.product((0,1),((0,1),(1,0),(1,1),(2,3)),(0,1,2,3),(0,1),(7,19)):
        assert left.turned(*args)==right.turned(*args),('turned',args);turned_count+=1
    print(f'PASS: {construction_count} car box constructions/cache guards, {footprint_count} independent footprints, 8 box getters, {damping_count} damping overlap bounds, {turned_count} turned object contacts, {split_count} independent split cases, {count-split_count} complete sphere cases; whole guarded heap/globals/ABI, real geometry and fixed-point helpers')


if __name__=='__main__':main()
