#!/usr/bin/env python3
"""Actual fixed WAV loader and header on x64, with controlled audio/file leaves.

Compare every case with the original x86, including callback order and pointer
table guards. The fixture does not implement DirectSound or a general RIFF reader.
"""
import os
from pathlib import Path
import subprocess
import tempfile

from check_64bit_address_loops import record
from check_64bit_car_part_tables import body
from differential_wave_loading import WaveLoading, cases, ROOT, STATES, model

PRELUDE = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <vector>
using BYTE=unsigned char;using WORD=uint16_t;using DWORD=uint32_t;using BOOL=int;
using HRESULT=int32_t;using LPVOID=void *;
#define TRUE 1
#define FALSE 0
constexpr DWORD DS3DMODE_NORMAL=0,DS3D_DEFERRED=1;
int state,slot;BYTE archive;bool missing;
std::vector<std::vector<uint32_t>> trace;
void event(std::initializer_list<uint32_t> values){trace.emplace_back(values);}
template<class T>void high(T *p){assert((uintptr_t)p>UINT32_MAX);}
alignas(16) BYTE wave[256],fileBytes[64];
char name[]="fixture.wav",g_strWave[8]="WAVE";
struct GenericFile{};
struct IDirectSound{};IDirectSound device,*g_unk0x005a2844=&device;
BOOL g_sound3DEnabled;int IID_IDirectSound3DBuffer;
struct IDirectSound3DBuffer {
 HRESULT SetMode(DWORD mode,DWORD deferred){high(this);event({6,(uint32_t)slot,mode,deferred});return state==5?(HRESULT)0x80004005:0;}
} spatial;
IDirectSound3DBuffer *g_sound3DBuffers[200];
struct IDirectSoundBuffer {
 HRESULT QueryInterface(const int &iid,LPVOID *out){
  high(this);high(out);assert(&iid==&IID_IDirectSound3DBuffer && out==(LPVOID *)&g_sound3DBuffers[slot]);
  event({4,(uint32_t)slot});if(state==3)return (HRESULT)0x80004002;
  *out=&spatial;return state==7?7:0;
 }
} buffer;
IDirectSoundBuffer *g_soundBuffers[200];
struct CSound{static BOOL IsSoundCallSuccessful(HRESULT result);};
struct CGenericFileLoader {
 static void *FindFile(GenericFile *file,char *path,BYTE *found,DWORD *id,BYTE skip){
  high(file);high(path);high(found);assert(file==(GenericFile *)fileBytes && path==name && !id && !skip);
  event({1});*found=archive;return missing?nullptr:wave;
 }
};
struct CFileBuffer{static void FreeGenericFileBuffer(void *p){high(p);assert(p==wave);event({7});}};
int Sound_GetLoadedSampleCount(){event({2,(uint32_t)slot});return slot;}
BOOL Sound_CreatePcmSampleBuffer(IDirectSound *ds,IDirectSoundBuffer **out,DWORD rate,int bits,int channels,int is3D,DWORD size);
BOOL Sound_CopyBufferData(IDirectSoundBuffer *p,DWORD offset,void *data,DWORD size){
 high(p);high(data);assert(p==&buffer);event({5,(uint32_t)slot,offset,(uint32_t)((BYTE *)data-wave),size});
 return state==4?0:state==7?-1:1;
}
'''
CREATE = r'''
BOOL Sound_CreatePcmSampleBuffer(IDirectSound *ds,IDirectSoundBuffer **out,DWORD rate,int bits,int channels,int is3D,DWORD size){
 high(ds);high(out);assert(ds==&device && out==&g_soundBuffers[slot]);
 event({3,(uint32_t)slot,rate,(uint32_t)bits,(uint32_t)channels,(uint32_t)is3D,size});
 if(state==2)return 0;*out=&buffer;
 if(state==8)((PcmWaveHeader *)wave)->dataSize=size+7;
 return state==7?-1:1;
}
'''
MAIN = r'''
int main(int argc,char **argv){
 assert(argc==6 && sizeof(void *)==8);high(wave);high(g_soundBuffers);high(g_sound3DBuffers);
 static_assert(sizeof(PcmWaveHeader)==44,"serialized header stays fixed on x64");
 static_assert(offsetof(PcmWaveHeader,waveId)==8 && offsetof(PcmWaveHeader,channels)==22 &&
  offsetof(PcmWaveHeader,sampleRate)==24 && offsetof(PcmWaveHeader,bitsPerSample)==34 &&
  offsetof(PcmWaveHeader,dataSize)==40,"loader field offsets");
 unsigned flags=(unsigned)atoi(argv[1]);g_sound3DEnabled=atoi(argv[2]);archive=(BYTE)atoi(argv[3]);state=atoi(argv[4]);slot=atoi(argv[5]);missing=state==0;
 assert(fread(wave,1,sizeof(wave),stdin)==sizeof(wave));BYTE before[sizeof(wave)];memcpy(before,wave,sizeof(wave));
 IDirectSoundBuffer *buffersBefore[200];IDirectSound3DBuffer *spatialsBefore[200];
 for(int i=0;i<200;i++){
  buffersBefore[i]=g_soundBuffers[i]=(IDirectSoundBuffer *)(fileBytes+i%32);
  spatialsBefore[i]=g_sound3DBuffers[i]=(IDirectSound3DBuffer *)(fileBytes+i%32);
 }
 int result=Sound_LoadWave(name,(BYTE)flags,(GenericFile *)fileBytes);
 bool created=false,queried=false;
 for(const auto &e:trace){if(e[0]==3 && state!=2)created=true;if(e[0]==4 && state!=3)queried=true;}
 if(created)buffersBefore[slot]=&buffer;if(queried)spatialsBefore[slot]=&spatial;
 if(created && state==8){uint32_t size;memcpy(&size,before+40,4);size+=7;memcpy(before+40,&size,4);}
 assert(!memcmp(before,wave,sizeof(wave)) && !memcmp(buffersBefore,g_soundBuffers,sizeof(buffersBefore)) &&
  !memcmp(spatialsBefore,g_sound3DBuffers,sizeof(spatialsBefore)));
 printf("%d\n",result);for(const auto &e:trace){for(auto v:e)printf("%u ",v);puts("");}
}
'''


def main():
    source = (ROOT/'CMR2Decomp/Sound.cpp').read_text()
    header = (ROOT/'CMR2Decomp/PcmWaveHeader.h').read_text()
    oracle = WaveLoading(ROOT/'cmr2bin/CMR2.exe')
    with tempfile.TemporaryDirectory(prefix='cmr2-native-wave-') as directory:
        tmp=Path(directory)
        cpp=tmp/'wave.cpp';exe=tmp/'wave'
        cpp.write_text(PRELUDE+record(header,'PcmWaveHeader')+CREATE+
                       body(source,'CSound::IsSoundCallSuccessful')+body(source,'Sound_LoadWave')+MAIN)
        subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing',
                        '-Werror=int-to-pointer-cast','-Werror=pointer-to-int-cast','-fsanitize=address,undefined',str(cpp),'-o',str(exe)],check=True)
        for i,case in enumerate(cases()):
            result,trace = oracle.run_case(case,51003+i)
            recipe,flags,enabled,archive,state = case
            process=subprocess.run([str(exe),str(flags),str(enabled),str(archive),str(STATES.index(state)),str(oracle.slot)],
                input=oracle.native_input,capture_output=True,
                env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
            assert process.returncode==0,(case,process.stderr.decode())
            lines=process.stdout.decode().splitlines()
            actual=int(lines[0]),[tuple(map(int,line.split())) for line in lines[1:]]
            assert actual==(result,trace)==model(case),(case,actual,(result,trace))
    print(f'PASS: {len(cases())} native WAV loader cases; actual production body/header, pointers >4 GB, '
          'native pointer tables, format widths, ownership/failures, provider order, data-size reload and file guards; ASan/UBSan')


if __name__=='__main__':
    main()
