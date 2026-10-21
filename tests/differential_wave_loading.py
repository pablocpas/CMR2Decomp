#!/usr/bin/env python3
"""Fixed WAV loader against independent branch/ownership models.

File lookup, allocation, copy and COM methods are controlled leaves. Exercise
the complete loader and real HRESULT predicate; no DirectSound device required.
"""
import itertools
import json
from pathlib import Path
import struct
import sys

from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_ECX, UC_X86_REG_EDX
from differential_car_info_records import Records, put
from differential_menu_list import ROOT, HEAP, STOP

WAVE, NAME, FILE = HEAP+0x1000, HEAP+0x2000, HEAP+0x3000
BUFFER, SPATIAL, DEVICE = HEAP+0x4000, HEAP+0x4100, HEAP+0x4200
QUERY, MODE = STOP+0x100, STOP+0x200
BUFFERS, SPATIALS = 0x5A23EC, 0x5A1FC8
STATES = ('missing', 'invalid', 'create_fail', 'query_fail', 'copy_fail', 'mode_fail', 'ok', 'negative_bool', 'reload')
FORMATS = ((1,8,22050,1), (2,16,48000,32), (0,0,0,0), (65535,65535,0xFFFFFFFF,0xFFFFFFFF))


def cases():
    return list(itertools.product(range(4), (0,1,2,255), (0,1,-1), (0,1,255), STATES))


def model(case):
    recipe,flags,enabled,archive,state = case
    channels,bits,rate,size = FORMATS[recipe]
    slot = (0,7,199,3)[recipe]
    trace = [(1,)]
    if state == 'missing':
        return 0,trace
    if state != 'invalid':
        spatial = int(bool(flags & 1 and enabled))
        trace += [(2,slot), (3,slot,rate,bits,channels,spatial,size)]
        if state == 'create_fail':
            return 0,trace
        if spatial:
            trace += [(2,slot), (2,slot), (4,slot)]
            if state != 'query_fail':
                trace += [(2,slot), (5,slot,0,44,(size+7)&0xFFFFFFFF if state=='reload' else size)]
                if state == 'copy_fail':
                    return 0,trace
                trace += [(2,slot), (6,slot,0,1)]
        else:
            trace += [(2,slot), (5,slot,0,44,(size+7)&0xFFFFFFFF if state=='reload' else size)]
    if not archive:
        trace += [(7,)]
    return 1,trace


class WaveLoading(Records):
    def provider(self, address, nargs):
        args = self.args(nargs)
        state = self.case[-1]
        value = 0
        if address == 0x4A9DF0:
            file,name,found,identifier,skip = args
            assert (file,name,identifier,skip)==(FILE,NAME,0,0)
            self.put(found,'<B',self.case[3])
            self.trace.append((1,))
            value = 0 if state=='missing' else WAVE
        elif address == 0x4C669A:
            left,right,length = args
            assert (left,right,length)==(WAVE+8,self.addr(0x520A34),4)
            value = int(self.read(left,4)!=b'WAVE')
        elif address == 0x4B7780:
            self.trace.append((2,self.slot))
            value = self.slot
        elif address == 0x4A20C0:
            device,out,rate,bits,channels,spatial,size = args
            assert device==DEVICE and out==self.addr(BUFFERS)+self.slot*4
            self.trace.append((3,self.slot,rate,bits,channels,spatial,size))
            value = 0 if state=='create_fail' else -1 if state=='negative_bool' else 1
            if value:
                self.put(out,'<I',BUFFER)
                if state=='reload':
                    self.put(WAVE+40,'<I',(size+7)&0xFFFFFFFF)
                    put(self.expected_heap,WAVE+40,'<I',(size+7)&0xFFFFFFFF)
        elif address == QUERY:
            buffer,iid,out = args
            assert (buffer,iid,out)==(BUFFER,self.addr(0x511C38),self.addr(SPATIALS)+self.slot*4)
            self.trace.append((4,self.slot))
            value = 0x80004002 if state=='query_fail' else 7 if state=='negative_bool' else 0
            if state!='query_fail':
                self.put(out,'<I',SPATIAL)
        elif address == 0x4A2210:
            buffer,offset,data,size = args
            assert buffer==BUFFER
            self.trace.append((5,self.slot,offset,data-WAVE,size))
            value = 0 if state=='copy_fail' else -1 if state=='negative_bool' else 1
        elif address == MODE:
            spatial,mode,deferred = args
            assert spatial==SPATIAL
            self.trace.append((6,self.slot,mode,deferred))
            value = 0x80004005 if state=='mode_fail' else 0
        elif address == 0x4AADE0:
            assert args==(WAVE,)
            self.trace.append((7,))
        else:
            raise AssertionError(hex(address))
        self.u.reg_write(UC_X86_REG_EAX,value & 0xFFFFFFFF)
        self.u.reg_write(UC_X86_REG_ECX,0xA5A5A5A5)
        self.u.reg_write(UC_X86_REG_EDX,0x5A5A5A5A)

    def run_case(self, case, seed):
        self.case = case
        recipe,flags,enabled,archive,state = case
        h = self.begin(seed)
        channels,bits,rate,size = FORMATS[recipe]
        self.slot = (0,7,199,3)[recipe]
        # The loader ignores RIFF/fmt/data ids, format tag and chunk lengths.
        # Poison those bytes rather than silently testing only canonical headers.
        put(h,WAVE+8,'<4s',b'WAVE' if state!='invalid' else b'WAV\0')
        put(h,WAVE+22,'<HI',channels,rate)
        put(h,WAVE+34,'<H',bits)
        put(h,WAVE+40,'<I',size)
        put(h,NAME,'<12s',b'fixture.wav\0')
        put(h,BUFFER,'<I',HEAP+0x5000)
        put(h,HEAP+0x5000,'<I',QUERY)
        put(h,SPATIAL,'<I',HEAP+0x5100)
        put(h,HEAP+0x5100+0x48,'<I',MODE)
        self.native_input = bytes(h[WAVE-HEAP:WAVE-HEAP+256])
        self.expected_heap = h[:]
        self.put(self.addr(0x5A2838),'<i',enabled)
        self.put(self.addr(0x5A2844),'<I',DEVICE)
        before = bytes([0x93])*800
        for address in (BUFFERS,SPATIALS):
            self.u.mem_write(self.addr(address),before)
            self.allowed.add((self.addr(address)+self.slot*4,4))
        self.intercept((0x4A9DF0,5),(0x4B7780,0),(0x4A20C0,7),(0x4A2210,4),(0x4AADE0,1))
        self.callbacks[self.addr(0x4C669A)] = (0,lambda:self.provider(0x4C669A,3))
        self.callbacks[QUERY] = (12,lambda:self.provider(QUERY,3))
        self.callbacks[MODE] = (12,lambda:self.provider(MODE,3))
        actual_heap = self.invoke_guarded(0x4A1F50,[NAME,flags,FILE])
        result = self.u.reg_read(UC_X86_REG_EAX)
        expected,trace = model(case)
        assert (result,self.trace)==(expected,trace),(case,result,self.trace,expected,trace)
        assert actual_heap==self.expected_heap,'file, payload and heap guards'
        for address,pointer,event in ((BUFFERS,BUFFER,3),(SPATIALS,SPATIAL,4)):
            want = bytearray(before)
            if any(e[0]==event for e in trace) and state!=('create_fail' if event==3 else 'query_fail'):
                struct.pack_into('<I',want,self.slot*4,pointer)
            assert self.read(self.addr(address),800)==want,'whole pointer table and untouched slots'
        assert self.read(self.addr(0x5A2838),4)==struct.pack('<i',enabled)
        assert self.read(self.addr(0x5A2844),4)==struct.pack('<I',DEVICE)
        return result,self.trace


def main():
    entities = json.loads(Path(sys.argv[1]).read_text())
    images = (WaveLoading(ROOT/'cmr2bin/CMR2.exe'),
              WaveLoading(Path(sys.argv[2]) if len(sys.argv)>2 else ROOT/'build/CMR2.exe',entities))
    for i,case in enumerate(cases()):
        try:
            assert images[0].run_case(case,51003+i)==images[1].run_case(case,51003+i)
        except Exception as error:
            raise AssertionError((case,51003+i)) from error
    print(f'PASS: {len(cases())} WAV loader cases; real HRESULT predicate, format widths, 2D/3D failures, '
          'ownership, provider order, size reload, pointer tables, poisoned file/heap guards and ABI')


if __name__=='__main__':
    main()
