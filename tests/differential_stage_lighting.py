#!/usr/bin/env python3
"""Original/rebuilt weather lighting state and final colour-provider traces.
Usage: differential_stage_lighting.py entities.json [rebuilt.exe]
Mesh creation and final drawing/scene setters are controlled leaves.
Weather blending also checks an independent signed fixed-point model, complete
guarded heap, global writes, the intermediate intensity store and callee ABI.
"""
import itertools, json, random, struct, sys
from pathlib import Path
from unicorn import UC_HOOK_MEM_WRITE
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP,
                              UC_X86_REG_EBX, UC_X86_REG_ESI, UC_X86_REG_EDI,
                              UC_X86_REG_EBP)
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP


def signed(value):
    value &= 0xffffffff
    return value if value < 0x80000000 else value - 0x100000000


def add(a, b):
    return signed(a + b)


def mul(a, b):
    return signed((signed(a) * signed(b)) >> 16)


def blend_model(values, factor, flag, refresh, yaw, view):
    """Integer model of the original colour/scene contract, including wrap."""
    state = list(values)
    trace = []
    intensity_stores = []
    diffuse = None
    distances = None
    if refresh:
        factor = min(signed(factor), 0x10000)
        state[0x5a] = factor & 0xffffffff

        def lerp(index):
            return add(signed(values[index]), mul(signed(values[index + 0x2d] - values[index]), factor))

        def vector(index):
            return [lerp(index + i) for i in range(3)]

        def scale(v, weight):
            return [mul(component, weight) for component in v]

        def plus(a, b):
            return [add(x, y) for x, y in zip(a, b)]

        def minus(a, b):
            return [signed(x - y) for x, y in zip(a, b)]

        def colour(v, alpha=255):
            return bytes([(component >> 16) & 255 for component in v] + [alpha & 255])

        def emit(address, *args):
            trace.append((address, list(args)))

        intensity_stores.append(signed(values[0x59] - values[0x2c]) & 0xffffffff)
        weight = lerp(0x2c)
        intensity_stores.append(weight & 0xffffffff)
        state[0x5b] = weight & 0xffffffff
        low = vector(0)
        high = plus(vector(3), low)
        reference = vector(6)
        diffuse = colour(vector(9))
        distance = lerp(0x2b)
        distances = (mul(distance, 0x66), mul(distance, 0x88))
        blend = lerp(0x27)
        object_colour = colour(vector(0x0f), lerp(0x29) >> 16)
        ambient = scale(vector(0x12), add(mul(weight, 0x3333), 0xcccc))
        ground = plus(scale(minus(vector(0x0c), ambient), weight), ambient)
        ground_ref = plus(scale(minus(reference, ground), mul(lerp(0x28), blend)), ground)
        ground_ref = plus(scale(minus(ground_ref, ambient), weight), ambient)
        sky = colour(vector(0x21), lerp(0x24) >> 16)
        if values[0x5d] != 0:
            emit(0x492fe0, colour(vector(0x1e)), lerp(0x25) & 0xffffffff, lerp(0x26) & 0xffffffff)
        ground_colour = colour(ground)
        if flag:
            ground_colour = bytes([255 if c > 235 else c + 20 for c in ground_colour[:3]] + [255])
        ramp = colour(vector(0x18), lerp(0x2a) >> 16)
        light = plus(scale(minus(vector(0x15), ambient), weight), ambient)
        if flag:
            object_ref = bytes([255, 255, 255, 250 if object_colour[3] > 200 else object_colour[3] + 50])
            reference_colour = ground_reference = bytes([255] * 4)
            light = [0xff0000] * 3
            blend = 0x4ccc
        else:
            object_ref = object_colour
            reference_colour = colour(reference)
            ground_reference = colour(ground_ref)
        emit(0x492220, object_colour, object_ref)
        emit(0x491d40, colour(low), colour(high), reference_colour, blend & 0xffffffff)
        emit(0x4920d0, ground_colour, ground_reference)
        emit(0x492470, ramp)
        emit(0x492520, sky)
        emit(0x4923d0, bytes([255, 255, 255, 255 if flag else 0]))
        emit(0x462cb0, colour(ambient))
        emit(0x492e30, struct.pack('<3i', *light))
        emit(0x4925c0, lerp(0x1b) & 0xffffffff, values[0x1c], values[0x1d])
        emit(0x463070)
    trace.extend([(0x4926f0, [0, 0, yaw]), (0x492bd0, [view]), (0x492f10, [])])
    return state, trace, diffuse, distances, intensity_stores


class Lighting(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.blend_running = False
        self.u.hook_add(UC_HOOK_MEM_WRITE, self.blend_write,
                        begin=self.base, end=self.base + self.size - 1)
        for address, nargs in [(0x4919a0,0),(0x406930,0),(0x407520,1),
                               (0x4925c0,3),(0x492900,1),(0x492fd0,1)]:
            self.callbacks[self.addr(address)] = (4*nargs,
                lambda a=address,n=nargs:self.provider(a,n))

    def blend_write(self, u, access, address, size, value, data):
        if self.blend_running:
            assert (address, size) in self.blend_globals, ('unexpected blend global write', hex(address), size)
            if address == self.member(0x547abc,0x547950):
                self.intensity_stores.append(value & 0xffffffff)

    def provider(self, address, nargs):
        args=self.args(nargs)
        self.trace.append((address,args))
        self.u.reg_write(UC_X86_REG_EAX, HEAP+0x3000 if address==0x407520 else 0)

    def reset(self):
        self.u.mem_write(self.base,bytes(self.memory))
        self.u.mem_write(HEAP,bytes(0x10000))
        self.u.mem_write(STACK,bytes(0x10000))
        self.trace=[]

    def invoke(self,address,args):
        sp=STACK+0xff00
        self.put(sp,'<'+'I'*(len(args)+1),STOP,*args)
        self.u.reg_write(UC_X86_REG_ESP,sp)
        self.u.emu_start(self.addr(address),STOP,count=1000000)
        assert self.u.reg_read(UC_X86_REG_EIP)==STOP

    def initial(self,seed,weather,missing):
        self.reset()
        r=random.Random(seed)
        self.u.mem_write(HEAP+0x1000,r.randbytes(0x48))
        self.u.mem_write(HEAP+0x2000,r.randbytes(0x48))
        self.u.mem_write(self.addr(0x547950),r.randbytes(0x178))
        self.put(HEAP+0x3000,'<2I',*weather)
        self.invoke(0x460da0,[0 if missing&1 else HEAP+0x1000,
                              0 if missing&2 else HEAP+0x2000])
        return self.read(self.addr(0x547950),0x178),self.trace

    def colour_provider(self,address,nargs,kind):
        args=list(self.args(nargs))
        for index,size in kind:
            args[index]=self.read(args[index],size)
        self.trace.append((address,args))
        self.u.reg_write(UC_X86_REG_EAX,0)

    def blend(self,seed,factor,view,flag,dirty=1,clock=0,changed=1,
              weather_word=0x00010001,view_kind=1,fog=1,profile=0,view_level=None):
        self.reset()
        r=random.Random(seed)
        self.u.mem_write(HEAP, r.randbytes(0x10000))
        self.u.mem_write(STACK, bytes([0xa5 if seed & 1 else 0]) * 0x10000)
        limits=(0x80000000,0xffffffff,0xffff0000,0xffff,0,1,0x10000,0x7fffffff)
        values=[r.randrange(256)<<16 if profile==0 else
                r.choice(limits) if profile==1 else r.getrandbits(32) for _ in range(94)]
        if profile==0:
            values[0x2c]=0x10000;values[0x59]=0x1999
        values[0x5a]=factor if not changed else 0xffffffff
        values[0x5c]=weather_word;values[0x5d]=fog
        self.put(self.addr(0x543d88),'<I',clock)
        self.put(self.addr(0x543d8c),'<I',0)
        self.u.mem_write(self.addr(0x543eb4),r.randbytes(4))
        self.put(self.addr(0x543d58),'<I',r.getrandbits(32))
        self.put(self.addr(0x543d5c),'<I',r.getrandbits(32))
        self.put(self.addr(0x543ef8),'<I',0x12345678 if seed&1 else 0)
        self.put(self.addr(0x547950),'<94I',*values)
        self.put(self.addr(0x543eb8),'<I',HEAP+0x4000)
        self.put(self.addr(0x547ac8),'<I',HEAP+0x5000)
        self.put(HEAP+0x4000+view*44+28,'<I',factor)
        self.put(HEAP+0x4000+view*44+40,'<I',dirty)
        self.put(HEAP+0x5000+view*376,'<I',view_kind)
        level=(0x10000 if flag else 0) if view_level is None else view_level
        self.put(HEAP+0x5000+view*376+84,'<I',level)
        specs=[(0x492fe0,3,[(0,4)]),(0x492220,2,[(0,4),(1,4)]),
               (0x491d40,4,[(0,4),(1,4),(2,4)]),(0x4920d0,2,[(0,4),(1,4)]),
               (0x492470,1,[(0,4)]),(0x492520,1,[(0,4)]),(0x4923d0,1,[(0,4)]),
               (0x462cb0,1,[(0,4)]),(0x492e30,1,[(0,12)]),(0x4925c0,3,[]),
               (0x463070,0,[]),(0x4926f0,3,[]),(0x492bd0,1,[]),(0x492f10,0,[])]
        for address,nargs,kind in specs:
            self.callbacks[self.addr(address)]=(nargs*4,
                lambda a=address,n=nargs,k=kind:self.colour_provider(a,n,k))
        refresh=bool((factor != values[0x5a] and clock) or
                     (weather_word >> 16 != weather_word & 0xffff) or dirty)
        active=bool(weather_word & 0xffff != 0xffff and signed(level)>0xcccc and view_kind==1)
        yaw,=struct.unpack('<I',self.read(HEAP+0x4000+view*44+36,4))
        expected,trace,diffuse,distances,stores=blend_model(values,factor,active,refresh,yaw,view)
        heap=bytearray(self.read(HEAP,0x10000))
        if refresh:struct.pack_into('<I',heap,0x4000+view*44+40,0)
        self.blend_globals={(self.addr(a),4) for a in (0x543d58,0x543d5c,0x543ef8)}
        self.blend_globals.update((self.member(a,0x547950),4) for a in (0x547ab8,0x547abc))
        self.blend_globals.update((self.addr(0x543eb4)+i,1) for i in range(4))
        saved={reg:0x13579bdf+i*0x11111111 for i,reg in enumerate(
            (UC_X86_REG_EBX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP))}
        for reg,value in saved.items():self.u.reg_write(reg,value)
        self.intensity_stores=[]
        prior_diffuse=self.read(self.addr(0x543eb4),4)
        prior_distances=[self.read(self.addr(a),4) for a in (0x543d58,0x543d5c)]
        self.blend_running=True
        try:self.invoke(0x461c30,[view])
        finally:self.blend_running=False
        assert self.u.reg_read(UC_X86_REG_ESP)==STACK+0xff08,'blend ABI cleanup'
        assert all(self.u.reg_read(reg)==value for reg,value in saved.items()),'blend callee-saved registers'
        assert self.read(HEAP,0x10000)==heap,'blend heap model/guards'
        actual_state=struct.unpack('<94I',self.read(self.addr(0x547950),376))
        assert actual_state==tuple(expected),('blend state model',seed,factor,profile,
            [(hex(i),hex(a),hex(b)) for i,(a,b) in enumerate(zip(actual_state,expected)) if a!=b])
        assert self.trace==trace,('blend colour/provider model',seed,factor,view,flag,profile,self.trace,trace)
        assert self.intensity_stores==stores,('blend intermediate intensity stores',self.intensity_stores,stores)
        assert self.read(self.addr(0x543eb4),4)==(diffuse if diffuse is not None else prior_diffuse),'blend diffuse model'
        for i,a in enumerate((0x543d58,0x543d5c)):
            want=struct.pack('<i',distances[i]) if distances is not None else prior_distances[i]
            assert self.read(self.addr(a),4)==want,'blend distance model'
        assert self.read(self.addr(0x543ef8),4)==bytes(4),'blend reset flag'
        state=[self.read(self.addr(a),size) for a,size in
               [(0x547950,376),(0x543eb4,4),(0x543d58,4),(0x543d5c,4),(0x543ef8,4)]]
        return state,self.trace,self.read(HEAP+0x4000,88)


def extended_blends():
    factors=(0,1,0x8000,0xffff,0x10000,0x10001,0x18000,
             0xffff8000,0xffff0000,0xffffffff,0x80000000,0x7fffffff)
    words=(0,0x00010001,0x00010000,0x0000ffff,0xffffffff,0xffff0001)
    levels=(0,0xcccc,0xcccd,0x10000,0x80000000,0x7fffffff)
    for seed,(profile,factor,view,word,kind) in enumerate(
            itertools.product(range(3),factors,range(3),words,range(3)),100):
        yield (seed,factor,view,seed&1,(seed>>1)&1,(seed>>2)&1,(seed>>3)&1,
               word,kind,(seed>>4)&1,profile,levels[seed%len(levels)])


def mutation_probe(path):
    instance=Lighting(path)
    memory=bytearray(instance.memory)
    offset=0x461d92-instance.base
    assert memory[offset:offset+4]==bytes.fromhex('0facd010')
    memory[offset+3]=15
    instance.memory=bytes(memory)
    try:instance.blend(1,0x8000,0,0)
    except AssertionError as error:
        assert 'model' in str(error) or 'intensity stores' in str(error),error
    else:raise AssertionError('weather intensity shift mutation survived')


def main():
    e=json.loads(Path(sys.argv[1]).read_text())
    a=Lighting(ROOT/'cmr2bin/CMR2.exe')
    b=Lighting(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e)
    count=0
    for seed,weather,missing in itertools.product(range(5),itertools.product(range(9),repeat=2),range(4)):
        aa,bb=a.initial(seed,weather,missing),b.initial(seed,weather,missing)
        if aa!=bb:
            print('FAIL initial lighting',seed,weather,missing)
            for i in range(94):
                if aa[0][i*4:i*4+4]!=bb[0][i*4:i*4+4]:
                    print(hex(i),aa[0][i*4:i*4+4].hex(),bb[0][i*4:i*4+4].hex())
            print(aa[1],bb[1]);return 1
        count+=1
    print(count,'weather lighting initialisation cases: identical complete state and providers')
    for seed,factor,view,flag,dirty,clock,changed in itertools.product(range(5),[0,1,0x8000,0xffff,0x10000,0x18000],range(2),range(2),range(2),range(2),range(2)):
        aa,bb=a.blend(seed,factor,view,flag,dirty,clock,changed),b.blend(seed,factor,view,flag,dirty,clock,changed)
        if aa!=bb:
            print('FAIL weather blend',seed,factor,view,flag,dirty,clock,changed)
            for x,y in zip(aa[1],bb[1]):
                if x!=y:print(x,y)
            print('state equal',aa[0]==bb[0],'object equal',aa[2]==bb[2]);return 1
        count+=1
    for case in extended_blends():
        assert a.blend(*case)==b.blend(*case),('differential extended weather blend',case)
        count+=1
    print(count,'weather initialisation/blend cases: model, heap guards, ABI, intensity stores and emitted colours identical')
    mutation_probe(ROOT/'cmr2bin/CMR2.exe')
    print('Wrong weather intensity fixed-point shift rejected by independent model')
    return 0


if __name__=='__main__':sys.exit(main())
