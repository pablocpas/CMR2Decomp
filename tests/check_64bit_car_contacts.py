#!/usr/bin/env python3
"""Compile actual contact bodies with native 64-bit pointers and exercise them.

Uses the port's existing portable fixed-point primitives; Car, SceneNode,
StageObject, CollisionBox and the eight function bodies come from this checkout.
Graphics/mesh headers are opaque fixtures because these paths never access them.
This is a focused runtime smoke test, not a claim that the whole game ports yet.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def body(source, name):
    match = re.search(r'^(?:int|void) ' + name + r'\([^;]*?\)\s*\{', source, re.M)
    assert match, name
    start, pos, depth = match.start(), match.end(), 1
    while depth:
        depth += (source[pos] == '{') - (source[pos] == '}')
        pos += 1
    return source[start:pos]


PRELUDE = r'''
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstddef>
typedef unsigned char BYTE;
typedef long long __int64;
#include "Car.h"
#include "Sector.h"
#include "LayoutChecks.h"
#define FIX_ABS(x) ((x) < 0 ? -(x) : (x))
unsigned short g_sqrtTable[4096];
int g_physicsScale = 65536;
FixVector g_collisionPush, g_unk0x005914b8, g_unk0x005915e8, g_collisionSphereCentre;
int g_collisionSphereRadius = 16384, g_unk0x005915dc = 32768, g_unk0x00591468 = 87949;
int g_unk0x005914d8, g_unk0x005915f8[0x26];
#define g_unk0x00591628 ((FixVector *)&g_unk0x005915f8[0xc])
char g_unk0x00590ecc[4], g_unk0x005914c4[4], g_unk0x005914d4, g_unk0x005915f4;
BYTE g_unk0x00590ec8[4], g_unk0x005914a4[4];
FixVector *g_pContacts0x005915e0, *g_pContacts0x00591394;
class CGameInfo { public: static int IsActiveCheatEnabled(int v) { assert(v == 4); return 0; } };
void StageObject_ApplyRecursiveFrameDelta(BYTE, char, int *, int) {}
void CarDamage_ApplyCollisionDeformImpulse(Car *, int *, FixVector *, int, unsigned char, int) {}
void Car_SpawnDebris(int, FixVector *, Car *, FixVector *, int, int) { assert(false); }
'''

LEAVES = r'''
int Collision_SphereVsBox(int *, int *factor, unsigned int *t, int) {
    *factor = 32768; *t = 0; return 1;
}
int Collision_TestOrientedBoxCornerOverlap(CollisionBox *, CollisionBox *, FixVector *, int) {
    return 1;
}
'''

MAIN = r'''
void identity(FixMatrix &m) {
    std::memset(&m, 0, sizeof(m)); m.right.x = m.up.y = m.forward.z = 65536;
}
void car(Car &c) {
    std::memset(&c, 0, sizeof(c)); identity(c.physicsMatrix);
    c.pWorld = &c.physicsMatrix; c.mass = 65536;
    c.halfExtents.x = c.halfExtents.y = c.halfExtents.z = 65536;
    c.right.x = c.up.y = c.forward.z = 65536; c.field_0xb35[0] = 1;
    assert(reinterpret_cast<uintptr_t>(&c) > UINT32_MAX);
    assert(reinterpret_cast<uintptr_t>(c.pWorld) > UINT32_MAX);
}
int main() {
    static_assert(sizeof(void *) == 8, "native pointer width");
    static_assert(offsetof(Car, pWorld) != 0x750, "runtime Car must grow");
    static_assert(offsetof(CollisionBox, pVertex) != 0x94, "native box pointers");
    static_assert(offsetof(StageObject, field_0x10) != 0x10, "field follows native mesh pointer");
    for (int i=0;i<4096;i++) g_sqrtTable[i] = static_cast<unsigned short>(std::sqrt((8+16*i)*65536.0));
    Car a, b; CollisionBox boxA={}, boxB={}; StageObject owner={};
    FixVector centreA={}, centreB={}, verticesA[8]={}, verticesB[8]={};
    StageObject *entry = &owner;
    assert(reinterpret_cast<uintptr_t>(entry) > UINT32_MAX);
    g_unk0x005915e8.z=65536;
    car(a); car(b); a.velocity.z=65536;
    a.useUpperCollisionCorners=b.useUpperCollisionCorners=1;
    Collision_ResolveCarContactImpulse(&a,&b);
    assert(a.field_0x5c4.z==-32768 && b.field_0x5c4.z==32768);
    assert(a.velocity.z==65536 && b.velocity.z==0);
    car(a); a.velocity.z=65536; a.field_0xb64=1; a.useUpperCollisionCorners=1;
    int flags=0x1000; std::memcpy(owner.field_0x10,&flags,sizeof(flags));
    assert(Collision_ResolveStaticObstacleContact(&a,&entry,nullptr,9)==1);
    assert(a.field_0x5c4.z==-655 && a.field_0xb35[10]==1);
    short contact; std::memcpy(&contact,a.field_0xad6+10,sizeof(contact)); assert(contact==9);
    car(a); boxA.axisA.x=65536; boxA.axisB.z=65536;
    boxA.pVertex=reinterpret_cast<int *>(&centreA); boxA.pArray=reinterpret_cast<int *>(verticesA);
    assert(Collision_CarVsBox(&a,reinterpret_cast<int *>(&boxA),32768)==1);
    assert(a.field_0x5dc.x==32768 && a.field_0x5dc.y==0 && a.field_0x5dc.z==0);
    car(a); car(b); boxB.axisA.x=65536; boxB.axisB.z=65536;
    boxB.pVertex=reinterpret_cast<int *>(&centreB); boxB.pArray=reinterpret_cast<int *>(verticesB);
    centreA.x=3*65536; centreB.x=5*65536; b.position.x=2*65536;
    a.corners[0].x=0; b.corners[0].x=2*65536;
    g_pContacts0x005915e0=reinterpret_cast<FixVector *>(&boxA);
    g_pContacts0x00591394=reinterpret_cast<FixVector *>(&boxB);
    g_unk0x005915f4=g_unk0x005914d4=1;
    assert(Collision_SeparateCarBoxes(&a,&b)==1);
    assert(a.field_0x5dc.x==65536 && b.field_0x5dc.x==-65536);
    assert(a.field_0x5dc.y==0 && b.field_0x5dc.y==0);
    return 0;
}
'''


def run(command, expected=0):
    # LeakSanitizer cannot inspect threads under the sandbox's ptrace. All
    # fixture records are automatic objects; keep ASan/UBSan enabled.
    result = subprocess.run(command, capture_output=True, text=True,
                            env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0',
                                     UBSAN_OPTIONS='halt_on_error=1'))
    if result.returncode != expected:
        raise RuntimeError(result.stdout + result.stderr)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port-root', type=Path, default=ROOT.parent / 'OpenCMR2')
    parser.add_argument('--compiler', default='clang++')
    parser.add_argument('--baseline-commit', help='also audit casts against this local Git commit')
    parser.add_argument('--audit-json', type=Path)
    args = parser.parse_args()
    source = ROOT / 'CMR2Decomp'
    with tempfile.TemporaryDirectory(prefix='cmr2-native-car-contacts-') as directory:
        tmp = Path(directory)
        for name in ('Car.h', 'SceneNode.h', 'Sector.h', 'LayoutChecks.h'):
            shutil.copyfile(source / name, tmp / name)
        shutil.copyfile(args.port_root / 'game/FixedPoint.h', tmp / 'FixedPoint.h')
        (tmp / 'Graphics.h').write_text('struct Graphics;\nstruct Texture;\nclass CGraphics { public: static const double m_oneOver65536; };\n')
        (tmp / 'Mesh.h').write_text('struct Mesh;\n')
        probe = tmp / 'layout.cpp'
        probe.write_text('#include "LayoutChecks.h"\nCMR2_LAYOUT_CHECK(DeliberateFailure, sizeof(int) == 5);\n')
        run([args.compiler, '-m64', '-fsyntax-only', str(probe)])
        failed = subprocess.run([args.compiler, '-m32', '-fsyntax-only', str(probe)], capture_output=True, text=True)
        assert failed.returncode and 'negative' in failed.stderr, '32-bit layout checks must stay active'
        collision = (source / 'Collision2D.cpp').read_text()
        fixed = (source / 'FixedPoint.cpp').read_text()
        record = re.search(r'struct CollisionBox \{.*?\n\};', collision, re.S).group()
        functions = ('Collision_ClampContactOffset', 'Collision_ClampAngularCorrection',
                     'Collision_CarVsBox', 'Collision_SeparateCarBoxes',
                     'Collision_ResolveCarContactImpulse', 'Collision_ResolveStaticObstacleContact')
        if args.baseline_commit:
            previous = subprocess.run(['git', 'show', args.baseline_commit + ':CMR2Decomp/Collision2D.cpp'],
                                      cwd=ROOT, capture_output=True, text=True, check=True).stdout
            audit = {'baseline_commit': args.baseline_commit,
                     'compiler': run([args.compiler, '--version']).stdout.splitlines()[0]}
            for label, text in (('baseline', previous), ('current', collision)):
                chunks = [PRELUDE, record, LEAVES]
                ranges = {}
                for name in functions:
                    first = '\n'.join(chunks).count('\n') + 2
                    chunk = body(text, name); chunks.append(chunk)
                    ranges[name] = (first, first + chunk.count('\n'))
                code = '\n'.join(chunks)
                fixture = tmp / (label + '-audit.cpp'); fixture.write_text(code)
                result = run([args.compiler, '-m64', '-std=c++17', '-fsyntax-only',
                              '-Wno-everything', '-Wint-to-pointer-cast',
                              '-Wpointer-to-int-cast', str(fixture)])
                diagnostics = []
                for match in re.finditer(r':(\d+):\d+: warning: ([^\n]+)', result.stderr):
                    line = int(match[1])
                    name = next((name for name, (lo, hi) in ranges.items() if lo <= line <= hi), None)
                    assert name, ('unexpected warning outside selected bodies', match[0])
                    diagnostics.append({'function': name, 'diagnostic': match[2]})
                audit[label] = {'body_source_sha256': hashlib.sha256(text.encode()).hexdigest(),
                                'warnings': len(diagnostics), 'errors': 0,
                                'by_function': {name: sum(d['function'] == name for d in diagnostics)
                                                for name in functions}, 'diagnostics': diagnostics}
            assert audit['current']['warnings'] == 0
            if args.audit_json:
                args.audit_json.write_text(json.dumps(audit, indent=2) + '\n')
            print('Native cast warnings:', audit['baseline']['warnings'], '->', audit['current']['warnings'])
        code = PRELUDE + record + LEAVES
        code += '\n'.join(body(fixed, name) for name in ('FixMatrix_RotateVector', 'FixMatrix_InverseRotateVector'))
        code += '\n'.join(body(collision, name) for name in functions) + MAIN
        fixture = tmp / 'contacts.cpp'; fixture.write_text(code)
        executable = tmp / 'contacts'
        run([args.compiler, '-m64', '-std=c++17', '-O2', '-fwrapv',
             '-fno-strict-aliasing', '-fsanitize=address,undefined',
             '-fno-sanitize=alignment', str(fixture), '-o', str(executable)])
        run([str(executable)])
    print('PASS: original-layout checks active on x86, inactive on x64; four actual contact bodies with pointers above 4 GB, ASan/UBSan')


if __name__ == '__main__':
    main()
