#!/usr/bin/env python3
"""Compare the complete eight-car contact bookkeeping with the original.

Usage: differential_car_contacts.py entities.json [rebuilt.exe]
Pair geometry/response are controlled leaves; pair enumeration, sector checks,
all eight counts/lists and separation timers execute in the actual code.
"""
import itertools
import json
from pathlib import Path
import random
import sys
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP


class Contacts(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {}
        for address, nargs in [(0x487b80,4),(0x486c30,4),(0x48a5f0,2),
                               (0x48ae90,2),(0x407270,0),(0x407e50,0),
                               (0x407e90,0)]:
            self.callbacks[self.addr(address)] = (4*nargs,
                lambda a=address,n=nargs:self.provider(a,n))

    def provider(self, address, nargs):
        args = self.args(nargs)
        if address == 0x487b80:
            self.u.reg_write(UC_X86_REG_EAX, 1)
        elif address == 0x48a5f0:
            a = (args[0]-HEAP)//0xc24
            b = (args[1]-HEAP)//0xc24
            self.u.reg_write(UC_X86_REG_EAX, int(self.pattern == 0 or (a+b)%3 == self.pattern-1))
        elif address == 0x48ae90:
            self.trace.append(tuple((p-HEAP)//0xc24 for p in args))
        elif address in [0x407270,0x407e50,0x407e90]:
            self.u.reg_write(UC_X86_REG_EAX, 0)

    def run(self, count, seed, pattern):
        rnd = random.Random(seed)
        self.pattern = pattern
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(HEAP, bytes(0x10000))
        self.u.mem_write(STACK, bytes(0x10000))
        # Poison all eight counts: every frame must clear both original dwords.
        self.u.mem_write(self.addr(0x5913dc), bytes([0x38]*8))
        self.u.mem_write(self.addr(0x5913f8), bytes([0xef]*64))
        self.put(self.addr(0x519c8c), '<i', 0x400)
        for i in range(8):
            car = HEAP+i*0xc24
            self.put(car+0xb1a, '<B', i)
            self.put(car+0xb00, '<4h', 3,4,5,6)
            self.put(car+0xc08, '<i', int(i%3==1))
            self.put(car+0x970, '<i', [0,0x200,0x800][i%3])
            self.put(car+0x974, '<i', 0x1000+i)
        order = list(range(8))
        rnd.shuffle(order)
        self.put(HEAP+0x8000, '<8h', *order)
        sp = STACK+0xff00
        self.put(sp, '<4I', STOP, HEAP, HEAP+0x8000, count)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.trace = []
        self.u.emu_start(self.addr(0x48a1f0), STOP, count=100000)
        assert self.u.reg_read(UC_X86_REG_EIP)==STOP
        return (self.read(HEAP,0x10000), self.read(self.addr(0x5913dc),8),
                self.read(self.addr(0x5913f8),64), self.trace)


def main():
    entities=json.loads(Path(sys.argv[1]).read_text())
    a=Contacts(ROOT/'cmr2bin/CMR2.exe')
    b=Contacts(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities)
    cases=0
    for args in itertools.product(range(9),range(8),range(4)):
        aa,bb=a.run(*args),b.run(*args)
        if aa!=bb:
            print('FAIL contact bookkeeping',args,'counts original/rebuilt',aa[1],bb[1])
            return 1
        cases+=1
    print(f'{cases} car contact cases: all eight counts/lists, timers and pair responses identical')
    return 0


if __name__=='__main__':sys.exit(main())
