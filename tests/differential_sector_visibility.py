#!/usr/bin/env python3
"""Compare complete original/rebuilt sector culling on valid camera/grid states.
Usage: differential_sector_visibility.py entities.json [rebuilt.exe]
The culling, triangle tests and fixed-point geometry all execute unchanged.
"""
import itertools,json,math,struct,sys
from pathlib import Path
from differential_stage_lighting import Lighting
from differential_menu_list import ROOT,HEAP
from unicorn.x86_const import UC_X86_REG_EAX


class Sectors(Lighting):
    def __init__(self,path,entities=None):
        super().__init__(path,entities)
        self.callbacks={}
        self.sqrt_table=struct.pack('<4096H',*(math.isqrt((8+16*i)*65536) for i in range(4096)))

    def detail_distance(self,base,level):
        self.reset()
        self.put(self.addr(0x520b74),'<I',HEAP)
        self.put(HEAP+0x3c4,'<i',base<<16)
        self.put(self.addr(0x52afa0)+0x30,'<I',(0x12345678 & ~(15<<21)) | (level<<21))
        self.invoke(0x423f30,[])
        result=self.u.reg_read(UC_X86_REG_EAX)
        assert result==((base<<16)*(level+1))//2, 'distance setting must scale the base distance'
        return result

    def cull(self,row,col,direction,radius,size=25,origin=(0,0),offset=0):
        self.reset()
        self.u.mem_write(self.addr(0x6e0ef4),self.sqrt_table)
        ox,oz=origin
        values={0x5207b0:radius<<16,0x71f600:16,0x72d248:size<<15,
                0x72d24c:size<<16,0x72d458:16,0x72d55c:0x10000//size,
                0x72d568:256,0x6ef5f0:ox<<16,0x6ef5f4:oz<<16}
        for a,v in values.items():self.put(self.addr(a),'<i',v)
        for y in range(16):
            for x in range(16):
                i=y*16+x;p=HEAP+0x2000+i*0x88
                self.put(self.addr(0x71f608)+i*4,'<I',p)
                px=(ox+x*size)<<16;pz=(oz-y*size)<<16
                self.put(p,'<3i',px,0,pz)
                loX=px-(size<<15);hiX=px+(size<<15)
                loZ=pz-(size<<15);hiZ=pz+(size<<15)
                self.put(p+0x5c,'<8i',loX,loZ,hiX,loZ,hiX,hiZ,loX,hiZ)
                self.put(p+0x7c,'<3i',-1,-2,-3)
        angle=direction*math.pi/4
        self.put(HEAP+0xf8,'<3i',round(math.sin(angle)*65536),0,round(math.cos(angle)*65536))
        self.put(HEAP+0x108,'<3i',((ox+col*size)<<16)+offset,0,((oz-row*size)<<16)-offset)
        self.invoke(0x4b7de0,[HEAP,0])
        count=int.from_bytes(self.read(self.addr(0x72d570),4),'little')
        return [self.read(self.addr(0x72d258),512),
                self.read(self.addr(0x6ed5f0),count*2),
                [self.read(HEAP+0x2000+i*0x88+0x7c,12) for i in range(256)]]


def main():
    e=json.loads(Path(sys.argv[1]).read_text())
    a=Sectors(ROOT/'cmr2bin/CMR2.exe');b=Sectors(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e)
    count=0
    for base,level in itertools.product([50,100,250],range(10)):
        assert a.detail_distance(base,level)==b.detail_distance(base,level)
        count+=1
    for base,layout in itertools.product(
            itertools.product([0,4,8,15],[0,4,8,15],range(8),[50,100,150,250,500]),
            [(25,(0,0),0),(25,(-1200,750),7<<16),(50,(350,-900),-12<<16)]):
        case=(*base,*layout)
        aa,bb=a.cull(*case),b.cull(*case)
        if aa!=bb:
            print('FAIL sector visibility',case,'visible counts',len(aa[1])//2,len(bb[1])//2)
            print('original',aa[1].hex(),'rebuilt',bb[1].hex());return 1
        count+=1
    near=a.cull(8,8,0,50)
    far=a.cull(8,8,0,250)
    assert len(far[1])>len(near[1])>18, 'fixtures must exercise the view triangle beyond neighboring sectors'
    print(count,'distance and sector visibility cases: identical settings, sectors, distances and flags; initialized square roots, shifted grids and far-plane growth')
    return 0


if __name__=='__main__':sys.exit(main())
