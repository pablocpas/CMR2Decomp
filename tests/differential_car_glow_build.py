#!/usr/bin/env python3
"""Complete original/rebuilt CIN glow creation and nearest-vertex association.

The pool allocator, glow setter, damage/part getters and exhaust setters are
real. CIN, scene-light creation and texture setup are controlled asset leaves.
Check an independent nearest-vertex model, heap, globals, provider order and ABI.
"""
import itertools
import json
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_car_info_records import Records, CAR, put
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import signed

SETS, POOL, PROFILE = HEAP+0x2000, HEAP+0x5000, HEAP+0x6000
VERTICES, BODY, NODES = HEAP+0x7000, HEAP+0xA000, HEAP+0xB000
TEXTURES, REAR, PARENT, SENTINEL = HEAP+0xC000, HEAP+0xD000, HEAP+0xD800, HEAP+0xE000
REGIONS = ((0x547F80,32),(0x547FC0,32),(0x547D00,640),
           (0x543580,64),(0x5431C0,64),(0x547FE0,12),(0x547FA0,12),
           (0x547FEC,4),(0x547FF0,4),(0x547FF4,4),(0x547CE0,32),(0x6A2BC8,4))


def build_cases():
    return itertools.product((0,7),(0,-1),(-1,0,1,20),(0,3),(0,1),(0,24))


class BuildGlows(Records):
    def provider(self,address,nargs):
        args=tuple(self.args(nargs));value=0
        if address==0x457E10:
            assert args==(CAR,0);value=PROFILE;self.trace.append(('CIN',))
        elif address==0x411060:
            value=PARENT;self.trace.append(('parent',))
        elif address==0x4B2F80:
            assert args[:4]==(0,0x140000,0x140000,0x140000) and args[6]==PARENT
            assert self.read(args[4],12)==bytes(12)
            assert struct.unpack('<4H',self.read(args[5],8))==(0,0,0xFF1D,0)
            value=REAR+self.created*0x200;self.created+=1;self.trace.append(('scene',value))
        elif address==0x4A3E20:
            assert args[0] in range(TEXTURES,TEXTURES+10*64,64) and args[1]==1
            self.trace.append(('texture',*args))
        else:
            raise AssertionError(hex(address))
        self.u.reg_write(UC_X86_REG_EAX,value)
        self.u.reg_write(UC_X86_REG_ECX,0xA5A5A5A5);self.u.reg_write(UC_X86_REG_EDX,0x5A5A5A5A)

    def snapshot(self):
        return b''.join(self.read(self.addr(a),size) for a,size in REGIONS)

    def build(self,car,enabled,count,part_count,pattern,capacity,seed):
        h=self.begin(seed);self.created=0;self.trace=[]
        self.intercept((0x457E10,2),(0x411060,0),(0x4B2F80,7),(0x4A3E20,2))
        for a,size in REGIONS:
            if a in (0x547F80,0x547FC0,0x547D00,0x543580,0x5431C0,0x547FEC,0x547FF0):
                self.u.mem_write(self.addr(a),struct.pack('<'+'I'*(size//4),*([SENTINEL]*(size//4))))
            else:self.u.mem_write(self.addr(a),bytes([0x71])*size)
        put(h,CAR+0xB1A,'<b',car);put(h,CAR+0x720,'<I',BODY)
        put(h,PROFILE,'<i',count)
        part=SETS+car*0x4D0;put(h,part+0x45C,'<i',part_count)
        self.put(self.addr(0x588990)+4*car,'<I',part)
        self.put(self.addr(0x588970)+4*car,'<i',enabled)
        present=0
        for slot in range(4):
            node=NODES+slot*0x200 if (slot+pattern)%3 else 0
            if node:present|=1<<slot
            self.put(self.addr(0x590B7C)+4*(slot*8+car),'<I',node)
        for slot in range(5):
            self.put(self.addr(0x547FAC)+4*slot,'<I',TEXTURES+slot*64)
            self.put(self.addr(0x547FF8)+4*slot,'<I',TEXTURES+(slot+5)*64)
        counts=((-1,0,3),(3,3,4))[pattern]
        vertices=[]
        for mesh,n in enumerate(counts):
            put(h,part+0x420+mesh*4,'<i',n)
            put(h,part+0x78+mesh*4,'<I',VERTICES+mesh*0x300)
            row=[]
            for vertex in range(4):
                # Mesh 1 duplicates mesh 0: a tie must keep the first match.
                pos=(vertex*65536,0,0) if mesh<2 else (0,vertex*65536,0)
                put(h,VERTICES+mesh*0x300+vertex*32,'<3i',*pos);row.append(pos)
            vertices.append(row)
        for i in range(20):
            point=PROFILE+4+i*40
            pos=(100*65536,0,0) if i%5==0 else (i%4*65536,0,0)
            put(h,point,'<6i',*pos,65536,0,0)
            put(h,point+24,'<2i',32768+i,49152+i*13)
            put(h,point+32,'<BBbB',9 if i in (2,18) else i%9,i%5,(-1,0,1,3)[i%4],0xA7)
            put(h,point+36,'<2h',-123,-456)
        occupied=0
        for i in range(24):
            taken=i<capacity and pattern and i%3==0
            put(h,POOL+i*92,'<i',77 if taken else 0);occupied+=bool(taken)
        self.put(self.addr(0x6A2A98),'<I',POOL);self.put(self.addr(0x6A2BCC),'<i',capacity);self.put(self.addr(0x6A2BC8),'<i',occupied)
        # Exact allowed global widths; snapshots guard all neighbouring slots.
        for a in (0x547F80,0x547FC0):self.allowed.add((self.addr(a)+car*4,4))
        for i in range(max(count,0)):self.allowed.add((self.addr(0x547D00)+(car*20+i)*4,4))
        for a in (0x543580,0x5431C0):
            for side in range(2):self.allowed.add((self.addr(a)+(car*2+side)*4,4))
        for a in (0x547FE0,0x547FA0):
            for off in (0,4,8):self.allowed.add((self.addr(a)+off,4))
        for a in (0x547FEC,0x547FF0,0x547FF4,0x6A2BC8):self.allowed.add((self.addr(a),4))
        for i in range(8):self.allowed.add((self.addr(0x547CE0)+i*4,4))
        before=bytes(h);before_globals=self.snapshot()
        self.native_input=before+before_globals+struct.pack('<iiI',enabled,capacity,present)
        actual=self.invoke_guarded(0x463FE0,[CAR])
        expected=bytearray(before)
        outputs={a:bytearray(before_globals[sum(n for _,n in REGIONS[:i]):sum(n for _,n in REGIONS[:i+1])]) for i,(a,size) in enumerate(REGIONS)}
        struct.pack_into('<I',outputs[0x547F80],4*car,PROFILE);struct.pack_into('<I',outputs[0x547FC0],4*car,PROFILE+4)
        trace=[]
        if car==0:
            trace=[('parent',),('scene',REAR),('parent',),('scene',REAR+0x200)]
            outputs[0x547FE0][:]=struct.pack('<3i',0x1E0000,0,0);outputs[0x547FA0][:]=struct.pack('<3i',0xF0000,0,0)
            outputs[0x547FEC][:]=struct.pack('<I',REAR);outputs[0x547FF0][:]=struct.pack('<I',REAR+0x200)
        trace.append(('CIN',));free=[i for i in range(capacity) if not (pattern and i%3==0)];side=0
        for i in range(max(count,0)):
            point=PROFILE+4+i*40;pos=struct.unpack_from('<3i',before,point-HEAP);slot=i%5
            trace += [('texture',TEXTURES+slot*64,1),('texture',TEXTURES+(slot+5)*64,1)]
            glow=POOL+free.pop(0)*92 if free else 0
            struct.pack_into('<I',outputs[0x547D00],(car*20+i)*4,glow)
            if glow:
                part_slot=(-1,0,1,3)[i%4]
                node=NODES+part_slot*0x200 if enabled and part_slot!=-1 and present&(1<<part_slot) else BODY
                put(expected,glow,'<7i',2,*pos,65536,0,0)
                put(expected,glow+0x34,'<3i',32768+i,32768+i,49152+i*13)
                put(expected,glow+0x40,'<i',65536)
                put(expected,glow+0x48,'<2I',TEXTURES+slot*64,TEXTURES+(slot+5)*64)
                put(expected,glow+0x50,'<2B',1,255 if i in (2,18) else 0)
                put(expected,glow+0x54,'<I',node);occupied+=1
            if i in (2,18):
                struct.pack_into('<I',outputs[0x543580],(car*2+side)*4,point)
                struct.pack_into('<I',outputs[0x5431C0],(car*2+side)*4,glow);side+=1
            if enabled:
                best=0x3E80000;closest=None
                for mesh in range(part_count):
                    for vertex in range(max(0,counts[mesh])):
                        delta=[signed(a-b) for a,b in zip(pos,vertices[mesh][vertex])]
                        distance=signed(sum(v*v for v in delta)>>16)
                        if distance<best:best=distance;closest=(mesh,vertex)
                if closest:put(expected,point+36,'<2h',*closest)
        outputs[0x547FF4][:]=struct.pack('<i',-65536);outputs[0x547CE0][:]=bytes(32);outputs[0x6A2BC8][:]=struct.pack('<i',occupied)
        want_globals=b''.join(bytes(outputs[a]) for a,_ in REGIONS)
        assert actual==expected,'complete glow/point heap model and guards'
        assert self.snapshot()==want_globals,'complete globals / neighbour guards'
        assert self.trace==trace,'asset order / arguments'
        return actual,want_globals,self.trace


def main():
    entities=json.loads(Path(sys.argv[1]).read_text())
    runs=(BuildGlows(ROOT/'cmr2bin/CMR2.exe'),BuildGlows(ROOT/'build/CMR2.exe',entities))
    cases=0
    for i,args in enumerate(build_cases()):
        result=[run.build(*args,30000+i) for run in runs]
        assert result[0]==result[1],(args,'original / rebuilt');cases+=1
    print(f'{cases} complete car-glow builds: 0 differences; real allocation/getters/exhaust links, nearest-vertex ties/cutoff, negative counts, full heap/globals, provider order and ABI')


if __name__=='__main__':main()
