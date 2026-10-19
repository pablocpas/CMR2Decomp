#!/usr/bin/env python3
"""Actual result bodies with high pointers, production layouts and old-code failures."""
import re
from pathlib import Path
import struct
import subprocess
import tempfile
from check_64bit_car_contacts import ROOT
from differential_category_best_results import case, oracle

PORT = ROOT.parent / 'OpenCMR2'


def function(source, name):
    match = re.search(r'^\w+(?: \w+)? ' + re.escape(name) + r'\([^;]*?\)\s*\{', source, re.M)
    assert match, name
    end, depth = match.end(), 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[match.start():end]


PRE = r'''
#include "port/types.h"
#include "Graphics.h"
#include "GameInfo.h"
#include "CategoryBestResults.h"
#include <cassert>
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
Graphics *g_pGraphics;
short g_menuRect[4];
BYTE g_menuFrameColour[4] = {250,250,250,255};
class CFrontend {public: static char m_stringDest[260];};
char CFrontend::m_stringDest[260];
char *CGenericFileLoader::StrLowerPolish(char *p) {
    for (char *q=p; *q; ++q) if (*q>='A' && *q<='Z') *q+=32;
    return p;
}
int draws, bars, expectedX, lastWidth;
void Font_DrawText(int font, char *text, int x, int y, int *colour, int flags) {
    assert((uintptr_t)text > UINT32_MAX && font==2 && x==expectedX && y==67);
    assert(colour==(int *)g_menuFrameColour && flags==0x11);
    for(char *q=text; *q; ++q) assert(!(*q>='A' && *q<='Z'));
    lastWidth=(int)strlen(text)*7; ++draws;
}
int Font_GetTextWidth(int font, BYTE *text) {
    assert(font==2 && text==(BYTE *)CFrontend::m_stringDest);
    return (int)strlen((char *)text)*7;
}
void Sprite_FillRect(BYTE *context, short *rect, BYTE *colour, int layer) {
    assert(context==&g_pGraphics->field309_0x150 && colour==g_menuFrameColour && layer==2);
    assert(rect[0]==expectedX+8+lastWidth && rect[1]==37 && rect[2]==2 && rect[3]==41);
    expectedX=rect[0]+10; ++bars;
}
BYTE *currentRecord;
CategoryBestResults *best;
bool missing;
int providerIndex, marked;
BYTE g_unk0x00817420[40];
BYTE *RallyData_GetDriverEventResults(int i) {assert(i==providerIndex); return missing?nullptr:currentRecord;}
BYTE *RallyData_GetDriverBestResults(int i) {assert(i==providerIndex); return (BYTE *)best;}
BYTE *Frontend_GetEventRallyStageTime(int i) {assert(i==providerIndex); return missing?nullptr:currentRecord+0x14;}
BYTE *Frontend_GetEventRallyResult(int i) {assert(i==providerIndex); return missing?nullptr:currentRecord+0xc;}
BYTE RallyData_GetDriverRecordSelectionValue(BYTE i) {assert(i==providerIndex); return 0x45;}
BYTE RallyData_GetDriverAutomaticGearbox(BYTE i) {assert(i==providerIndex); return 3;}
void RallyData_MarkDriverRecordDirty(int i) {assert(i==providerIndex); ++marked;}
int fixtureRand() {return 0x7654;}
#define rand fixtureRand
'''

MAIN = r'''
int main() {
    static_assert(sizeof(void *)==8 && sizeof(CategoryBestResults)==0x504,"native record size");
    static_assert(offsetof(CategoryBestResults,rallyStageTimes)==0x150,"pointer-free file record");
    auto *storage=(BYTE *)malloc(2*1024*1024);
    assert(storage && (uintptr_t)storage>UINT32_MAX);
    Graphics graphics; g_pGraphics=&graphics;
    memset(&graphics,0xa5,sizeof(graphics)); graphics.resX=640;graphics.resY=480;
    Graphics oldGraphics=graphics;
    for(int n=0;n<=8;n++) for(int alias=0;alias<2;alias++) {
        char texts[8][16], *p[9]={};
        memset(texts,0xa5,sizeof(texts));
        for(int i=0;i<n;i++) {strcpy(texts[i],i%3==1?"":"STAGE TIME");p[i]=texts[alias?0:i];}
        draws=bars=0;expectedX=30;
        GameMenus_DrawTextRow(30,67,p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8]);
        assert(draws==n && bars==(n?n-1:0));
        assert(!memcmp(&graphics,&oldGraphics,sizeof(graphics)));
        for(int i=0;i<8;i++) assert((unsigned char)texts[i][15]==0xa5);
    }
    uint32_t input[7]; unsigned cases=0;
    while(fread(input,sizeof(input),1,stdin)==1) {
        assert(fread(storage,0x10000,1,stdin)==1);
        best=(CategoryBestResults *)(storage+0x1000); currentRecord=storage+0x3000;
        CFG_COUNTRY=input[1];CFG_STAGE=input[2];CFG_DIFFICULTY=input[3];CFG_GROUP=input[4];
        providerIndex=input[5];missing=input[6];marked=0;
        memset(g_unk0x00817420,0xa5,sizeof(g_unk0x00817420));
        BYTE result=0;
        switch(input[0]) {
        case 0:result=Frontend_MergeBestRallyStageTime(providerIndex,best);break;
        case 1:result=Frontend_MergeBestRallyResult(providerIndex,best);break;
        case 2:result=Frontend_MergeBestPlayerResult(providerIndex,best);break;
        case 3:result=Frontend_MergeBestArcadeStageTime(providerIndex);break;
        case 4:result=Frontend_MergeBestArcadeResult(providerIndex,best);break;
        default:assert(false);
        }
        assert(marked==result);
        assert(fwrite(&result,1,1,stdout)==1);
        assert(fwrite(g_unk0x00817420,sizeof(g_unk0x00817420),1,stdout)==1);
        assert(fwrite(storage,0x10000,1,stdout)==1); ++cases;
    }
    free(storage);
    fprintf(stderr,"PASS %u native result merges and 18 variadic rows, high pointers and full heap\n",cases);
}
'''


def main():
    source = (ROOT / 'CMR2Decomp/Frontend.cpp').read_text()
    rows = function((ROOT / 'CMR2Decomp/GameMenus.cpp').read_text(), 'GameMenus_DrawTextRow')
    names = []
    for addr in ('008173f8','008173fc','00817400','00817404'):
        names.append(re.search(r'// GLOBAL: CMR2 0x'+addr+r'\s*int (\w+);', source)[1])
    support = '\n'.join('int '+n+';' for n in names)
    for name in ('RecordHeader','DeviceResult','RecordResult','RecordEntry'):
        support += '\n'+re.search(r'struct '+name+r' \{.*?\};',source,re.S)[0]
    functions = ('Frontend_CopyOptionWord','Frontend_MergeFourNibbleOptionFields',
                 'Frontend_MergeFourTwoSixBitOptionFields','Frontend_MergeThreeTwoSixBitOptionFields',
                 'Frontend_MergeBestRallyStageTime','Frontend_MergeBestRallyResult',
                 'Frontend_MergeBestPlayerResult','Frontend_MergeBestArcadeStageTime',
                 'Frontend_MergeBestArcadeResult')
    support += '\n'+'\n'.join(function(source,n) for n in functions)
    main = MAIN
    for placeholder, name in zip(('CFG_COUNTRY','CFG_STAGE','CFG_DIFFICULTY','CFG_GROUP'), names):
        main = main.replace(placeholder,name)
    code = PRE + support + rows + main
    old_rows = rows.replace('va_list ppNext;\n    va_start(ppNext, pText);','char **ppNext = &pText;')
    old_rows = old_rows.replace('va_arg(ppNext, char *)','*++ppNext').replace('    va_end(ppNext);','')
    old_pointers = support.replace('base = (CategoryBestResults *)RallyData_GetDriverBestResults(index);',
                                  'base = (CategoryBestResults *)(intptr_t)(int)(intptr_t)RallyData_GetDriverBestResults(index);')
    assert old_rows != rows and old_pointers != support
    inputs, expected = bytearray(), bytearray()
    for seed in range(0,576,3):
        for kind in range(5):
            heap, (country,stage,level,group), offset, source_offset = case(seed,kind)
            missing = seed%13==0 and kind in (0,1,2)
            wanted, result = oracle(heap,kind,offset,source_offset,missing)
            dirty = bytearray(b'\xa5'*40)
            if result and kind != 3:dirty[(seed%4)*10+(0,1,2,0,3)[kind]]=1
            inputs += struct.pack('<7I',kind,country,stage if kind==0 else stage%3,level,group,seed%4,missing)+heap
            expected += bytes([result])+dirty+wanted
    with tempfile.TemporaryDirectory(prefix='cmr2-results-native-') as directory:
        folder = Path(directory)
        for name, text in (('typed',code),('old-varargs',PRE+support+old_rows+main),
                           ('old-pointer',PRE+old_pointers+rows+main)):
            cpp, exe = folder/(name+'.cpp'), folder/name
            cpp.write_text(text)
            subprocess.run(['clang++','-include',str(ROOT/'CMR2Decomp/FloatToInt.h'),'-std=c++17','-O2','-g','-fsanitize=address,undefined',
                            '-fno-sanitize-recover=all','-fms-extensions','-Wno-everything',
                            '-I'+str(PORT/'src'),'-I'+str(PORT/'src/port/compat'),
                            '-I'+str(PORT/'game'),str(cpp),'-o',str(exe)],check=True)
            result = subprocess.run([str(exe)],input=bytes(inputs),capture_output=True)
            if name == 'typed':
                assert result.returncode==0,result.stderr.decode()
                assert result.stdout==bytes(expected),'whole heap/result/flags model'
                print(result.stderr.decode().strip())
            else:
                assert result.returncode != 0, name+' negative control unexpectedly passed'
                assert b'Sanitizer' in result.stderr,result.stderr.decode()
                print(name+': old-code negative control fails under sanitizers')


if __name__ == '__main__':
    main()
