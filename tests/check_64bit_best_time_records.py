#!/usr/bin/env python3
"""Run the actual five-pass builder and identifier helpers with native addresses.

The oracle is the original Win32 executable, including all unused output bytes.
Only its category-save provider is controlled; native inputs live above 4 GB.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from check_64bit_car_contacts import ROOT, body
from differential_best_time_codes import BestTimeCodes, TABLES
from differential_menu_list import HEAP


class Oracle(BestTimeCodes):
    def record(self):
        if self.calls == 0:
            self.native_input = self.read(HEAP + 0x1000, 0x600)
            self.native_input += b''.join(self.read(self.addr(a), n) for a, n in TABLES)
        super().record()


PRELUDE = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include "BestTimeRecords.h"
typedef unsigned char BYTE;
class CFrontend { public: static char m_stringDest[260]; };
char CFrontend::m_stringDest[260];
BYTE *saved;
unsigned calls;
BYTE *RallyData_GetAvailableCategorySaveRecord(int index) {
    assert(index==0 && reinterpret_cast<uintptr_t>(saved)>UINT32_MAX);
    ++calls;return saved;
}
void FrontendProfile_BuildGuestIdentifier(unsigned, unsigned, BYTE, char *);
void FrontendProfile_ScrambleIdentifier(unsigned, unsigned *, char *, char *);
// Controlled formatting leaf: indexed copy avoids the unchanged Win32
// formatter's cursor one byte before its array (covered by the x86 harness).
void FrontendText_FormatCodeGroups(char *text) {
    strcpy(CFrontend::m_stringDest,text); unsigned out=0;
    for(unsigned i=0;i<strlen(CFrontend::m_stringDest);i++) {
        text[out++]=CFrontend::m_stringDest[i]; if((i+1)%4==0)text[out++]=' ';
    }
    text[out]=0;
}
'''


def main():
    game = (ROOT / 'CMR2Decomp/GameInfo.cpp').read_text(encoding='latin1')
    frontend = (ROOT / 'CMR2Decomp/FrontendScreens.cpp').read_text(encoding='latin1')
    definitions = '\n'.join('BYTE g_unk0x%08x[%d];' % (a, n) for a, n in TABLES)
    for name in ('g_unk0x00526ea4', 'g_unk0x00526eb0', 'g_unk0x00526ebc'):
        definitions += '\n' + re.search(r'char '+name+r'\[12\] = .*?;', frontend).group()
    functions = body(game, 'FrontendRecords_BuildScrambledBestTimeTables')
    functions += body(frontend, 'FrontendProfile_ScrambleIdentifier')
    functions += body(frontend, 'FrontendProfile_BuildGuestIdentifier')
    main_code = '''
int main() {
    static_assert(sizeof(void *)==8,"native pointers");
    static_assert(sizeof(BestTimeCursor)==sizeof(void *),"scratch preserves addresses");
    alignas(16) BYTE nativeSaved[0x600];saved=nativeSaved;
    assert(fread(saved,1,0x600,stdin)==0x600);
'''
    for a, n in TABLES:
        main_code += 'assert(fread(g_unk0x%08x,1,%d,stdin)==%d);\n' % (a, n, n)
    main_code += 'FrontendRecords_BuildScrambledBestTimeTables();assert(calls==250);\n'
    main_code += 'assert(fwrite(saved,1,0x600,stdout)==0x600);\n'
    for a, n in TABLES:
        main_code += 'assert(fwrite(g_unk0x%08x,1,%d,stdout)==%d);\n' % (a, n, n)
    main_code += '}\n'
    oracle = Oracle(ROOT / 'cmr2bin/CMR2.exe')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-best-times-') as directory:
        tmp = Path(directory)
        shutil.copyfile(ROOT / 'CMR2Decomp/BestTimeRecords.h', tmp / 'BestTimeRecords.h')
        fixture = tmp / 'records.cpp';fixture.write_text(PRELUDE+definitions+functions+main_code)
        executable = tmp / 'records'
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fwrapv','-fno-strict-aliasing',
                        '-fsanitize=address,undefined',str(fixture),'-o',str(executable)],check=True)
        for seed in range(64):
            outputs, _, _ = oracle.run(seed, 0xa5)
            expected = oracle.native_input[:0x600] + b''.join(outputs)
            result = subprocess.run([str(executable)],input=oracle.native_input,capture_output=True,
                                    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',
                                             UBSAN_OPTIONS='halt_on_error=1'))
            assert result.returncode==0,(seed,result.stderr.decode())
            assert result.stdout==expected,('native outputs/save guards',seed)
    print('PASS: three actual bodies on x64, 64 scenarios / 16000 provider calls, '
          'native save pointers above 4 GB; all five tables match original, ASan/UBSan')


if __name__ == '__main__':
    main()
