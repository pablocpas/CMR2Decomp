#!/usr/bin/env python3
"""Ride-height setup and complete wheel-spring integration against integer models.

Only the cheat query is controlled. The integrator and inverse rotation are
real; verify full poisoned heap, input matrices/history, global writes and ABI.
"""
import itertools
import json
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_car_info_records import Records, put, trunc
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import mul, signed

CAR, WORLD = HEAP + 0x1000, HEAP + 0x3000
SETTINGS = (-65536, -0x6666, -1, 0, 1, 0x7FFF, 0x8000, 0xFFFF, 65536, 0x18000, 0x1FF00)
MASSES = (-0x80000000, -9, -8, -1, 0, 1, 7, 8, 9, 65536, 0x7FFFFFFF)
DISTANCES = (-0x4CCD, -0x4CCC, -0x4CCB, -1, 0, 1, 0x4CCB, 0x4CCC, 0x4CCD)
BASES = (((65536,0,0),(0,65536,0),(0,0,65536)),
         ((0,65536,0),(-65536,0,0),(0,0,65536)),
         ((65536,0,0),(0,46341,46341),(0,-46341,46341)))


def get(h, offset):
    return struct.unpack_from('<i', h, CAR-HEAP+offset)[0]


def vector(h, offset):
    return struct.unpack_from('<3i', h, CAR-HEAP+offset)


def setter_model(h, setting, cheat):
    out = bytearray(h)
    setting = -0x6666 if cheat else setting
    def store(offset, value):
        put(out, CAR+offset, '<i', signed(value))
    store(0x9B8, mul(setting,0x6666)+0x9999)
    store(0x9BC, mul(setting,0x3333)+0x6666)
    store(0x8D8, trunc(get(h,0x75C),8))
    store(0x9C0, trunc(65536 << 16, mul(setting,-0x1999)+0x3333))
    store(0x9C4, trunc(65536 << 16, mul(setting,-0x1AAAA)+0x50000))
    for wheel in range(4):
        store(0x9E8+wheel*4,0x4CCC)
        store(0x9F8+wheel*4,mul(setting,0x5999)+0x4000)
        store(0x988+wheel*4,mul(0x1999,65536-setting))
        store(0x978+wheel*4,mul(setting,0x8000)+0x8000)
    store(0xA08,-0x1999)
    return out


def spring_model(h, step, scale, matrix):
    out = bytearray(h)
    up = struct.unpack_from('<3i',h,matrix-HEAP+16)
    for wheel in range(4):
        velocity, offset = get(h,0x9C8+wheel*4), get(h,0x9D8+wheel*4)
        distance = abs(signed(get(h,0x8DC+wheel*4)-vector(h,0x270+wheel*12)[1]))
        if get(h,0xB74) and distance<0x4CCC and any(vector(h,0x330+wheel*12)):
            current, previous, older = (vector(h,p+wheel*12) for p in (0x270,0x300,0x330))
            second = [signed(a-2*b+c) for a,b,c in zip(current,previous,older)]
            impulse = mul(signed(sum(mul(a,b) for a,b in zip(up,second))),mul(0x9C28,scale))
            if impulse>0:
                velocity = signed(velocity-impulse)
        restoring = mul(get(h,0x9E8+wheel*4),offset)
        damping = mul(get(h,0x9F8+wheel*4),velocity)
        velocity = signed(velocity+mul(step,signed(-signed(restoring+damping))))
        offset = signed(offset+mul(step,velocity))
        if offset < -0x3333:
            offset,velocity = -0x3333,max(velocity,0)
        put(out,CAR+0x9C8+wheel*4,'<i',velocity)
        put(out,CAR+0x9D8+wheel*4,'<i',offset)
    return out


class Springs(Records):
    def provider(self, address, nargs):
        assert address==0x4063F0 and tuple(self.args(nargs))==(6,)
        self.trace.append(('cheat',6))
        self.u.reg_write(UC_X86_REG_EAX,self.cheat)
        self.u.reg_write(UC_X86_REG_ECX,0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX,0x5A5A5A5A)

    def setter(self, setting, cheat, mass, seed):
        h = self.begin(seed)
        self.cheat = cheat
        self.intercept((0x4063F0,1))
        put(h,CAR+0x75C,'<i',mass)
        self.put(self.addr(0x53CC18),'<I',CAR)
        self.native_input = bytes(h)
        expected = setter_model(h,setting,cheat)
        out = self.invoke_guarded(0x43DFF0,[setting])
        assert out==expected,('height model/guards',setting,cheat,mass)
        assert self.trace==[('cheat',6)]
        assert self.read(self.addr(0x53CC18),4)==struct.pack('<I',CAR)
        return out,self.trace

    def integrate(self, active, history, basis, alias, step, scale, seed):
        h = self.begin(seed)
        matrix = CAR if alias else WORLD
        put(h,CAR+0x750,'<I',matrix)
        for row,values in enumerate(BASES[basis]):
            put(h,matrix+row*16,'<3i',*values)
        put(h,CAR+0xB74,'<i',active)
        for wheel in range(4):
            older = (1000+wheel*31,2000-wheel*47,-3000+wheel*17) if history else (0,0,0)
            previous = tuple(v+(i+1)*113 for i,v in enumerate(older))
            current = tuple(v+(i+1)*113+((seed+wheel)%5-2)*(i+1)*701 for i,v in enumerate(previous))
            for offset,values in ((0x330,older),(0x300,previous),(0x270,current)):
                put(h,CAR+offset+wheel*12,'<3i',*values)
            put(h,CAR+0x8DC+wheel*4,'<i',current[1]+DISTANCES[(seed+wheel)%len(DISTANCES)])
            put(h,CAR+0x9C8+wheel*4,'<i',(-65536,-1,0,1,65536)[(seed+wheel)%5])
            put(h,CAR+0x9D8+wheel*4,'<i',(-0x3334,-0x3333,-0x3332,0,65536)[(seed+wheel*2)%5])
            put(h,CAR+0x9E8+wheel*4,'<i',0 if seed%7==0 else 0x4CCC)
            put(h,CAR+0x9F8+wheel*4,'<i',(0,0x4000,0x9999)[(seed+wheel)%3])
        self.put(self.addr(0x53CC18),'<I',CAR)
        self.put(self.addr(0x519C8C),'<i',step)
        self.put(self.addr(0x519C90),'<i',scale)
        self.native_input = bytes(h)
        expected = spring_model(h,step,scale,matrix)
        out = self.invoke_guarded(0x42E8E0,[])
        assert out==expected,('spring model/guards',active,history,basis,alias,step,scale,seed)
        assert self.read(self.addr(0x53CC18),4)==struct.pack('<I',CAR)
        return out,self.trace


def cases():
    return ([('setter',args) for args in itertools.product(SETTINGS,(0,1),MASSES)] +
            [('integrate',args) for args in itertools.product((0,1),(0,1),range(3),(0,1),
              (-16384,0,16384,65536,131072),(0,65536,131072))])


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    rebuilt = Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe'
    images = (Springs(ROOT/'cmr2bin/CMR2.exe'),Springs(rebuilt,entities))
    for i,(method,args) in enumerate(cases()):
        try:
            out = [getattr(image,method)(*args,3001+i) for image in images]
            assert out[0]==out[1], 'original / rebuilt'
        except Exception as error:
            raise AssertionError((method,args,3001+i)) from error
    print(f'{len(cases())} spring/setup cases: 0 differences; independent fixed-point models, cheat override, '
          'all four springs, strict ground/history/clamp boundaries, rotated/aliased matrices, zero/signed time steps, '
          'full heap guards, global widths and ABI')


if __name__=='__main__':
    main()
