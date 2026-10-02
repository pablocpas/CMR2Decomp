#!/usr/bin/env python3
"""Compare complete original/rebuilt sector culling on valid camera/grid states.
Usage: differential_sector_visibility.py entities.json [rebuilt.exe]
The culling, triangle tests and fixed-point geometry all execute unchanged.
"""
import itertools,json,math,sys
from pathlib import Path
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT,HEAP


class Sectors(Lighting):
    def __init__(self,path,entities=None):
        super().__init__(path,entities)
        self.callbacks={}

    def cull(self,row,col,direction,radius):
        self.reset()
        values={0x5207b0:radius<<16,0x71f600:16,0x72d248:25<<15,
                0x72d24c:25<<16,0x72d458:16,0x72d55c:0x10000//25,
                0x72d568:256,0x6ef5f0:0,0x6ef5f4:0}
        for a,v in values.items():self.put(self.addr(a),'<I',v)
        for y in range(16):
            for x in range(16):
                i=y*16+x;p=HEAP+0x2000+i*0x88
                self.put(self.addr(0x71f608)+i*4,'<I',p)
                self.put(p,'<3i',x*25<<16,0,-y*25<<16)
                loX=(x*25-12)<<16;hiX=(x*25+12)<<16
                loZ=(-y*25-12)<<16;hiZ=(-y*25+12)<<16
                self.put(p+0x5c,'<8i',loX,loZ,hiX,loZ,hiX,hiZ,loX,hiZ)
                self.put(p+0x7c,'<3i',-1,-2,-3)
        angle=direction*math.pi/4
        self.put(HEAP+0xf8,'<3i',round(math.sin(angle)*65536),0,round(math.cos(angle)*65536))
        self.put(HEAP+0x108,'<3i',col*25<<16,0,-row*25<<16)
        self.invoke(0x4b7de0,[HEAP,0])
        count=int.from_bytes(self.read(self.addr(0x72d570),4),'little')
        return [self.read(self.addr(0x72d258),512),
                self.read(self.addr(0x6ed5f0),count*2),
                [self.read(HEAP+0x2000+i*0x88+0x7c,12) for i in range(256)]]


def main():
    e=json.loads(Path(sys.argv[1]).read_text())
    a=Sectors(ROOT/'cmr2bin/CMR2.exe');b=Sectors(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e)
    count=0
    for case in itertools.product([0,4,8,15],[0,4,8,15],range(8),[50,100,150,250]):
        aa,bb=a.cull(*case),b.cull(*case)
        if aa!=bb:
            print('FAIL sector visibility',case,'visible counts',len(aa[1])//2,len(bb[1])//2)
            print('original',aa[1].hex(),'rebuilt',bb[1].hex());return 1
        count+=1
    print(count,'sector visibility cases: identical visible sectors, distances and flags')
    return 0


if __name__=='__main__':sys.exit(main())
