#!/usr/bin/env python3
"""Generate, gate and isolate evidence-backed migration families.

A recipe supplies reviewed classifications and transformations. Discovery uses
rg to collect every spelling reference, not to infer a field's meaning. Gates
run as argv without a shell. Dependent families are indivisible during isolation.
"""
import argparse
import difflib
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
CLASSIFICATIONS = {'pointer', 'number', 'mixed', 'file_offset', 'layout'}


def safe_path(root, name):
    path = (root / name).resolve()
    if not path.is_relative_to(root.resolve()) or '.git' in path.relative_to(root).parts:
        raise ValueError(f'Path outside source workspace: {name}')
    return path


def read_optional(path):
    return path.read_bytes() if path.exists() else None


def validate_recipe(recipe):
    families = recipe['families']
    ids = [f['id'] for f in families]
    if not ids or not all(isinstance(name, str) and name for name in ids) or len(set(ids)) != len(ids):
        raise ValueError('Family IDs must be nonempty and unique')
    for family in families:
        if not family.get('evidence') or family.get('classification') not in CLASSIFICATIONS:
            raise ValueError(f"{family['id']}: classification and evidence required")
        for dependency in family.get('requires', []):
            if dependency not in ids or ids.index(dependency) >= ids.index(family['id']):
                raise ValueError('Dependencies must precede their consumers in the recipe')
        if not family.get('rules'):
            raise ValueError(f"{family['id']}: no transformations")
    return families


def validate_plan(plan):
    if plan.get('version') != 1:
        raise ValueError('Unsupported plan version')
    families = validate_recipe({'families': [dict(f, rules=f.get('operations')) for f in plan['families']]})
    for family in families:
        for operation in family['operations']:
            if operation['path'] not in plan['inputs'] or operation['count'] <= 0:
                raise ValueError(f"{family['id']}: operation lacks a frozen input or match count")
    return plan


def generate(root, recipe):
    """Freeze glob results, match counts and input hashes into a reviewable plan."""
    families = validate_recipe(recipe)
    inputs, current, result = {}, {}, []
    for family in families:
        operations = []
        for rule in family['rules']:
            paths = rule.get('paths', [])
            names = sorted({str(p.relative_to(root)) for pattern in paths for p in root.glob(pattern) if p.is_file()})
            if 'create' in rule:
                names = [rule['create']]
            if not names:
                raise ValueError(f"{family['id']}: empty file selection: {paths}")
            total = 0
            for name in names:
                path = safe_path(root, name)
                if name not in current:
                    inputs[name] = read_optional(path)
                    current[name] = inputs[name]
                before = current[name]
                if 'create' in rule:
                    if before is not None:
                        raise ValueError(f'New file already exists: {name}')
                    after, count = rule['content'].encode(), 1
                else:
                    if before is None:
                        raise ValueError(f'Missing input: {name}')
                    source = before.decode('utf-8')
                    pattern = rule['find'] if rule.get('regex') else re.escape(rule['find'])
                    replacement = rule['replace']
                    after_text, count = re.subn(pattern, replacement if rule.get('regex') else lambda m: replacement, source, flags=re.MULTILINE)
                    after = after_text.encode('utf-8')
                total += count
                if count:
                    operations.append({'path': name, 'before': before.decode('utf-8') if before is not None else None, 'after': after.decode('utf-8'), 'create': 'create' in rule, 'find': rule.get('find'), 'replace': rule.get('replace'), 'regex': rule.get('regex', False), 'count': count})
                    current[name] = after
            if total != rule['count']:
                raise ValueError(f"{family['id']}: expected {rule['count']} matches, found {total}")
        if not operations:
            raise ValueError(f"{family['id']}: no changes")
        result.append({k: family[k] for k in ('id', 'evidence', 'classification') } | {'requires': family.get('requires', []), 'symbols': family.get('symbols', []), 'operations': operations})
    return {'version': 1, 'families': result, 'inputs': {name: hashlib.sha256(value).hexdigest() if value is not None else None for name, value in inputs.items()}}


def components(families):
    """Dependency-connected families must be applied/parked as one unit."""
    groups = [{family['id']} for family in families]
    for family in families:
        for dependency in family.get('requires', []):
            selected = [group for group in groups if family['id'] in group or dependency in group]
            merged = set().union(*selected)
            groups = [group for group in groups if group not in selected] + [merged]
    order = {family['id']: i for i, family in enumerate(families)}
    return sorted(groups, key=lambda group: min(order[name] for name in group))


def render(inputs, families, selected):
    state = dict(inputs)
    for family in families:
        if family['id'] not in selected:
            continue
        for operation in family['operations']:
            name = operation['path']
            if operation['create']:
                if state[name] is not None:
                    raise ValueError(f'New file already exists: {name}')
                state[name] = operation['after'].encode()
            else:
                if state[name] is None:
                    raise ValueError(f"{family['id']}: missing dependency for {name}")
                pattern = operation['find'] if operation['regex'] else re.escape(operation['find'])
                replacement = operation['replace']
                source, count = re.subn(pattern, replacement if operation['regex'] else lambda m: replacement, state[name].decode(), flags=re.MULTILINE)
                if count != operation['count']:
                    raise ValueError(f"{family['id']}: dependent/overlapping edits require an explicit dependency: {name}")
                state[name] = source.encode()
    return state


def publish(root, state):
    for name, value in state.items():
        path = safe_path(root, name)
        if value is None:
            if path.exists():
                path.unlink()
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(value)


def score_regressions(baseline, current):
    result = []
    for address, previous in baseline.items():
        row = current.get(address)
        if row is None:
            result.append((address, 'missing function'))
        else:
            for key in ('s', 'fz', 'x'):
                if previous.get(key) is not None and row.get(key, -1) < previous[key]:
                    result.append((address, key, previous[key], row.get(key)))
    return result


def apply_plan(root, plan, gate, output):
    """Accept large batches; recursively split failures against accepted changes."""
    validate_plan(plan)
    families = plan['families']
    inputs = {name: read_optional(safe_path(root, name)) for name in plan['inputs']}
    for name, content in inputs.items():
        actual = hashlib.sha256(content).hexdigest() if content is not None else None
        if actual != plan['inputs'][name]:
            raise ValueError(f'Input changed since generation: {name}')
    metrics = root/'CMR2PROGRESS/bytes.json'
    baseline = json.loads(metrics.read_text()) if metrics.exists() else None
    accepted, rejected, probes = set(), [], []
    last_published = dict(inputs)
    metric_bytes = metrics.read_bytes() if metrics.exists() else None
    output.mkdir(parents=True, exist_ok=True)

    def probe(selected):
        state = render(inputs, families, selected)
        for name, expected in last_published.items():
            if read_optional(safe_path(root, name)) != expected:
                raise RuntimeError(f'Concurrent source edit detected; preserving it: {name}')
        publish(root, state)
        last_published.update(state)
        number = len(probes) + 1
        log = output/f'probe-{number:03}.txt'
        with log.open('wb') as stream:
            process = subprocess.run(gate, cwd=root, stdout=stream, stderr=subprocess.STDOUT)
        if metric_bytes is not None and read_optional(metrics) != metric_bytes:
            metrics.write_bytes(metric_bytes)
            raise RuntimeError('Probe gate rewrote baseline metrics; use a read-only matching gate')
        regressions = score_regressions(baseline, json.loads(metrics.read_text())) if baseline is not None else []
        passed = process.returncode == 0 and not regressions
        probes.append({'families': sorted(selected), 'exit_code': process.returncode, 'regressions': regressions, 'passed': passed, 'log': str(log.resolve())})
        print(f"{'PASS' if passed else 'PARK'} probe {number}: {', '.join(sorted(selected)) or 'baseline'}", flush=True)
        return passed

    def isolate(groups):
        candidate = set().union(*groups)
        if probe(accepted | candidate):
            accepted.update(candidate)
        elif len(groups) == 1:
            rejected.append(sorted(candidate))
        else:
            middle = len(groups)//2
            isolate(groups[:middle])
            isolate(groups[middle:])

    try:
        # Establish that unrelated pending work does not already fail the gate.
        if not probe(set()):
            raise RuntimeError('Baseline gate fails; no migration was applied')
        isolate(components(families))
        # The last rejected probe must not be left in the workspace.
        final = render(inputs, families, accepted)
        for name, expected in last_published.items():
            if read_optional(safe_path(root, name)) != expected:
                raise RuntimeError(f'Concurrent source edit detected; preserving it: {name}')
        publish(root, final)
        last_published.update(final)
    except BaseException:
        publish(root, {name: value for name, value in inputs.items() if read_optional(safe_path(root, name)) == last_published[name]})
        raise
    report = {'accepted': sorted(accepted), 'parked_dependency_groups': rejected, 'probes': probes, 'baseline_metrics_sha256': hashlib.sha256(json.dumps(baseline, sort_keys=True).encode()).hexdigest()}
    (output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    return report


def discovery(root, symbols):
    command = ['rg', '--json', '-n', '-F']
    for symbol in symbols:
        command += ['-e', symbol]
    result = subprocess.run(command + ['CMR2Decomp'], cwd=root, capture_output=True, text=True)
    if result.returncode not in (0, 1):
        raise RuntimeError(result.stderr)
    return [json.loads(line)['data'] for line in result.stdout.splitlines() if json.loads(line)['type'] == 'match']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--recipe', type=Path)
    parser.add_argument('--plan', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--finalize', action='store_true', help='full build, measurement, metadata, gate and suite; never commits')
    parser.add_argument('--discover', nargs='+', metavar='SYMBOL')
    parser.add_argument('--gate', nargs=argparse.REMAINDER, help='argv gate override; default is MSVC6 changed-TU matching')
    args = parser.parse_args()
    root = args.root.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    if args.discover:
        (args.output/'references.json').write_text(json.dumps(discovery(root, args.discover), indent=2)+'\n')
        return
    if bool(args.recipe) == bool(args.plan):
        parser.error('Specify exactly one of --recipe or --plan')
    plan = generate(root, json.loads(args.recipe.read_text())) if args.recipe else json.loads(args.plan.read_text())
    validate_plan(plan)
    (args.output/'plan.json').write_text(json.dumps(plan, indent=2)+'\n')
    inputs = {name: read_optional(safe_path(root, name)) for name in plan['inputs']}
    final = render(inputs, plan['families'], {f['id'] for f in plan['families']})
    patch = ''.join(''.join(difflib.unified_diff((inputs[name] or b'').decode().splitlines(True), (final[name] or b'').decode().splitlines(True), fromfile='a/'+name, tofile='b/'+name)) for name in sorted(inputs))
    (args.output/'migration.patch').write_text(patch)
    if args.finalize and not args.apply:
        parser.error('--finalize requires --apply')
    if not args.apply:
        print(f"Review {args.output/'migration.patch'} and {args.output/'plan.json'}; sources unchanged")
        return
    gate = args.gate or [sys.executable, 'scripts/match.py', '--changed', '--no-diff']
    report = apply_plan(root, plan, gate, args.output)
    if args.finalize and report['accepted']:
        pipeline = [('build', ['scripts/build.py']), ('measure', ['scripts/measure.py']), ('prepare', ['scripts/prepare_fastcmp.py']), ('matching', ['scripts/match.py', '--changed', '--no-diff']), ('suite', ['tests/run_differential_suite.py', '--jobs', '3'])]
        baseline = json.loads((root/'CMR2PROGRESS/bytes.json').read_text())
        for name, command in pipeline:
            with (args.output/f'{name}.txt').open('wb') as stream:
                result = subprocess.run([sys.executable, *command], cwd=root, stdout=stream, stderr=subprocess.STDOUT)
            regressions = score_regressions(baseline, json.loads((root/'CMR2PROGRESS/bytes.json').read_text()))
            if result.returncode or regressions:
                report['stable'] = False
                report['final_failure'] = {'step': name, 'exit_code': result.returncode, 'regressions': regressions}
                (args.output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
                raise RuntimeError(f'Final {name} failed; accepted changes remain uncommitted for review')
        report['stable'] = True
        (args.output/'report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps({key: value for key, value in report.items() if key != 'probes'}, indent=2))


if __name__ == '__main__':
    main()
