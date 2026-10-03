#!/usr/bin/env python3
"""Generate the behavior-validation queue; do not infer proof from matching.

Runtime branch edges come from successful original-machine Unicorn harnesses.
An indirect dispatch or incomplete static CFG keeps coverage explicitly partial.
Even complete branch coverage leaves domain, boundary and dependency review open.
"""
import argparse
import collections
import csv
import hashlib
import json
from pathlib import Path
import re

from audit_logic import CRITICAL

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def branch_coverage(rows, observed):
    indexed = {a: (size, text) for a, size, text in rows}
    reachable, work, partial = set(), [rows[0][0]] if rows else [], not bool(rows)
    while work:
        address = work.pop()
        if address in reachable:
            continue
        if address not in indexed:
            partial = True
            continue
        reachable.add(address)
        size, text = indexed[address]
        op = text.split(' ', 1)[0]
        if op in ('ret', 'retf', 'int3', 'hlt'):
            continue
        if op.startswith('j') or op.startswith('loop'):
            target = re.fullmatch(r'\w+ (0x[\da-f]+)', text)
            if target:
                work.append(int(target[1], 16))
            else:
                partial = True
            if op == 'jmp':
                continue
        work.append(address + size)
    expected = set()
    for address in reachable:
        size, text = indexed[address]
        op = text.split(' ', 1)[0]
        if (op.startswith('j') and op != 'jmp') or op.startswith('loop'):
            target = re.fullmatch(r'\w+ (0x[\da-f]+)', text)
            if target:
                expected.update(((address, address + size), (address, int(target[1], 16))))
            else:
                partial = True
    observed = set(map(tuple, observed))
    missing = expected - observed
    return {
        'cfg_partial': partial,
        'known_conditional_edges': len(expected),
        'observed_known_edges': len(expected & observed),
        'missing_edges': sorted(missing),
        'complete_known_edges': not partial and not missing,
        'observed_branch_edges': sorted(observed),
    }


REVIEW_FIELDS = ('input_domain', 'original_machine_review', 'boundary_cases',
                 'dependencies', 'regression_sensitivity', 'limitations')


def review_gaps(review, function, manifest, tests, original_sha):
    """A completed coverage graph cannot replace a current, explicit review."""
    if not isinstance(review, dict) or review.get('reviewed') is not True:
        return ['review_input_domain_and_boundaries',
                'document_real_and_controlled_dependencies',
                'verify_regression_detects_representative_faults']
    if any(not isinstance(review.get(key), str) or not review[key].strip() for key in REVIEW_FIELDS):
        return ['complete_behavior_review']
    headers = {p: value for p, value in manifest['source_sha256'].items() if p.endswith('.h')}
    if (review.get('original_sha256') != original_sha or
            review.get('compiler_sha256') != manifest['compiler_sha256'] or
            review.get('compiler_flags') != manifest['flags'] or
            review.get('qifist_files') != manifest['qifist_files'] or
            review.get('source_sha256') != manifest['source_sha256'].get('CMR2Decomp/' + function['file']) or
            review.get('header_sha256') != headers):
        return ['refresh_behavior_review_for_current_source_and_compiler']
    reviewed = review.get('reviewed_harnesses', [])
    hashes = review.get('harness_sha256', {})
    if (not reviewed or not hashes or
            not set(reviewed).issubset(function['tests_passed']) or
            not set(reviewed).issubset(hashes) or
            any(tests['test_source_sha256'].get(name) != value for name, value in hashes.items())):
        return ['refresh_behavior_review_for_current_harnesses']
    return []


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--disassembly', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=ROOT / 'CMR2PROGRESS')
    args = parser.parse_args()
    auditpath = ROOT / 'CMR2PROGRESS/logic-audit.json'
    testpath = ROOT / 'CMR2PROGRESS/logic-tests.json'
    audit = json.loads(auditpath.read_text())
    tests = json.loads(testpath.read_text())
    manifest = json.loads((ROOT / 'build/manifest.json').read_text())
    if (audit['build'] != manifest or audit['test_evidence_sha256'] != sha(testpath)
            or tests['exe_sha256'] != manifest['exe_sha256']):
        parser.error('Build, audit or behavior evidence changed; refresh them first.')
    for suffix in ('exe', 'pdb'):
        if sha(ROOT / ('build/CMR2.' + suffix)) != manifest[suffix + '_sha256']:
            parser.error('Build artifacts changed since validation.')
    if sha(ROOT / 'cmr2bin/CMR2.exe') != audit['original_sha256']:
        parser.error('Original executable changed since validation.')
    if sha(ROOT / 'tests/logic-targets.json') != tests['suite_sha256']:
        parser.error('Harness registry changed since validation.')
    for name, expected in tests['test_source_sha256'].items():
        if (name.startswith('differential_') or name in ('matching_entities.py',
                'record_unicorn_coverage.py', 'run_differential_suite.py')):
            if sha(ROOT / 'tests' / name) != expected:
                parser.error('Harness or coverage recorder changed: ' + name)
    if any(sha(ROOT / path) != expected for path, expected in manifest['source_sha256'].items()):
        parser.error('Sources changed since validation.')
    disassembly = json.loads(args.disassembly.read_text())
    reviewpath = ROOT / 'CMR2PROGRESS/confidence-reviews.json'
    reviews = json.loads(reviewpath.read_text()) if reviewpath.exists() else {}
    pending = {f['address']: f for f in audit['functions'] if f['status'] != 'local_byte_exact'}
    evidence = collections.defaultdict(lambda: {'instructions': set(), 'edges': set(), 'tests': []})
    for result in tests['results']:
        if result['exit_code'] != 0:
            continue
        for address, coverage in result.get('coverage', {}).get('functions', {}).items():
            if coverage['instructions']:
                evidence[address]['instructions'].update(coverage['instructions'])
                evidence[address]['edges'].update(map(tuple, coverage['branch_edges']))
                evidence[address]['tests'].append(result['test'])
    callers = collections.defaultdict(set)
    for address, function in pending.items():
        for target in function['machine_features']['original']['calls']:
            if target in pending:
                callers[target].add(address)
    queue = []
    for address, function in pending.items():
        runtime = evidence[address]
        rows = disassembly.get(address, {}).get('original', [])
        coverage = branch_coverage(rows, runtime['edges']) if rows else None
        gaps = []
        if function['status'] == 'test_failed':
            gaps.append('fix_differential_failure')
        if not function['tests_passed']:
            gaps.append('add_direct_differential_harness')
        if not runtime['instructions']:
            gaps.append('measure_original_branch_coverage')
        elif coverage is None or coverage['cfg_partial']:
            gaps.append('resolve_dispatch_and_reachable_cases')
        elif not coverage['complete_known_edges']:
            gaps.append('exercise_missing_conditional_edges')
        if function['unknown']:
            gaps.append('resolve_unknown_relocations')
        gaps.extend(review_gaps(reviews.get(address), function, manifest, tests,
                                audit['original_sha256']))
        tested = bool(function['tests_passed'])
        priority = (1000 if function['status'] == 'test_failed' else 0)
        priority += 100 if function['file'] in CRITICAL else 20
        priority += 70 if not tested else 0
        priority += min(60, 5 * len(callers[address]))
        priority += 20 if function['unknown'] else 0
        queue.append({
            'address': address, 'name': function['name'], 'file': function['file'],
            'line': function['line'], 'original_bytes': function['original_bytes'],
            'matching': function['byte_score'], 'priority': priority,
            'evidence': 'fixtures_pass' if tested else 'unverified',
            'tests_passed': function['tests_passed'],
            'pending_direct_callers': sorted(callers[address]),
            'original_branch_coverage': coverage if runtime['instructions'] else None,
            'coverage_tests': runtime['tests'],
            'high_confidence': not gaps,
            'automatic_mutation_eligible': not gaps,
            'behavior_review': reviews.get(address),
            'open_checks': gaps,
        })
    queue.sort(key=lambda f: (-f['priority'], f['original_bytes'], f['address']))
    counts = {
        'pending': len(queue),
        'fixtures_pass': sum(f['evidence'] == 'fixtures_pass' for f in queue),
        'runtime_coverage_measured': sum(bool(f['coverage_tests']) for f in queue),
        'complete_known_conditional_edges': sum(bool(f['original_branch_coverage'] and
            f['original_branch_coverage']['complete_known_edges']) for f in queue),
        'high_confidence_certified': sum(f['high_confidence'] for f in queue),
        'automatic_mutation_eligible': sum(f['automatic_mutation_eligible'] for f in queue),
    }
    output = {
        'build': manifest, 'original_sha256': audit['original_sha256'],
        'audit_sha256': sha(auditpath), 'test_evidence_sha256': sha(testpath),
        'disassembly_sha256': sha(args.disassembly), 'counts': counts,
        'confidence_reviews_sha256': sha(reviewpath) if reviewpath.exists() else None,
        'limits': ['Known branch coverage is not path coverage or a proof of equivalence.',
                   'Native/Wine-only execution has no measured branch coverage here.',
                   'Controlled providers validate boundary arguments, not provider behavior.',
                   'High confidence requires explicit current review and is bounded by its input domain.',
                   'Mutation eligibility is a recorded prerequisite; it does not start a search.'],
        'functions': queue,
    }
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / 'validation-queue.json').write_text(json.dumps(output, indent=2) + '\n')
    columns = ['address', 'name', 'file', 'line', 'original_bytes', 'matching',
               'priority', 'evidence', 'pending_direct_callers', 'open_checks']
    with (args.output / 'validation-queue.tsv').open('w', newline='') as handle:
        writer = csv.DictWriter(handle, columns, delimiter='\t', lineterminator='\n')
        writer.writeheader()
        for function in queue:
            writer.writerow({k: ';'.join(function[k]) if isinstance(function[k], list)
                             else function[k] for k in columns})
    print(counts)


if __name__ == '__main__':
    main()
