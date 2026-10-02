#!/usr/bin/env python3
"""Compare per-wheel car lighting and the final contiguous RGB light buffer.
Usage: differential_car_lighting.py entities.json [rebuilt.exe]
Ground samples and scene colours are controlled; the RGB conversion runs x86.
"""
import itertools,json,random,struct,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT,HEAP

class CarLighting(Lighting):
    def __init__(self,path,entities=None):
        super().__init__(path,entities);self.callbacks={}
        for addr,nargs in [(0x42b5f0,1),(0x491790,6),(0x4917f0,3),(0x4b6340,3),
                           (0x4b3ae0,2),(0x4b3840,1),(0x422fb0,1),(0x4984b0,2),(0x4b34d0,4)]:
            self.callbacks[self.addr(addr)]=(4*nargs,lambda a=addr,n=nargs:self.provide(a,n))
    def provide(self,address,nargs):
        args=self.args(nargs);result=0
        if address==0x42b5f0:result=HEAP+0x2000
        elif address==0x491790:
            for i,p in enumerate(args[1:]):self.put(p,'<H',i)
        elif address==0x4b6340:
            result=self.heights[self.wheel]&0xffffffff;self.wheel+=1
        elif address==0x4b3ae0:self.u.mem_write(args[0],self.colour)
        elif address==0x4b3840:self.u.mem_write(args[0],self.ambient)
        elif address==0x422fb0:result=self.player
        elif address in [0x4984b0,0x4b34d0]:self.trace.append((address,args))
        self.u.reg_write(UC_X86_REG_EAX,result)
    def run_car(self,seed,mask,player):
        self.reset();r=random.Random(seed);self.wheel=0;self.player=player
        self.colour=bytes(r.randrange(256) for _ in range(3))+b'\xff'
        self.ambient=bytes(r.randrange(256) for _ in range(3))+b'\xff'
        self.heights=[r.randrange(-0x8000,0x20000) for _ in range(4)]
        self.put(HEAP+0x2000+0xa9e,'<4h',*[i if mask&(1<<i) else -1 for i in range(4)])
        self.put(HEAP+0x2000+0xb1a,'<b',0)
        self.put(self.addr(0x5920b0),'<I',HEAP+0x6000)
        self.invoke(0x462aa0,[0,0])
        return self.read(HEAP+0x2000,0xc00),self.read(self.addr(0x543f28),256),self.trace

def main():
    e=json.loads(Path(sys.argv[1]).read_text());a=CarLighting(ROOT/'cmr2bin/CMR2.exe')
    b=CarLighting(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e)
    count=0
    for case in itertools.product(range(12),range(16),range(2)):
        aa,bb=a.run_car(*case),b.run_car(*case)
        if aa!=bb:
            print('FAIL car lighting',case,'state',aa[:2]==bb[:2]);print(aa[2],bb[2]);return 1
        count+=1
    print(count,'car lighting cases: identical wheel colours, RGB light and car state');return 0
if __name__=='__main__':sys.exit(main())
