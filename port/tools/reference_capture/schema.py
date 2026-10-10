#!/usr/bin/env python3
"""Generate a scalar comparison schema using the port's actual 32-bit layouts."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[2]
SOURCES = ['game/FixedPoint.h', 'game/Car.h', 'game/CarParts.h', 'game/CarPhysics.h', 'game/StageTiming.cpp']
BLOCKS = {'car': ('Car', 1, 0xc24), 'parts': ('CarPartSet', 1, 0x4d0),
          'damage': ('CarDamageRecord', 1, 0x290), 'contact': ('CarContact', 1, 0x2a4),
          'timing': ('CarStageTiming', 1, 0x88), 'checkpoint': ('Unk0x00542e78', 1, 0x1c),
          'transforms': ('CarTransforms', 1, 0xfc), 'shadow': ('CarTransforms', 1, 0xfc),
          'wheels': ('FixMatrix', 4, 0x100), 'world': ('FixMatrix', 1, 0x40), 'body': ('FixMatrix', 1, 0x40)}
FORMATS = {'int': 'i', 'unsigned int': 'I', 'short': 'h', 'unsigned short': 'H',
           'char': 'b', 'signed char': 'b', 'unsigned char': 'B', 'BYTE': 'B', 'DWORD': 'I', 'float': 'f'}


def generate(output):
    structs = {}
    for name in SOURCES:
        for kind, body in re.findall(r'struct\s+(\w+)\s*\{(.*?)\};', (ROOT/name).read_text(), re.S):
            structs[kind] = body
    entries, excluded, code = {}, {}, []
    def walk(kind, expression, label, fields, omit):
        for declaration in re.sub(r'//[^\n]*', '', structs[kind]).split(';'):
            declaration = declaration.strip().replace('*', '* ')
            if not declaration:
                continue
            match = re.fullmatch(r'(.+?)\s+(\w+)((?:\[[^\]]+\])*)', declaration)
            if not match:
                raise ValueError(f'unsupported field in {kind}: {declaration}')
            field_type, name, dimensions = match.groups()
            path = f'{label}.{name}' if label else name
            address = f'({expression})+offsetof({kind},{name})'
            if '*' in field_type or name.startswith('pad') or (kind == 'FixMatrix' and name in ('rw', 'uw', 'fw', 'pw')):
                omit.append({'name': path, 'reason': 'address' if '*' in field_type else 'padding/unused'})
                continue
            sizes = [int(x, 0) for x in re.findall(r'\[([^\]]+)\]', dimensions)]
            count = 1
            for size in sizes:
                count *= size
            for i in range(count):
                subpath = path + (f'[{i}]' if dimensions else '')
                subaddress = f'({address})+{i}*sizeof({field_type})'
                if field_type in FORMATS:
                    number = len(code)
                    fields.append({'name': subpath, 'format': FORMATS[field_type], 'offset_index': number})
                    code.append(f'printf("%u\\n",(unsigned)({subaddress}));')
                else:
                    walk(field_type, subaddress, subpath, fields, omit)
    assertions = []
    for block, (kind, count, size) in BLOCKS.items():
        entries[block], excluded[block] = [], []
        assertions.append(f'static_assert(sizeof({kind})*{count}=={size},"{kind} layout");')
        for i in range(count):
            walk(kind, f'{i}*sizeof({kind})', f'[{i}]' if count > 1 else '', entries[block], excluded[block])
    output.mkdir(parents=True, exist_ok=True)
    cpp = output/'state_schema.cpp'
    local_structs = '\n'.join(f'struct {name} {{{structs[name]}}};' for name in ('CarStageTiming', 'Unk0x00542e78'))
    cpp.write_text('#include "port/types.h"\n#include "Car.h"\n#include "CarParts.h"\n#include "CarPhysics.h"\n#include <cstdio>\n#include <cstddef>\n'+local_structs+'\n'+'\n'.join(assertions)+'\nint main(){\n'+'\n'.join(code)+'\n}\n')
    binary = output/'state_schema'
    subprocess.run(['clang++', '-m32', '-std=c++17', '-I'+str(ROOT/'game'), '-I'+str(ROOT/'src'), str(cpp), '-o', str(binary)], check=True)
    offsets = list(map(int, subprocess.check_output([str(binary)], text=True).splitlines()))
    for fields in entries.values():
        for field in fields:
            field['offset'] = offsets[field.pop('offset_index')]
    result = {'schema': 1, 'sizes': {b: v[2] for b,v in BLOCKS.items()}, 'fields': entries, 'excluded': excluded,
              'source_sha256': {s: hashlib.sha256((ROOT/s).read_bytes()).hexdigest() for s in SOURCES}}
    (output/'schema.json').write_text(json.dumps(result, indent=2)+'\n')
    print(f'{sum(map(len, entries.values()))} scalar fields: {output / "schema.json"}')


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('output', type=Path)
    generate(p.parse_args().output.resolve())
