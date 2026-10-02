#!/usr/bin/env python3
"""Original/rebuilt weather lighting state and final colour-provider traces.
Usage: differential_stage_lighting.py entities.json [rebuilt.exe]
Mesh creation and final drawing/scene setters are controlled leaves.
"""
import itertools, json, random, struct, sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP


class Lighting(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        for address, nargs in [(0x4919a0,0),(0x406930,0),(0x407520,1),
                               (0x4925c0,3),(0x492900,1),(0x492fd0,1)]:
            self.callbacks[self.addr(address)] = (4*nargs,
                lambda a=address,n=nargs:self.provider(a,n))

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

    def blend(self,seed,factor,view,flag,dirty=1,clock=0,changed=1):
        self.reset()
        r=random.Random(seed)
        values=[r.randrange(256)<<16 for _ in range(94)]
        values[0x2c]=0x10000;values[0x59]=0x1999
        values[0x5a]=factor if not changed else 0xffffffff
        values[0x5c]=0x00010001;values[0x5d]=1
        self.put(self.addr(0x543d88),'<I',clock)
        self.put(self.addr(0x543d8c),'<I',0)
        self.put(self.addr(0x547950),'<94I',*values)
        self.put(self.addr(0x543eb8),'<I',HEAP+0x4000)
        self.put(self.addr(0x547ac8),'<I',HEAP+0x5000)
        self.put(HEAP+0x4000+view*44+28,'<I',factor)
        self.put(HEAP+0x4000+view*44+40,'<I',dirty)
        self.put(HEAP+0x5000+view*376,'<I',1)
        self.put(HEAP+0x5000+view*376+84,'<I',0x10000 if flag else 0)
        specs=[(0x492fe0,3,[(0,4)]),(0x492220,2,[(0,4),(1,4)]),
               (0x491d40,4,[(0,4),(1,4),(2,4)]),(0x4920d0,2,[(0,4),(1,4)]),
               (0x492470,1,[(0,4)]),(0x492520,1,[(0,4)]),(0x4923d0,1,[(0,4)]),
               (0x462cb0,1,[(0,4)]),(0x492e30,1,[(0,12)]),(0x4925c0,3,[]),
               (0x463070,0,[]),(0x4926f0,3,[]),(0x492bd0,1,[]),(0x492f10,0,[])]
        for address,nargs,kind in specs:
            self.callbacks[self.addr(address)]=(nargs*4,
                lambda a=address,n=nargs,k=kind:self.colour_provider(a,n,k))
        self.invoke(0x461c30,[view])
        state=[self.read(self.addr(a),size) for a,size in
               [(0x547950,376),(0x543eb4,4),(0x543d58,4),(0x543d5c,4),(0x543ef8,4)]]
        return state,self.trace,self.read(HEAP+0x4000,88)


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
    print(count,'weather initialisation/blend cases: identical state and all emitted colours')
    return 0


if __name__=='__main__':sys.exit(main())
