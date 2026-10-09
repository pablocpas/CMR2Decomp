#!/usr/bin/env python3
"""Inventory remaining raw layouts and unnamed fields without changing source.

    python3 scripts/audit_readability.py
    python3 scripts/audit_readability.py --json docs/structure-recovery/inventory.json

This is a syntactic work list, not a proof that an access is wrong or that a
field's meaning is known. Comments and string literals are excluded. Stable
function addresses, source locations and hashes make findings traceable.
"""
import argparse
import bisect
import hashlib
import json
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[1]
LEXICAL = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
FUNCTION = re.compile(r'//\s*FUNCTION:\s*CMR2\s+(0x[0-9a-fA-F]+)')
UNKNOWN = re.compile(r'\b(?:Unk\w*|(?:[gm]_)?unk(?:0x|_)\w*|m_unk\w*|unknown\w*)\b')
FIELD = re.compile(r'\b(?:field\w*0x[0-9a-fA-F]+|unknown\w*|unk(?:0x|_)\w*)\b')
TYPE = re.compile(r'\b(struct|class|union)\s+([A-Za-z_]\w*)\s*(?::[^;{]+)?\s*\{')
DEREFERENCE = re.compile(r'\*\s*\(\s*([\w: ]+?\s*\*+)\s*\)\s*\(')
CALL = re.compile(r'\b(memcpy|memset|memmove|malloc|calloc|AllocateLockedBuffer)\s*\(')
STRIDE = re.compile(r'\*\s*(0x[0-9a-fA-F]+)\b')


def mask_noncode(source):
    return LEXICAL.sub(lambda m: re.sub(r'[^\n]', ' ', m[0]), source)


def closing(code, start, opening='(', close=')'):
    depth = 0
    for i in range(start, len(code)):
        if code[i] == opening:
            depth += 1
        elif code[i] == close:
            depth -= 1
            if depth == 0:
                return i
    return None


def arguments(code):
    parts, start, depth = [], 0, 0
    for i, c in enumerate(code):
        if c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
        elif c == ',' and depth == 0:
            parts.append(code[start:i].strip())
            start = i + 1
    parts.append(code[start:].strip())
    return parts


def scan_file(path, relative):
    source = path.read_text(encoding='latin1')
    code = mask_noncode(source)
    line_starts = [0] + [m.end() for m in re.finditer('\n', source)]
    functions = [(m.start(), m[1].lower()) for m in FUNCTION.finditer(source)]
    function_starts = [p for p, _ in functions]

    def location(pos):
        row = {'file': relative, 'line': bisect.bisect_right(line_starts, pos)}
        ix = bisect.bisect_right(function_starts, pos) - 1
        if ix >= 0:
            row['nearest_function'] = functions[ix][1]
        return row

    types = []
    for m in TYPE.finditer(code):
        start = m.end() - 1
        end = closing(code, start, '{', '}')
        if end is None:
            continue
        fields = []
        for field in FIELD.finditer(code, start + 1, end):
            # Declarations only: exclude identifiers used in inline bodies.
            before = code[code.rfind('\n', start, field.start()) + 1:field.start()]
            after = code[field.end():end]
            if not re.fullmatch(r'\s*(?:[\w:]+\s+)*[\w:]+\s*[\s*]*', before):
                continue
            if not re.match(r'\s*(?:\[[^\]]*\]\s*)*(?:;|:|=)', after):
                continue
            fields.append(dict(location(field.start()), name=field[0],
                               declaration=source[field.start() - len(before):source.find('\n', field.end())].strip()))
        types.append(dict(location(m.start()), kind=m[1], name=m[2],
                          end_line=bisect.bisect_right(line_starts, end), unnamed_fields=fields))

    raw = []
    for m in DEREFERENCE.finditer(code):
        end = closing(code, m.end() - 1)
        if end is None:
            continue
        expr = code[m.end():end]
        if not re.search(r'\+|\-|\*|0x[0-9a-fA-F]+', expr):
            continue
        raw.append(dict(location(m.start()), access_type=' '.join(m[1].split()),
                        expression=source[m.start():end + 1]))

    sizes = []
    for m in CALL.finditer(code):
        end = closing(code, m.end() - 1)
        if end is None:
            continue
        args = arguments(code[m.end():end])
        sizes_arg = args[:2] if m[1] == 'calloc' else args[-1:]
        if not any(re.search(r'\b0x[0-9a-fA-F]+\b|(?<![\w.])\d{2,}\b', arg) for arg in sizes_arg):
            continue
        if all('sizeof' in arg for arg in sizes_arg):
            continue
        sizes.append(dict(location(m.start()), call=m[1], size_expressions=sizes_arg))

    strides = [dict(location(m.start()), value=m[1].lower()) for m in STRIDE.finditer(code)]
    unknown = [dict(location(m.start()), name=m[0]) for m in UNKNOWN.finditer(code)]
    return {'sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'types': types,
            'raw_accesses': raw, 'fixed_sizes': sizes, 'hex_multipliers': strides,
            'unknown_identifiers': unknown}


def inventory(source_dir):
    records = {p.name: scan_file(p, p.name) for p in sorted(source_dir.iterdir())
               if p.suffix in ('.cpp', '.h')}
    groups = ('types', 'raw_accesses', 'fixed_sizes', 'hex_multipliers', 'unknown_identifiers')
    data = {key: [item for record in records.values() for item in record[key]] for key in groups}
    # A nested named type's declaration may also appear inside its parent.
    fields = {(f['file'], f['line'], f['name']) for t in data['types'] for f in t['unnamed_fields']}
    files = {name: {key: len(record[key]) for key in groups} for name, record in records.items()}
    metrics = {key: len(items) for key, items in data.items()}
    metrics['unnamed_field_declarations'] = len(fields)
    metrics['unique_unknown_identifiers'] = len({row['name'] for row in data['unknown_identifiers']})
    return dict(schema_version=1,
                caveat='Syntactic candidates; nearest_function is a context hint, not ownership. Hex multipliers can be numeric scales rather than strides.',
                source_sha256={name: record['sha256'] for name, record in records.items()},
                metrics=metrics, files=files, **data)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--source-dir', type=Path, default=ROOT / 'CMR2Decomp')
    ap.add_argument('--json', type=Path, help='write the complete work list and source hashes')
    args = ap.parse_args()
    data = inventory(args.source_dir)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(data, indent=2) + '\n')
    print(json.dumps(data['metrics'], indent=2))
    print('\nFiles with most raw accesses:')
    for name, record in sorted(data['files'].items(), key=lambda row: -row[1]['raw_accesses'])[:12]:
        if record['raw_accesses']:
            print(f"{name:26} {record['raw_accesses']:5} raw accesses, {record['fixed_sizes']:3} fixed sizes")


if __name__ == '__main__':
    main()
