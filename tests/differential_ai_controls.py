#!/usr/bin/env python3
"""Valid AI table/driver cases against original machine code.

Usage: differential_ai_controls.py entities.json [rebuilt.exe]
Route selection and car telemetry are supplied; control-table lookup runs for
real. Projection leaves use shared valid output rows in each image. Both the
five controls and the driver's caller return must survive.
"""
import itertools
import json
from pathlib import Path
import random
import sys
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP

class AI(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for addr, count, method in [
            (0x421370,1,self.node), (0x47cbc0,5,self.route),
            (0x498620,4,self.telemetry), (0x47c5e0,1,self.obstacle),
            (0x406940,0,self.zero), (0x406950,0,self.zero),
            (0x47cd10,4,self.modes), (0x4063f0,1,self.zero),
            (0x498ca0,3,self.project), (0x47ca30,1,self.noop)]:
            self.callbacks[self.addr(addr)] = (count * 4, method)

    def zero(self): self.u.reg_write(UC_X86_REG_EAX, 0)
    def noop(self): pass
    def node(self): self.u.reg_write(UC_X86_REG_EAX, 0)
    def obstacle(self): self.u.reg_write(UC_X86_REG_EAX, self.blocked)
    def route(self):
        car, table, node, variant, mode = self.args(5)
        self.put(variant, '<I', 0)
        self.put(mode, '<I', 1)
    def telemetry(self):
        self.put(self.addr(0x58e178), '<I', 0)
    def modes(self):
        car, modes, node, state = self.args(4)
        self.put(modes, '<2I', 0, 0)
    def project(self):
        table, state, rows = self.args(3)
        assert rows == self.addr(0x58e0b8)

    def run(self, root, slot, layers, values, blocked=0, preview=0):
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, bytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        self.blocked = blocked
        self.u.mem_write(self.addr(0x58e178), bytes(0xb8))
        self.put(self.addr(0x58e37c)+slot*4, '<I', HEAP+0x2000)
        self.put(HEAP+0x2000, '<I', 1)
        self.put(self.addr(0x58e394)+4, '<I', HEAP+0x3000)
        self.put(self.addr(0x58e394)+7*4, '<I', HEAP+0x4000)
        self.put(self.addr(0x58e394)+47*4, '<I', HEAP+0x5000)
        self.put(HEAP+0x5008, '<I', layers)
        self.put(self.addr(0x58e4a4), '<I', HEAP+0x6000)
        self.put(HEAP+0x600a, '<b', 5)
        self.put(HEAP+0x600e, '<b', 5)
        # Five entries select distinct outputs of the last computed row.
        for i in range(5):
            self.put(HEAP+0x4000+i*4, '<4b', i+1, 1, 0, 0)
        self.put(self.addr(0x58e0b8)+(layers-1)*48+4, '<5i', *values)
        args = [1, HEAP+0x7000, self.addr(0x58e178)] if root==0x47c9a0 else [HEAP+0x1000,slot,preview]
        sp=STACK+0xff00
        self.put(sp,'<'+str(len(args)+1)+'I',STOP,*args)
        self.u.reg_write(UC_X86_REG_ESP,sp)
        self.trace=[]
        self.u.emu_start(self.addr(root),STOP,count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP)==STOP, 'AI driver did not return'
        return (self.read(HEAP,0x10000), self.read(self.addr(0x58e178),0xb8),
                self.read(self.addr(0x58e230),24))

def main():
    entities=json.loads(Path(sys.argv[1]).read_text())
    original=AI(ROOT/'cmr2bin/CMR2.exe')
    rebuilt=AI(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities)
    cases=0
    rng=random.Random(1742)
    for root,slot,layers in itertools.product([0x47c9a0,0x47bdd0],range(6),range(1,5)):
        for values in [(0,0,63,0,0),(63,0,0,63,1),(0,63,63,0,0), tuple(rng.randint(-63,63) for _ in range(5))]:
            for blocked,preview in ([(0,0)] if root==0x47c9a0 else [(0,0),(0,1),(5,0)]):
                args=(root,slot,layers,values,blocked,preview)
                try:
                    a,b=original.run(*args),rebuilt.run(*args)
                except Exception as e:
                    print('FAIL AI case', args, type(e).__name__, str(e));return 1
                if a!=b:
                    print('FAIL AI case',args)
                    for part,(aa,bb) in enumerate(zip(a,b)):
                        diffs=[(i,x,y) for i,(x,y) in enumerate(zip(aa,bb)) if x!=y]
                        if diffs: print('memory region',part,'differences',diffs[:12])
                    return 1
                cases+=1
    print(f'{cases} AI cases: identical five-channel outputs, car controls and driver state')
    return 0
if __name__=='__main__':sys.exit(main())
