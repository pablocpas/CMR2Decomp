#!/usr/bin/env python3
"""Actual shadow registration/emission bodies vs original x86, with ASan/UBSan.

Production ShadowPart/SceneNode/Mesh declarations and native pointers above
4 GiB. Checks every byte of the work buffers, every emitted vertex reference,
allocation sizes, immutable input and allocation guards.
"""
import os
import shutil
import struct
import subprocess
import tempfile
from pathlib import Path

from check_64bit_car_contacts import body
from differential_shadow_buffers import ShadowBuffers, cases, ROOT

PRELUDE = r'''
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#define __forceinline inline
typedef long long __int64;
#include "windows.h"
#include "Graphics.h"
#include "SceneNode.h"
#include "ShadowPart.h"
struct Block {BYTE *base;size_t size;};
std::vector<Block> blocks;
class CFileBuffer {public:static void *AllocateLockedBuffer(unsigned size){
 BYTE *p=(BYTE *)malloc(size+32);assert(p && (uintptr_t)p>UINT32_MAX);
 memset(p,0xa5,size+32);blocks.push_back({p,size});return p+16;
}};
ShadowCaster *g_shadowCasters[30];BYTE g_shadowCasterCount;
unsigned short g_shadowVertexRemap[0x3f0];int g_shadowLevel;DWORD g_shadowColour;
float g_sceneLightDirF[3]={0,0,1},g_sceneLightBasisF[9]={0,0,1,0,1,0,1,0,0};
float g_netZero=0,g_unk0x00511ce8=0.1f,g_unk0x00511360=10;
Mesh *Mesh_GetShadowCylinder(Mesh *){assert(false);return NULL;}
#define CASTS_SHADOW(n) ((n)->type==0 && (n)->pObject!=NULL && ((n)->flags&255)!=6 && ((n)->flags&255)!=15 && ((n)->flags&255)!=14)
'''
MAIN = r'''
void output(const void *p,size_t n){assert(fwrite(p,1,n,stdout)==n);}
int main(int argc,char **argv){
 assert(argc==4 && sizeof(void *)==8);int count=atoi(argv[1]),kind=atoi(argv[2]),factor=atoi(argv[3]);
 static_assert(sizeof(ShadowPart)==0x78,"native shadow part");
 static_assert(offsetof(SceneNode,worldF)!=0x118,"native node matrix");
 SceneNode nodes[4]={};Mesh meshes[4]={};MeshTriangle faces[4][2]={};
 const int indices[5][2][3]={{{0,1,2}},{{3,4,5}},{{0,1,2},{3,4,5}},{{0,1,2},{0,2,4}},{{0,0,0}}};
 int faceCount=kind==4?0:kind>=2?2:1;
 const float positions[6][3]={{-2,0,1},{0,2,1},{2,0,1},{-2,0,-1},{0,-2,-1},{2,0,-1}};
 for(int i=0;i<count;i++){
  nodes[i].pFirstChild=i+1<count?&nodes[i+1]:NULL;nodes[i].pObject=&meshes[i];nodes[i].flags=5;nodes[i].viewMask=255;
  nodes[i].worldF[0]=nodes[i].worldF[5]=nodes[i].worldF[10]=nodes[i].worldF[15]=1;
  nodes[i].worldF[12]=float(i);nodes[i].worldF[13]=float(-i);nodes[i].worldF[14]=float(i*2);
  meshes[i].field_0x10=6;meshes[i].triangleCount=faceCount;meshes[i].pTriangles=faces[i];
  for(int j=0;j<faceCount;j++)for(int k=0;k<3;k++)faces[i][j].vertexIndex[k]=indices[kind][j][k];
 }
 BYTE beforeNodes[sizeof(nodes)],beforeMeshes[sizeof(meshes)],beforeFaces[sizeof(faces)];
 memcpy(beforeNodes,nodes,sizeof(nodes));memcpy(beforeMeshes,meshes,sizeof(meshes));memcpy(beforeFaces,faces,sizeof(faces));
 Scene_AddShadowCaster(nodes,1);assert(g_shadowCasterCount==1 && g_shadowCasters[0]->partCount==count);
 assert(blocks.size()==2+size_t(count)*6 && blocks[0].size==sizeof(ShadowCaster) && blocks[1].size==count*sizeof(ShadowPart));
 g_shadowLevel=0xff0000;g_shadowColour=0xff123456;
 for(int i=0;i<count;i++){
  auto *part=&g_shadowCasters[0]->pParts[i];assert(part->pMesh==&meshes[i] && part->pNode==&nodes[i]);
  const size_t sizes[]={72,24,72,size_t(faceCount)*3*sizeof(BYTE *),288,72};
  for(int j=0;j<6;j++)assert(blocks[2+i*6+j].size==sizes[j]);
  memcpy(part->pVertexWork,positions,sizeof(positions));memcpy(part->pVertexWork2,positions,sizeof(positions));
  Scene_EmitProjectedShadowMesh(part,factor);
  output(part->light,48);output(&part->faceCount,2);output(&part->vertexCount,2);
  output(part->pVertexFlags,24);output(part->pVertexWork3,72);output(part->pVertices,288);
  for(int j=0;j<faceCount*3;j++){
   uint32_t value=j<part->faceCount*3?uint32_t((part->pTriangleWork[j]-part->pVertices)/48):uint32_t((uintptr_t)part->pTriangleWork[j]);
   output(&value,4);
  }
 }
 assert(!memcmp(beforeNodes,nodes,sizeof(nodes)) && !memcmp(beforeMeshes,meshes,sizeof(meshes)) && !memcmp(beforeFaces,faces,sizeof(faces)));
 for(auto b:blocks){for(int i=0;i<16;i++)assert(b.base[i]==0xa5 && b.base[16+b.size+i]==0xa5);free(b.base);}
}
'''


def main():
    src = ROOT / 'CMR2Decomp'
    source = (src / 'SceneNode.cpp').read_text()
    start = source.index('static __forceinline void ShadowPart_Init(')
    end = source.index('\n}', start) + 2
    functions = [source[start:end], body(source, 'Scene_AddShadowCaster'),
                 body(source, 'Scene_EmitProjectedShadowMesh')]
    fp = (ROOT.parent / 'OpenCMR2/game/FixedPoint.cpp').read_text()
    functions = [body(fp, 'FloatMatrix_RotateVector'), body(fp, 'FloatMatrix_InverseRotateVector')] + functions
    with tempfile.TemporaryDirectory(prefix='cmr2-native-shadow-buffers-') as directory:
        tmp = Path(directory)
        for header in ('FloatToInt.h','ShadowPart.h', 'SceneNode.h', 'Sector.h', 'Mesh.h'):
            shutil.copyfile(src / header, tmp / header)
        shutil.copyfile(ROOT.parent / 'OpenCMR2/game/FixedPoint.h', tmp / 'FixedPoint.h')
        (tmp / 'windows.h').write_text('typedef unsigned char BYTE;typedef unsigned int DWORD;typedef int BOOL;typedef intptr_t INT_PTR;\n')
        (tmp / 'Graphics.h').write_text('class CGraphics{public:static constexpr double m_oneOver65536=1.0/65536.0;};\n')
        cpp = tmp / 'shadow.cpp'
        driver = PRELUDE + '\n'.join(functions) + MAIN
        cpp.write_text(driver)
        exe = tmp / 'shadow'
        command = ['clang++','-include',str(ROOT/'CMR2Decomp/FloatToInt.h'), '-m64', '-std=c++17', '-O2', '-fPIE', '-pie', '-fwrapv',
                   '-ffp-contract=off', '-fno-strict-aliasing', '-I', str(tmp),
                   '-Werror=int-to-pointer-cast', '-Werror=pointer-to-int-cast',
                   '-fsanitize=address,undefined', str(cpp), '-o', str(exe)]
        subprocess.run(command, check=True)
        env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0', UBSAN_OPTIONS='halt_on_error=1')
        original = ShadowBuffers(ROOT / 'cmr2bin/CMR2.exe')
        count = 0
        for case in cases():
            expected = bytearray()
            for light, faces, vertices, flags, work, data, edges in original.run(*case):
                expected.extend(light + struct.pack('<2h', faces, vertices) + flags + work + data)
                expected.extend(struct.pack('<' + 'I' * len(edges), *(v & 0xffffffff for v in edges)))
            result = subprocess.run([str(exe), *map(str, case)], capture_output=True, env=env)
            assert not result.returncode, (case, result.stderr.decode())
            assert result.stdout == expected, (case, [(i, a, b) for i, (a, b) in enumerate(zip(result.stdout, expected)) if a != b][:12])
            count += 1
        controls = [
            ('half-size allocation', 'triangleCount * 3 * sizeof(*pPart->pTriangleWork)', 'triangleCount * 0xc'),
            ('packed four-byte slots', '(sizeof(*pEdge) - sizeof(*pi))', '0'),
            ('advanced shared-vertex origin', 'pPart->pVertices + s * 0x30', 'pVtx + s * 0x30'),
        ]
        case = (3, 3, 65536)
        expected = bytearray()
        for light, faces, vertices, flags, work, data, edges in original.run(*case):
            expected.extend(light + struct.pack('<2h', faces, vertices) + flags + work + data)
            expected.extend(struct.pack('<' + 'I' * len(edges), *(v & 0xffffffff for v in edges)))
        for name, old, new in controls:
            assert old in driver, name
            cpp.write_text(driver.replace(old, new))
            subprocess.run(command, check=True)
            result = subprocess.run([str(exe), *map(str, case)], capture_output=True, env=env)
            assert result.returncode or result.stdout != expected, ('negative control escaped', name)
    print(f'PASS: {count} native shadow registration/emission cases vs original x86; production layouts, complete guarded buffers, eight-byte vertex pointers and ASan/UBSan; three negative controls detected')


if __name__ == '__main__':
    main()
