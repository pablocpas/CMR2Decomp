#!/usr/bin/env python3
"""Compare the menu pulse and the full RGBA value returned to draw callers.

Usage: differential_menu_colour.py entities.json [rebuilt.exe]
The real pulse and getter execute; only the frame clock is supplied.
"""
import json
import math
from pathlib import Path
import sys
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_ESP
from differential_menu_list import Drawing, ROOT, HEAP, STACK, STOP


class Colour(Drawing):
    def __init__(self, path, entities=None):
        super().__init__(path, entities)
        self.callbacks = {self.addr(0x4a9b80): (0, self.clock)}

    def clock(self):
        self.u.reg_write(UC_X86_REG_EAX, self.tick)

    def call(self, address):
        sp = STACK+0xff00
        self.put(sp, '<I', STOP)
        self.u.reg_write(UC_X86_REG_ESP, sp)
        self.u.emu_start(self.addr(address), STOP, count=10000)
        assert self.u.reg_read(UC_X86_REG_EIP)==STOP

    def run(self, tick):
        self.tick = tick
        self.u.mem_write(self.base, bytes(self.memory))
        self.u.mem_write(STACK, bytes(0x10000))
        self.put(self.addr(0x6e2ef4), '<4096i',
                 *(int(math.sin(i*math.tau/4096)*65536) for i in range(4096)))
        self.put(self.addr(0x82b1bc), '<I', 0xdeadbeef)
        self.call(0x501ac0)
        self.call(0x501ab0)
        pointer = self.u.reg_read(UC_X86_REG_EAX)
        assert self.read(self.addr(0x82b1bc),4)==bytes.fromhex('efbeadde')
        return self.read(pointer,4)


def main():
    entities=json.loads(Path(sys.argv[1]).read_text())
    a=Colour(ROOT/'cmr2bin/CMR2.exe')
    b=Colour(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities)
    ticks=list(range(120))+list(range(0xffffff88,0x100000000))
    for tick in ticks:
        aa,bb=a.run(tick),b.run(tick)
        if aa!=bb:
            print('FAIL RGBA pulse',tick,aa.hex(),bb.hex())
            return 1
        assert aa[0]==aa[1]==aa[2] and aa[3]==255
    print(f'{len(ticks)} menu colour cases: identical full RGBA read by drawing callers, alpha 255')
    return 0


if __name__=='__main__':sys.exit(main())
