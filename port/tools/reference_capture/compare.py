#!/usr/bin/env python3
"""Compare complete CMRSTAT1 recordings by scalar field, retaining raw files."""
import argparse
from collections import Counter
import hashlib
import gzip
import json
from pathlib import Path
import struct
import sys

MAGIC = b'CMRSTAT1'
MAX_BLOCK = 1024 * 1024


def read_exact(file, count):
    data = file.read(count)
    if len(data) != count:
        raise ValueError('truncated capture (no complete recording)')
    return data


def word(file):
    return struct.unpack('<I', read_exact(file, 4))[0]


def ticks(path, schema):
    with (gzip.open(path, 'rb') if path.suffix == '.gz' else path.open('rb')) as file:
        if read_exact(file, 8) != MAGIC:
            raise ValueError('unsupported capture header')
        count = 0
        while True:
            marker = word(file)
            if marker == 0x444f4e45:
                if word(file) != count or not count or file.read(1):
                    raise ValueError('invalid capture footer/count/trailing data')
                return
            if marker != 0x5449434b:
                raise ValueError('invalid tick marker')
            tick, state, rng, time, driving = struct.unpack('<5I', read_exact(file, 20))
            if tick != count:
                raise ValueError('non-sequential tick index')
            values = {'tick': tick, 'state': state, 'rng': rng, 'time': time, 'driving_tick': driving}
            blocks = {}
            while True:
                length = word(file)
                if not length:
                    break
                if length > 64:
                    raise ValueError('invalid block name length')
                name = read_exact(file, length).decode('ascii')
                size = word(file)
                if name in blocks or size > MAX_BLOCK:
                    raise ValueError('duplicate or oversized block')
                blocks[name] = read_exact(file, size)
            for required, size in [('input', 52), ('physics', 12)]:
                if len(blocks.get(required, b'')) != size:
                    raise ValueError(f'missing/invalid {required}')
            order = blocks.get('order', b'')
            if not 2 <= len(order) <= 16 or len(order) % 2:
                raise ValueError('invalid car order')
            ids = struct.unpack('<' + 'h'*(len(order)//2), order)
            if len(set(ids)) != len(ids) or any(i < 0 or i >= 8 for i in ids):
                raise ValueError('invalid car indices')
            values['order'] = list(ids)
            for index, name in enumerate(['tick', 'state', 'left', 'right', 'throttle', 'brake', 'handbrake', 'steering', 'pedal', 'shift0', 'shift1', 'shift2', 'shift3']):
                values['input.' + name] = struct.unpack_from('<i', blocks['input'], index*4)[0]
            if values['input.tick'] != tick or values['input.state'] != state:
                raise ValueError('input/tick boundary mismatch')
            for index, name in enumerate(['step', 'scale', 'stage_clock']):
                values['physics.' + name] = struct.unpack_from('<i', blocks['physics'], index*4)[0]
            expected = {'input', 'physics', 'order'}
            for car in ids:
                for kind, size in schema['sizes'].items():
                    name = f'{kind}.{car}'
                    expected.add(name)
                    if name not in blocks:
                        raise ValueError(f'missing block {name}')
                    data = blocks[name]
                    if not data:
                        values[name + '.present'] = False
                        continue
                    if len(data) != size:
                        raise ValueError(f'invalid size {name}: {len(data)} != {size}')
                    values[name + '.present'] = True
                    for field in schema['fields'][kind]:
                        fmt = field['format']
                        # Exact float bits compare consistently, including NaNs.
                        value = struct.unpack_from('<' + ('I' if fmt == 'f' else fmt), data, field['offset'])[0]
                        values[name + '.' + field['name']] = value
            if blocks.keys() != expected:
                raise ValueError('unrecognized capture block')
            yield values
            count += 1


def fingerprint(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def compare(reference, candidate, schema):
    left, right = list(ticks(reference, schema)), list(ticks(candidate, schema))
    differences = {}
    groups = Counter()
    for a, b in zip(left, right):
        for name in sorted(a.keys() | b.keys()):
            av, bv = a.get(name), b.get(name)
            if av == bv:
                continue
            if name not in differences:
                differences[name] = {'first_tick': a['tick'], 'reference': av, 'candidate': bv, 'ticks_different': 0}
            differences[name]['ticks_different'] += 1
            differences[name]['last_tick'] = a['tick']
            groups[name.split('.')[0]] += 1
    identical = len(left) == len(right) and not differences
    first = min((d['first_tick'] for d in differences.values()), default=None)
    return {'schema': 1, 'identical': identical, 'reference_sha256': fingerprint(reference),
            'candidate_sha256': fingerprint(candidate), 'reference_ticks': len(left), 'candidate_ticks': len(right),
            'first_difference_tick': first, 'different_fields': len(differences), 'different_values_by_block': dict(groups),
            'differences': differences}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('reference', type=Path)
    p.add_argument('candidate', type=Path)
    p.add_argument('--schema', required=True, type=Path)
    p.add_argument('--output', required=True, type=Path)
    args = p.parse_args()
    if args.output.exists():
        raise ValueError('report already exists; choose a new output')
    schema = json.loads(args.schema.read_text())
    result = compare(args.reference, args.candidate, schema)
    result['schema_sha256'] = fingerprint(args.schema)
    args.output.write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k != 'differences'}, indent=2))
    return 0 if result['identical'] else 1


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, struct.error) as error:
        print(f'comparison failed: {error}', file=sys.stderr)
        sys.exit(2)
