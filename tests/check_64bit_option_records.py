#!/usr/bin/env python3
"""Exercise actual menu texture loading and numeric getters with native pointers.

Texture/GenericFile definitions and all five bodies are read from this checkout.
Only Win32/DDraw declarations and archive/texture/selection providers are fixtures.
The full menus still contain legacy address APIs; this checks the completed paths.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from check_64bit_car_contacts import ROOT, body


PRELUDE = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include "Texture.h"
class CFrontend {
public:
    static char m_stringDest[260], m_strUK[3];
    static int GetArchivePrimaryFlagEntry(int);
    static int GetArchiveSecondaryFlagEntry(int);
    static int GetArchivePrimaryIDEntry(int);
    static int GetArchiveSecondaryIDEntry(int);
};
char CFrontend::m_stringDest[260], CFrontend::m_strUK[3]="UK";
class CInstallInfo { public:
    static const char *GetSetupRepDir() { return "Setup"; }
    static const char *GetFrontendDir() { return "Front"; }
};
int country, loads, nullResidue;
unsigned int RallyDataCountryIndex() { return country; }
GenericFile commonFile={}, stageFile={};
int *OptionMenu_GetCommonArchive() { return reinterpret_cast<int *>(&commonFile); }
int *OptionMenu_GetStageArchive() { return reinterpret_cast<int *>(&stageFile); }
Texture *textures;
Texture *CTexture::FindLoadTexture(GenericFile *archive, char *name, bool *loaded,
                                 LPVOID extra, bool flag, unsigned int flags) {
    assert(loads<22 && archive==(loads==3?&stageFile:&commonFile));
    assert(!loaded && !extra && !flag && !flags && std::strstr(name,".tga"));
    Texture *result=loads%3==nullResidue?nullptr:&textures[loads];
    if(result) assert(reinterpret_cast<uintptr_t>(result)>UINT32_MAX);
    ++loads;return result;
}
'''

MAIN = r'''
int main() {
    Texture nativeTextures[22]={}; textures=nativeTextures;
    static_assert(sizeof(void *)==8,"native pointers");
    static_assert(offsetof(Texture,width)!=0x120,"width follows native surface pointer");
    static_assert(std::is_same<decltype(CFrontend::GetArchivePrimaryIDEntry(0)),int>::value,"numeric ID return");
    static_assert(std::is_same<decltype(CFrontend::GetArchiveSecondaryIDEntry(0)),int>::value,"numeric ID return");
    static_assert(std::is_same<decltype(CFrontend::GetArchivePrimaryFlagEntry(0)),int>::value,"numeric flag return");
    static_assert(std::is_same<decltype(CFrontend::GetArchiveSecondaryFlagEntry(0)),int>::value,"numeric flag return");
    for(int i=0;i<28;i++) {
        g_unk0x00516b40.ids[i]=static_cast<int>(0x81000000u+i*0x10101u);
        g_unk0x00516b40.flags[i]=static_cast<int>(0x91000000u+i*0x11101u);
    }
    for(int i=0;i<28;i++) {
        assert(CFrontend::GetArchivePrimaryIDEntry(i)==g_unk0x00516b40.ids[i]);
        assert(CFrontend::GetArchivePrimaryFlagEntry(i)==g_unk0x00516b40.flags[i]);
    }
    for(int i=0;i<6;i++) assert(CFrontend::GetArchiveSecondaryIDEntry(i)==g_unk0x00516b40.ids[i+22]);
    for(int i=0;i<14;i++) assert(CFrontend::GetArchiveSecondaryFlagEntry(i)==g_unk0x00516b40.flags[i+14]);
    for(country=0;country<8;country++) for(nullResidue=-1;nullResidue<3;nullResidue++) {
        loads=0;OptionMenu_LoadPartTextures();assert(loads==22);
        Texture *values[22]={g_unk0x00831360,g_unk0x00831364,g_unk0x00831368,g_unk0x00831668,
                            g_unk0x0083166c,g_unk0x00831670,g_unk0x008313ac,g_unk0x00831648,
                            g_unk0x008313b0,g_unk0x00831674};
        for(int i=0;i<12;i++) values[i+10]=g_unk0x0083137c[i];
        for(int i=0;i<22;i++) assert(values[i]==(i%3==nullResidue?nullptr:&textures[i]));
    }
    return 0;
}
'''


def main():
    source = ROOT / 'CMR2Decomp'
    game = (source / 'GameInfo.cpp').read_text()
    frontend = (source / 'Frontend.cpp').read_text()
    load = body(game, 'OptionMenu_LoadPartTextures')
    strings = sorted(set(re.findall(r'\bg_str0x\w+', load)))
    definitions = []
    sources = [p.read_text(encoding='latin1') for p in source.glob('*.cpp')]
    for name in strings:
        pattern = r'\bchar ' + name + r'\[[^\]]*\]\s*=\s*"(?:\\.|[^"\\])*";'
        matches = [m[0] for text in sources for m in re.finditer(pattern, text)]
        assert len(matches) == 1, (name, matches)
        definitions += matches
    definitions += re.findall(r'^Texture \*g_unk0x00831\w+(?:\[12\])?;', game, re.M)
    assert len(definitions) == len(strings) + 11
    record = re.search(r'struct Unk0x00516b40 \{.*?\n\};', frontend, re.S).group()
    getters = ('GetArchivePrimaryFlagEntry', 'GetArchiveSecondaryFlagEntry',
               'GetArchivePrimaryIDEntry', 'GetArchiveSecondaryIDEntry')
    code = PRELUDE + '\n'.join(definitions) + record + '\nUnk0x00516b40 g_unk0x00516b40;\n'
    code += '\n'.join(body(frontend, 'CFrontend::' + name) for name in getters) + load + MAIN
    with tempfile.TemporaryDirectory(prefix='cmr2-native-option-records-') as directory:
        tmp = Path(directory)
        for name in ('Texture.h', 'GenericFileLoader.h'):
            shutil.copyfile(source / name, tmp / name)
        (tmp / 'windows.h').write_text('typedef unsigned char BYTE; typedef unsigned short USHORT,WORD; typedef unsigned int DWORD; typedef int BOOL; typedef void *LPVOID;\n')
        # Texture.h's relative SDK include resolves inside the fixture directory.
        include = tmp / 'game'; include.mkdir()
        shutil.move(tmp / 'Texture.h', include / 'Texture.h')
        shutil.copyfile(source / 'GenericFileLoader.h', include / 'GenericFileLoader.h')
        sdk = tmp / 'third_party/dx7sdk-7001/include'; sdk.mkdir(parents=True)
        (sdk / 'ddraw.h').write_text('#include <windows.h>\nstruct IDirectDrawSurface7; struct DDSURFACEDESC2 { DWORD words[31]; };\n')
        fixture = tmp / 'options.cpp'; fixture.write_text(code)
        executable = tmp / 'options'
        command = ['clang++', '-m64', '-std=c++17', '-O2', '-fwrapv', '-fno-strict-aliasing',
                   '-I'+str(tmp), '-I'+str(include), '-fsanitize=address,undefined',
                   str(fixture), '-o', str(executable)]
        subprocess.run(command, check=True)
        subprocess.run([str(executable)], check=True,
                       env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1'))
    print('PASS: five actual option bodies on x64, 32 loading scenarios / 704 stores, non-null pointers above 4 GB; 76 numeric getter cases, ASan/UBSan')


if __name__ == '__main__':
    main()
