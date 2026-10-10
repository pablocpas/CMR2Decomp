#!/usr/bin/env python3
"""Complete placement, wheel-travel and effect bodies; guarded heap and ABI.

Terrain/scene/surface-parameter leaves are controlled. Weather lookup and noise
copy execute their actual bodies, including when called by placement/reset.
Wheel travel also has an independent integer model. No drawing is involved.
"""
import json
import math
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_car_info_records import Records, put, trunc
from differential_menu_list import ROOT, HEAP
from differential_stage_lighting import mul, signed

CAR, WORLD, BODY, NODE = (HEAP + i for i in (0x1000, 0x3000, 0x4000, 0x5000))
POS, DIR, RAMPS = (HEAP + i for i in (0x6000, 0x6100, 0x9000))
TARGETS = {'place': 0x431ff0, 'reset': 0x43e680, 'travel': 0x42e450,
           'ramp': 0x460c80, 'noise': 0x4789b0, 'effect': 0x46eeb0}
PROVIDERS = ((0x43eef0, 1), (0x4930e0, 2), (0x42de20, 0),
             (0x42f820, 0), (0x42f8c0, 0), (0x432b30, 0),
             (0x42c870, 1), (0x4ac480, 2), (0x4781d0, 2),
             (0x42e8e0, 0), (0x42b5f0, 1))
SURFACES = (-32768, -1, 0, 1, 3, 12, 13, 26, 27, 32767)
BLENDS = (-2147483648, -1, 0, 1, 2147483647)


def get(h, offset, fmt='<i'):
    return struct.unpack_from(fmt, h, CAR - HEAP + offset)[0]


def vector(h, offset):
    return struct.unpack_from('<3i', h, CAR - HEAP + offset)


def travel_model(h):
    h = bytearray(h)
    normal = vector(h, 0x48c)
    dot = signed(sum(a*b for a, b in zip(vector(h, 0x36c), normal)) >> 16)
    def store(offset, value):
        put(h, CAR + offset, '<i', signed(value))
    if dot > 0xcccc:
        inverse = signed(trunc(65536 << 16, dot))
        values = [mul(-mul(signed(get(h, 0x274+12*i)-get(h, 0x8dc+4*i)), normal[1]), inverse)
                  for i in range(4)]
        avg = trunc(signed(sum(values)), 4)
        offsets = [mul(get(h, 0x9c0), signed(v-avg)) for v in values]
        maximum = max(map(abs, offsets))
        for i in range(4):
            d = signed(get(h, 0x8dc+4*i)-get(h, 0x274+12*i)-get(h, 0x958))
            store(0x928+4*i, max(d, get(h, 0xa08)) if d < 0 else 0)
            store(0x808+4*i, mul(offsets[i], trunc(65536 << 16, maximum))
                  if maximum > 65536 else offsets[i])
    else:
        for i in range(4):
            store(0x928+4*i, 0); store(0x808+4*i, 0)
    for i in range(4):
        if get(h, 0xc00) or get(h, 0x9c+36*i) < 1 or get(h, 0xb2c+i, '<b') or not get(h, 0xb74):
            store(0x938+4*i, 0); store(0x948+4*i, 0)
            continue
        dot = signed(sum(a*b for a, b in zip(normal, vector(h, 0x42c+12*i))) >> 16)
        t = signed(get(h, 0x778)-dot)
        t = 65536 if t > 65536 else 0 if t < 0xccc else t
        if get(h, 0x948+4*i) >= 1:
            store(0x948+4*i, max(0, signed(get(h, 0x948+4*i)-t)))
        else:
            x, _, z = vector(h, 0x270+12*i)
            distance = abs(abs(x)-abs(z))
            if get(h, 0x938+4*i) == 0:
                store(0x938+4*i, mul(mul((distance % 0x401) << 6, t), get(h, 0x9c+36*i)))
            else:
                store(0x938+4*i, 0)
                store(0x948+4*i, mul(mul((distance % 0x201) << 7, 0x320000), get(h, 0xa0+36*i)))
    return h


class State(Records):
    def provider(self, address, nargs):
        args = tuple(self.args(nargs))
        self.trace.append((address, args))
        value = 0
        if address == 0x43eef0:
            assert args == (CAR,)
            p = struct.unpack('<3i', self.read(CAR+0x2d0, 12))
            for i in range(8):
                self.put(CAR+0x270+12*i, '<3i', *(p[j]+(i+1)*(j+2)*131+self.seed*7 for j in range(3)))
        elif address == 0x4930e0:
            assert args == (CAR, 8)
            for i in range(8):
                y = struct.unpack('<i', self.read(CAR+0x274+12*i, 4))[0]
                self.put(CAR+0x8dc+4*i, '<i', y+((self.seed+i)%5-2)*8192)
        elif address == 0x42de20:
            pass  # The configured normal is the controlled terrain result.
        elif address == 0x42f820:
            self.put(CAR+0xbac, '<8i', *((self.seed+i)%2 for i in range(8)))
        elif address == 0x42f8c0:
            y = struct.unpack('<i', self.read(CAR+0x2d4, 4))[0]
            self.put(CAR+0x2d4, '<i', y+0x3000+self.seed*128)
        elif address == 0x432b30:
            self.put(CAR+0x860, '<4i', *(65536+i*8192 for i in range(4)))
        elif address == 0x42c870:
            assert args == (get(self.heap, 0xb1a, '<b') & 0xffffffff,)
        elif address == 0x4ac480:
            assert args == (NODE, CAR+0x2d0)
            self.u.mem_write(NODE+0xc8, self.read(args[1], 12))  # current.position
        elif address == 0x4781d0:
            assert args[0] == CAR
            self.put(CAR+0xa78, '<I', args[1] ^ 0x12345678)
        elif address == 0x42e8e0:
            self.trace.append(('suspension', self.read(CAR+0x808, 16), self.read(CAR+0x928, 48)))
        elif address == 0x42b5f0:
            value = CAR
        else:
            raise AssertionError(hex(address))
        self.u.reg_write(UC_X86_REG_EAX, value)
        self.u.reg_write(UC_X86_REG_ECX, 0xa5a5a5a5)
        self.u.reg_write(UC_X86_REG_EDX, 0x5a5a5a5a)

    def run(self, kind, seed):
        h = self.begin(seed)
        self.seed = seed
        self.intercept(*PROVIDERS)
        self.put(self.addr(0x53cc18), '<I', CAR)
        self.put(self.addr(0x543ecc), '<I', RAMPS)
        self.put(self.addr(0x6e0ef4), '<4096H', *(math.isqrt((8+16*i)*65536) for i in range(4096)))
        for i in range(-128, 128):
            put(h, RAMPS+i*12+8, '<i', BLENDS[(seed+i)%len(BLENDS)])
        index = seed % 256
        put(h, CAR+0xb1a, '<B', index)
        normal = ((0,65536,0), (16384,60000,8192), (0,0,0), (0,-65536,0))[seed%4]
        put(h, CAR+0x48c, '<3i', *normal)
        put(h, CAR+0x36c, '<3i', 0, (0,0xcccc,0xcccd,65536)[(seed//4)%4], 0)
        heading = ((65536,0,0), (0,0,65536), (0,0,0), (65536,0,65536), (-65536,0,0))[seed%5]
        put(h, CAR+0x360, '<3i', *heading)
        put(h, POS, '<3i', seed*271-100000, seed*97-10000, 400000-seed*53)
        put(h, DIR, '<3i', *heading)
        put(h, CAR+0x2d0, '<3i', seed*271-100000, seed*97-10000, 400000-seed*53)
        world = (WORLD, CAR, WORLD)[seed%3]
        body = (BODY, CAR+0x40, world)[seed%3]
        put(h, CAR+0x720, '<I', NODE)
        put(h, CAR+0x750, '<2I', world, body)
        for i in range(8):
            put(h, CAR+0x270+12*i, '<3i', 1000*(i+1)+seed, 30000-i*15000+seed, -700*(i+2))
            put(h, CAR+0x42c+12*i, '<3i', 0, i*5000-7000, 0)
            put(h, CAR+0x8dc+4*i, '<i', i*12000-10000-seed)
            put(h, CAR+0x80+36*i+0x1c, '<2i', (-1,1,65536,131072)[(seed+i)%4], 65536)
            put(h, CAR+0xb2c+i, '<b', (0,0,0,-1)[(seed//16+i)%4])
            put(h, CAR+0xaae+2*i, '<h', SURFACES[(seed//8)%len(SURFACES)])
        for i in range(4):
            put(h, CAR+0x938+4*i, '<i', (0,1)[(seed//32+i)%2])
            put(h, CAR+0x948+4*i, '<i', (-1,0,1,65536)[(seed//64+i)%4])
        put(h, CAR+0x958, '<i', (seed%5-2)*20000)
        put(h, CAR+0xa08, '<i', -20000-(seed%3)*20000)
        put(h, CAR+0x9c0, '<i', (65536,262144,0,-65536)[seed%4])
        put(h, CAR+0x778, '<i', (0,0xccb,0xccc,65536,131072)[seed%5])
        put(h, CAR+0xc00, '<i', int(seed%11==0))
        put(h, CAR+0xb74, '<i', int(seed%7!=0))
        self.native_input = bytes(h)
        args = {'place': [POS, DIR], 'reset': [CAR], 'travel': [], 'ramp': [CAR],
                'noise': [CAR], 'effect': [seed-128, seed%8]}[kind]
        if kind == 'reset':
            self.allowed.add((self.addr(0x53cc18), 4))
        out = self.invoke_guarded(TARGETS[kind], args)
        result = self.u.reg_read(UC_X86_REG_EAX)
        if kind == 'travel':
            assert out == travel_model(h), ('independent travel model', seed)
        elif kind in ('ramp', 'effect'):
            blend = BLENDS[(seed+(index if index<128 else index-256))%5]
            expected = blend & 0xffffffff if kind=='ramp' else int(SURFACES[(seed//8)%10] in (0,3,12,13,26) and blend>0)
            assert out == h and (result if kind=='ramp' else result & 255) == expected, (kind, seed)
        elif kind == 'noise':
            expected = h[:]; put(expected, CAR+0xa74, '<i', get(h,0xa78))
            assert out == expected, ('noise copy and guards', seed)
        else:
            assert out[CAR-HEAP+0x384:CAR-HEAP+0x3a8] == out[CAR-HEAP+0x360:CAR-HEAP+0x384]
            assert out[CAR-HEAP+0x504:CAR-HEAP+0x564] == out[CAR-HEAP+0x4a4:CAR-HEAP+0x504]
            assert get(out,0xa74)==get(out,0xa78)
            assert out[CAR-HEAP+0x300:CAR-HEAP+0x330] == out[CAR-HEAP+0x270:CAR-HEAP+0x2a0]
            assert out[CAR-HEAP+0x330:CAR-HEAP+0x360] == out[CAR-HEAP+0x300:CAR-HEAP+0x330]
            if kind == 'place':
                assert out[body-HEAP:body-HEAP+64] == out[world-HEAP:world-HEAP+64]
                assert get(out,0xb1e,'<B')==1 and get(out,0xc14)==1
                assert not any(out[CAR-HEAP+0xa9e:CAR-HEAP+0xaae])
        return out, (result if kind=='ramp' else result & 255) if kind in ('ramp','effect') else 0, self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    original = State(ROOT/'cmr2bin/CMR2.exe')
    rebuilt = State(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe', entities)
    count = 0
    for kind, size in (('place',512),('reset',256),('travel',1024),('ramp',256),('noise',128),('effect',1280)):
        for seed in range(size):
            a, b = original.run(kind,seed), rebuilt.run(kind,seed)
            assert a == b, (kind,seed, 'original/rebuilt heap, return or provider order')
            count += 1
    print(f'{count} car initial-state cases: complete heap/ABI/provider order match; real weather/noise and independent wheel-travel model')


if __name__ == '__main__': main()
