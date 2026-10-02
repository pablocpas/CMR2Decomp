#!/usr/bin/env python3
"""Execute the original and rebuilt complete CPU neural control network.

Usage: differential_ai_network.py entities.json [rebuilt.exe]
No network helpers are mocked: input selection, fixed-point weights, activation,
layer bias and the five-channel lookup all execute in their own machine code.
"""
import itertools
import json
from pathlib import Path
import random
import sys

from unicorn.x86_const import UC_X86_REG_EIP, UC_X86_REG_ESP
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP


class Network(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}

    def run(self, seed, layers):
        rnd = random.Random(seed)
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, bytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        selection, desc, weights, state, output, activation = [HEAP+i for i in [0x1000,0x2000,0x3000,0x6000,0x7000,0x8000]]
        self.put(self.addr(0x58e394) + 7*4, '<I', selection)
        self.put(self.addr(0x58e394) + 47*4, '<I', desc)
        self.put(self.addr(0x58e394) + 67*4, '<I', activation)
        self.put(activation, '<64i', *(int(i * 65536 / 64) for i in range(64)))
        for i in range(5):
            self.put(selection + i*4, '<4b', i+1, 1, 0, 0)
        self.put(desc + 8, '<i', layers)
        selectors = [0,1,3,4,6,8,9,10,11,12,14,18,20]
        for i in range(1,6):
            self.put(desc+i, '<b', rnd.choice(selectors))
        for i in range(layers):
            self.put(desc+0xc+i, '<b', 5)
        for i in range(1,layers):
            self.put(desc+0x10+i*4, '<I', weights)
            self.put(weights, '<30i', *(rnd.choice([0,0x10000,-0x10000,0x2000000,-0x2000000]) for j in range(30)))
            weights += 120
        self.put(state, '<46i', *(rnd.randint(-0x800000,0x800000) for i in range(46)))
        for i in range(4):
            self.put(self.addr(0x58e0b8)+i*48, '<i', 0x10000)
        sp=STACK+0xff00
        self.put(sp, '<4I', STOP, 1, output, state)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.u.emu_start(self.addr(0x47c9a0), STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP) == STOP
        return self.read(HEAP,0x10000),self.read(self.addr(0x58e0b8),192)


def main():
    e=json.loads(Path(sys.argv[1]).read_text())
    original=Network(ROOT/'cmr2bin/CMR2.exe')
    rebuilt=Network(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',e)
    cases=0
    for seed,layers in itertools.product(range(64),range(1,5)):
        a,b=original.run(seed,layers),rebuilt.run(seed,layers)
        if a!=b:
            print('FAIL network',seed,layers)
            for part,(aa,bb) in enumerate(zip(a,b)):
                print('region',part,'offsets',[hex(i) for i,(x,y) in enumerate(zip(aa,bb)) if x!=y][:20])
            return 1
        cases+=1
    print(f'{cases} complete CPU network cases: identical five controls and every projection row')
    return 0


if __name__=='__main__':
    sys.exit(main())
