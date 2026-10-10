#!/usr/bin/env python3
"""Execute actual network wrappers with 64-bit buffers and controlled COM leaves."""
import os,re,subprocess,tempfile
from pathlib import Path
from check_64bit_car_contacts import ROOT,body
PRELUDE=r'''
#include <cassert>
#include <initializer_list>
#include <cstdint>
#include <cstdio>
#include <cstring>
using BYTE=unsigned char;using DWORD=uint32_t;using DPID=DWORD;using HRESULT=int32_t;using BOOL=int;
#define __stdcall
#define TRUE 1
#define FALSE 0
constexpr HRESULT hr(unsigned x){return (HRESULT)x;}
const HRESULT DPERR_INVALIDOBJECT=hr(0x88770082),DPERR_GENERIC=hr(0x80004005),DPERR_INVALIDPARAMS=hr(0x80070057),DPERR_BUFFERTOOSMALL=hr(0x8877001e),DPERR_INVALIDPLAYER=hr(0x88770096),DPERR_NOMESSAGES=hr(0x887700be);
const DWORD DPRECEIVE_ALL=1;
struct DPNAME {DWORD dwSize,dwFlags;char *lpszShortNameA,*lpszLongNameA;};
DPNAME g_networkPlayerName;
struct IDirectPlay4A {void **table;HRESULT Receive(DWORD*,DWORD*,DWORD,void*,DWORD*);HRESULT CreatePlayer(DWORD*,DPNAME*,void*,void*,DWORD,DWORD);};
IDirectPlay4A object;void *table[32];bool present;HRESULT status;unsigned calls,frees,allocs;void *payload;unsigned length,guaranteed;unsigned receiverWrites;
alignas(16) BYTE oldData[512],newData[512];bool oldPresent,allocSuccess;
struct CGame {static DWORD m_localPlayerId,m_unk0x005a1fbc,m_unk0x005a1fc0;static void *m_unk0x005a1fb8;static IDirectPlay4A *GetDirectPlay(){return present?&object:nullptr;}};
DWORD CGame::m_localPlayerId=0x87654321,CGame::m_unk0x005a1fbc,CGame::m_unk0x005a1fc0;void *CGame::m_unk0x005a1fb8;
struct CFileBuffer {static void FreeGenericFileBuffer(void *p){assert(p==oldData && (uintptr_t)p>UINT32_MAX);++frees;}static void *AllocateLockedBuffer(DWORD size){assert(size==length);++allocs;return allocSuccess?newData:nullptr;}};
HRESULT send(void *self,DPID from,DPID to,DWORD flags,void *p,DWORD size){assert(self==&object && from==0x87654321 && to==0xfedcba98 && flags==(guaranteed==1) && p==payload && size==length && (uintptr_t)p>UINT32_MAX);++calls;return status;}
HRESULT setData(void *self,DPID player,void *p,DWORD size,DWORD flags){assert(self==&object && player==0x87654321 && flags==2 && p==payload && size==length && (uintptr_t)p>UINT32_MAX);++calls;return status;}
HRESULT IDirectPlay4A::Receive(DWORD *sender,DWORD *receiver,DWORD flags,void *p,DWORD *size){assert(this==&object && flags==1 && p==(oldPresent?oldData:nullptr) && *size==19 && (uintptr_t)sender>UINT32_MAX);*sender=0x81234567;*receiver=0x91234567;*size=length;if(!status && p)memset(p,0x17,8);++calls;return status;}
char firstName[32],lastName[32];
HRESULT IDirectPlay4A::CreatePlayer(DWORD *id,DPNAME *name,void *event,void *data,DWORD size,DWORD flags){assert(this==&object && id==&CGame::m_localPlayerId && name==&g_networkPlayerName && !event && data==payload && size==length && !flags);assert(name->dwSize==sizeof(DPNAME) && !name->dwFlags && name->lpszShortNameA==firstName && name->lpszLongNameA==lastName && (uintptr_t)firstName>UINT32_MAX);++calls;return status;}
using DPSendFn=HRESULT(*)(void*,DPID,DPID,DWORD,void*,DWORD);
using DPSetPlayerDataFn=HRESULT(*)(void*,DPID,void*,DWORD,DWORD);
'''
MAIN=r'''
int main(){
 static_assert(sizeof(void*)==8,"native network pointers");object.table=table;table[26]=(void*)send;table[29]=(void*)setData;
 assert((uintptr_t)&object>UINT32_MAX && (uintptr_t)newData>UINT32_MAX);
 unsigned statuses[]={0,1,0x88770082,0x88770096,0x88770168,0x8877010e,0x88770816,0x80004005,0x7fffffff,0x8877001e,0x80070057,0x887700be};unsigned n=0;
 for(unsigned st:statuses)for(unsigned yes=0;yes<2;yes++)for(unsigned g=0;g<4;g++)for(unsigned bytes:{0u,1u,16u,257u}){
  status=hr(st);present=yes;guaranteed=g;length=bytes;payload=oldData;memset(oldData,0xd3,sizeof(oldData));memset(newData,0xc7,sizeof(newData));BYTE before[512];memcpy(before,oldData,512);
  for(unsigned kind=0;kind<2;kind++){
   calls=0;char result=kind?Network_SetLocalPlayerData(payload,length):Network_SendPlayerMessage((int)0xfedcba98,guaranteed,payload,length);
   assert(result==(present && !status) && calls==yes && !memcmp(oldData,before,512));++n;
  }
  calls=0;CGame::m_unk0x005a1fc0=0x12345678;int created=Network_CreateLocalPlayer(firstName,lastName,payload,length);
  assert(created==(present && !status) && calls==yes && CGame::m_unk0x005a1fc0==(created?1:0x12345678));
  assert(g_networkPlayerName.dwSize==sizeof(DPNAME) && g_networkPlayerName.lpszShortNameA==firstName && g_networkPlayerName.lpszLongNameA==lastName);++n;
  oldPresent=g&1;allocSuccess=g&2;CGame::m_unk0x005a1fb8=oldPresent?oldData:nullptr;CGame::m_unk0x005a1fbc=19;
  void *oldBuffer=CGame::m_unk0x005a1fb8;void *out=(void*)firstName;DWORD sender=0x12345678;calls=frees=allocs=0;
  int received=Network_PollReceivedMessageBuffer(&sender,&out);
  assert(received==(present && !status) && calls==yes && sender==(present?0x81234567:0x12345678));
  bool resize=present && status==DPERR_BUFFERTOOSMALL;
  assert(frees==(resize && oldPresent) && allocs==resize);
  assert(out==(received?oldBuffer:(void*)firstName));
  assert(CGame::m_unk0x005a1fb8==(resize?(allocSuccess?(void*)newData:nullptr):oldBuffer));
  assert(CGame::m_unk0x005a1fbc==(resize && allocSuccess?length:19));
  if(received && oldPresent)memset(before,0x17,8);
  assert(!memcmp(oldData,before,512));for(BYTE b:newData)assert(b==0xc7);++n;
 }
 printf("PASS: %u actual native Send/SetLocal/CreatePlayer/Receive scenarios; >4 GB payload/name/output/buffer pointers, COM argument identities, null/HRESULT/growth/allocation paths and full buffer guards, ASan/UBSan; COM and allocation leaves controlled\n",n);
}
'''
def native_body(text,name):
 match=re.search(r"^(?:char|int) "+name+r"\([^;]*?\)\s*\{",text,re.M);assert match,name
 start,pos,depth=match.start(),match.end(),1
 while depth:
  depth+=(text[pos]=='{')-(text[pos]=='}');pos+=1
 return text[start:pos]

def main():
 text=(ROOT/'CMR2Decomp/Game.cpp').read_text(encoding='latin1');code=PRELUDE
 for name in ('Network_SendPlayerMessage','Network_SetLocalPlayerData','Network_CreateLocalPlayer','Network_PollReceivedMessageBuffer'):code+=native_body(text,name)
 code+=MAIN
 with tempfile.TemporaryDirectory(prefix='cmr2-native-network-') as d:
  p=Path(d);(p/'network.cpp').write_text(code)
  subprocess.run(['clang++','-m64','-std=c++17','-O2','-fPIE','-pie','-fwrapv','-fno-strict-aliasing','-fsanitize=address,undefined',str(p/'network.cpp'),'-o',str(p/'network')],check=True)
  r=subprocess.run([str(p/'network')],capture_output=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
  assert r.returncode==0,r.stderr.decode();print(r.stdout.decode(),end='')
if __name__=='__main__':main()
