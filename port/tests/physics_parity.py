#!/usr/bin/env python3
"""Run the real stage against frozen original vehicle data at six render cadences."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools/reference_capture'))
from compare import ticks
from physics import compare_vehicle, clock_summary

REFERENCE = ROOT / 'tests/reference/finland-stage1'


def authoritative(record):
    # Retain every canonical car field except the four interpolation bytes and
    # remaining steps in the current render frame, plus complete part/damage/
    # checkpoint/timing and body matrices. The stored simulation rate is checked.
    # Presentation buffers, shared RNG and clocks remain in the full comparator.
    return {key: value for key, value in record.items()
            if key.startswith(('car.', 'parts.', 'damage.', 'checkpoint.', 'timing.',
                               'world.', 'body.', 'shadow.'))
            and key not in {f'car.0.field_0xa90[{i}]' for i in range(4)}
            and key != 'car.0.field_0xb43'}


def verify(probe, data, output):
    schema = json.loads((REFERENCE / 'schema.json').read_text())
    original = list(ticks(REFERENCE / 'original-state.bin.gz', schema))
    report = {'schema': 1, 'reference_sha256': hashlib.sha256(
        (REFERENCE / 'original-state.bin.gz').read_bytes()).hexdigest(),
        'probe_sha256': hashlib.sha256(probe.read_bytes()).hexdigest(),
        'authoritative_scalars_per_tick': len(authoritative(original[0])), 'cadences': {}}
    for cadence in ('30', '60', '120', '144', '240', 'variable'):
        run = output / cadence
        run.mkdir(parents=True)
        env = dict(os.environ, OPENCMR2_HEADLESS='1', SDL_VIDEODRIVER='dummy',
                   SDL_AUDIODRIVER='dummy', XDG_DATA_HOME=str(run / 'user'),
                   OPENCMR2_REFERENCE_CLOCK='1', OPENCMR2_STATE_CAPTURE=str(run / 'state.bin'))
        with (run / 'run.log').open('w') as log:
            subprocess.run([str(probe), str(data), str(run / 'trace.jsonl'), cadence,
                            '500', str(REFERENCE / 'inputs.csv')], env=env,
                           stdout=log, stderr=subprocess.STDOUT, check=True, timeout=60)
        candidate = list(ticks(run / 'state.bin', schema))
        vehicle = compare_vehicle(original, candidate)
        if not vehicle['identical']:
            raise ValueError(f'{cadence} FPS: {vehicle}')
        for left, right in zip(original, candidate):
            a, b = authoritative(left), authoritative(right)
            if a != b:
                names = [k for k in sorted(a.keys() | b.keys()) if a.get(k) != b.get(k)]
                key = names[0]
                raise ValueError(f'{cadence} FPS tick {left["tick"]}: {key}: {a.get(key)} != {b.get(key)}')
        clock = clock_summary(candidate, 100000)
        if clock['ticks_between_10s_and_18s'] != 200:
            raise ValueError(f'{cadence} FPS: expected 200 updates in 8 seconds: {clock}')
        report['cadences'][cadence] = {'vehicle': vehicle, 'all_authoritative_values_identical': True,
                                       'clock': clock}
        print(f'{cadence}: 500 steps identical to original + SilentPatch; 25 Hz', flush=True)
    (output / 'acceptance.json').write_text(json.dumps(report, indent=2) + '\n')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--probe', type=Path, required=True)
    p.add_argument('--data', type=Path, required=True)
    p.add_argument('--output', type=Path)
    args = p.parse_args()
    if args.output:
        args.output.mkdir(parents=True, exist_ok=False)
        verify(args.probe.resolve(), args.data.resolve(), args.output.resolve())
    else:
        with tempfile.TemporaryDirectory(prefix='opencmr2-physics-') as directory:
            verify(args.probe.resolve(), args.data.resolve(), Path(directory))


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(f'physics parity failed: {error}', file=sys.stderr)
        sys.exit(1)
