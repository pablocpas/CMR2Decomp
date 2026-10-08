#!/usr/bin/env python3
"""Compare CPU rally timing samples and complete split tables with original x86.
Usage: differential_rally_times.py entities.json [rebuilt.exe]
Route distances, difficulty, mode and random numbers are controlled providers.
"""
import itertools,json,random,sys
from pathlib import Path
from unicorn.x86_const import UC_X86_REG_EAX
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT,HEAP


class Times(Lighting):
    def __init__(self,path,entities=None):
        super().__init__(path,entities)
        self.callbacks={}
        for address,nargs in [(0x421420,0),(0x4583c0,0),(0x4583b0,1),
                               (0x405d90,0),(0x407e60,0),(0x4c6676,0)]:
            self.callbacks[self.addr(address)]=(nargs*4,lambda a=address:self.select(a))

    def select(self,address):
        values={0x421420:10000,0x4583c0:self.splits,0x405d90:self.difficulty,0x407e60:self.mode}
        if address==0x4583b0:value=self.args(1)[0]*(10000//self.splits)<<16
        elif address==0x4c6676:value=self.random.randrange(0x8000)
        else:value=values[address]
        self.u.reg_write(UC_X86_REG_EAX,value)

    def run_times(self,seed,splits,difficulty,mode):
        self.reset();self.splits=splits;self.difficulty=difficulty;self.mode=mode
        self.random=random.Random(seed);r=random.Random(seed)
        self.put(self.addr(0x54241c),'<I',HEAP+0x1000)
        self.u.mem_write(HEAP+0x1000,bytes(r.randrange(1,25) for _ in range(80)))
        for i in range(160):self.put(HEAP+0x1080+i*4,'<I',r.randrange(40,140)<<16)
        self.put(HEAP+0x3000+0xa0,'<4I',1200<<16,8000<<16,0x3000,0xa000)
        self.put(self.addr(0x541f98),'<I',15)
        self.put(self.addr(0x542c74),'<I',1)
        # Independent boundaries expose the first differing calculation.
        self.invoke(0x455f00,[HEAP+0x3000,HEAP+0x4000])
        samples=self.read(HEAP+0x4000,64)
        self.invoke(0x456330,[HEAP+0x3000])
        factors=self.read(HEAP+0x3000,0xb0)
        self.invoke(0x4564d0,[HEAP+0x3000,HEAP+0x5000])
        deltas=self.read(HEAP+0x5000,640)
        self.invoke(0x456710,[HEAP+0x5000])
        spread=self.read(HEAP+0x5000,640)
        self.invoke(0x456960,[HEAP+0x5000])
        return [samples,factors,deltas,spread,self.read(self.member(0x542198,0x541f98),640)]


def main():
    e=json.loads(Path(sys.argv[1]).read_text())
    a=Times(ROOT/'cmr2bin/CMR2.exe');b=Times(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e)
    count=0
    for case in itertools.product(range(10),[2,4,5,8],range(3),range(2)):
        aa,bb=a.run_times(*case),b.run_times(*case)
        if aa!=bb:
            print('FAIL rally times',case)
            for name,x,y in zip(['samples','factors','deltas','spread','split times'],aa,bb):
                if x!=y:print(name,'first difference',next(i for i in range(len(x)) if x[i]!=y[i]),x[:64].hex(),y[:64].hex())
            return 1
        count+=1
    print(count,'rally CPU timing cases: identical samples, factors, deltas and split times')
    return 0


if __name__=='__main__':sys.exit(main())
