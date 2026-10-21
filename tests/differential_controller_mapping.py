#!/usr/bin/env python3
"""Controller-mask lookup and its binding predicate against independent models.

The lookup's predicate is controlled to check dispatch/defaults for every action;
the real predicate is also exercised for its nine declared keyboard entries.
Check read-only poisoned records/maps/heap, callback arguments and x86 ABI.
"""
import itertools
import json
from pathlib import Path
import random
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_car_info_records import Records, short
from differential_menu_list import ROOT
from differential_stage_lighting import signed

TABLE, SLOTS, STRIDE = 0x532250, 0x5168F4, 0x2F0


def cases():
    return ([('lookup',args) for args in itertools.product((0,1,2,7),(0,2,5),range(-1,11),
              (0,1,0x8000,0xFFFF),(0,1,-1))] +
            [('bound',args) for args in itertools.product((0,5),range(9),(0,1,0x8000,0xFFFF),(0,1,255))])


class Mappings(Records):
    def setup(self, seed, profile, action, mask, key=0):
        h = self.begin(seed)
        self.table = bytearray(random.Random(seed^0x41D).randbytes(6*STRIDE))
        if 0 <= action < 10:
            struct.pack_into('<H',self.table,profile*STRIDE+0x128+action*2,mask)
        if 0 <= action < 9:
            self.table[profile*STRIDE+0x13E+action] = key
        self.slots = [(seed+i)%6 for i in range(8)]
        self.put(self.addr(TABLE),'<'+str(len(self.table))+'s',bytes(self.table))
        return h

    def provider(self, address, nargs):
        action,pointer = self.args(nargs)
        profile = (pointer-self.addr(TABLE))//STRIDE
        assert pointer==self.addr(TABLE)+profile*STRIDE and 0<=profile<6
        self.trace.append((signed(action),profile))
        self.u.reg_write(UC_X86_REG_EAX,self.predicate & 0xFFFFFFFF)
        self.u.reg_write(UC_X86_REG_ECX,0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX,0x5A5A5A5A)

    def check(self, target, args, h, expected, trace):
        self.put(self.addr(SLOTS),'<8H',*self.slots)
        self.native_input = bytes(self.table)+struct.pack('<8H',*self.slots)
        assert self.invoke_guarded(target,args)==h,'read-only heap'
        assert self.read(self.addr(TABLE),6*STRIDE)==self.table,'read-only controller records'
        assert self.read(self.addr(SLOTS),16)==struct.pack('<8H',*self.slots),'read-only slot map'
        result = self.u.reg_read(UC_X86_REG_EAX)
        result = short(result) if target==0x40C2A0 else signed(result)
        assert (result,self.trace)==(expected,trace),(hex(target),args,result,self.trace,expected,trace)
        return result,self.trace

    def lookup(self, slot, profile, action, mask, predicate, seed):
        h = self.setup(seed,profile,action,mask)
        self.slots[slot] = profile
        self.predicate = predicate
        self.intercept((0x40C270,2))
        initial = short(mask) if 0<=action<10 else 0
        skipped = initial==0 and slot<2
        fixed = skipped or predicate!=0
        expected = (1<<action if 0<=action<10 else 0) if fixed else initial
        return self.check(0x40C2A0,[slot,action],h,expected,[] if skipped else [(action,profile)])

    def bound(self, profile, action, mask, key, seed):
        h = self.setup(seed,profile,action,mask,key)
        return self.check(0x40C270,[action,self.addr(TABLE)+profile*STRIDE],h,int(mask!=0 and key!=0),[])


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    rebuilt = Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe'
    images = (Mappings(ROOT/'cmr2bin/CMR2.exe'),Mappings(rebuilt,entities))
    for i,(method,args) in enumerate(cases()):
        try:
            outputs = [getattr(image,method)(*args,11003+i) for image in images]
            assert outputs[0]==outputs[1], 'original / rebuilt'
        except Exception as error:
            raise AssertionError((method,args,11003+i)) from error
    print(f'PASS: {len(cases())} controller mapping cases; signed masks, slot/profile aliases, all ten actions and defaults, '
          'controlled predicate dispatch plus real nine-key predicate, whole-record/map/heap guards, global writes and ABI')


if __name__=='__main__':
    main()
