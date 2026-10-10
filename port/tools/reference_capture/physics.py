#!/usr/bin/env python3
"""Report vehicle parity separately from clock pacing and presentation bytes."""
import argparse
import bisect
import json
from pathlib import Path
import sys
from compare import ticks, fingerprint

# These are explicit gameplay values; the full binary comparison remains available.
CAR_FIELDS = ('type', 'gear', 'speed', 'sector', 'heading', 'position', 'positionPrev', 'positionPrev2',
              'velocity', 'velocityNext', 'angularVelocity', 'right', 'up', 'forward', 'groundNormal',
              'cornerOnGround', 'cornerTriangle', 'wheelSurface', 'wheelSurfaceType', 'cornerHeight',
              'wheelLoad', 'wheelTorque', 'wheelSlip', 'wheelSlipLateral', 'wheelSlipping',
              'cornerLoad', 'cornerForce', 'baseForce', 'tyreGrip', 'cornerMass', 'wheel0x988', 'wheel0x9a8',
              'handbrake', 'handbrakeForce', 'brakeInput', 'brakeBias', 'driveSplit',
              'field_0x818', 'field_0x824', 'field_0x7a4', 'field_0xb21', 'field_0xb9c')


def vehicle_state(record):
    result = {k: record[k] for k in ('state', 'driving_tick', 'order', 'physics.step', 'physics.scale')}
    for key, value in record.items():
        if key.startswith('input.'):
            result[key] = value
        if key.startswith('car.'):
            field = key.split('.', 2)[2].split('.', 1)[0].split('[', 1)[0]
            if field in CAR_FIELDS:
                result[key] = value
        if key.startswith(('parts.', 'damage.', 'checkpoint.')):
            result[key] = value
    for car in record['order']:
        for field in ('position.x', 'position.y', 'position.z', 'velocity.x', 'velocity.y', 'velocity.z', 'speed', 'gear'):
            if f'car.{car}.{field}' not in result:
                raise ValueError(f'missing authoritative vehicle field: {field}')
    return result


def compare_vehicle(reference, candidate, aligned=False):
    first, differing_ticks = None, 0
    for a, b in zip(reference, candidate):
        left, right = vehicle_state(a), vehicle_state(b)
        if aligned:
            left.pop('input.tick'); right.pop('input.tick')
        found = [k for k in sorted(left.keys() | right.keys()) if left.get(k) != right.get(k)]
        if found:
            differing_ticks += 1
            if first is None:
                name = found[0]
                first = {'tick': a['tick'], 'candidate_tick': b['tick'], 'driving_tick': a['driving_tick'],
                         'field': name, 'reference': left.get(name), 'candidate': right.get(name)}
    return {'identical': len(reference) == len(candidate) and first is None,
            'first_difference': first, 'different_ticks': differing_ticks,
            'scalar_fields_per_tick': len(vehicle_state(reference[0]))}


def compare_driving(reference, candidate):
    left = [t for t in reference if t['state'] == 8]
    right = [t for t in candidate if t['state'] == 8]
    count = min(len(left), len(right))
    result = compare_vehicle(left[:count], right[:count], aligned=True)
    result.update({'common_driving_ticks': count, 'reference_driving_ticks': len(left),
                   'candidate_driving_ticks': len(right)})
    return result


def clock_summary(records, origin):
    times = [t['time'] - origin for t in records]
    if times[0] < 0 or times != sorted(times):
        raise ValueError('game clock went backwards or predates specified origin')
    driving = [t for t in records if t['state'] == 8]
    return {'ticks': len(records), 'first_tick_ms': times[0], 'last_tick_ms': times[-1],
            'driving_ticks': len(driving), 'first_driving_tick_ms': driving[0]['time']-origin if driving else None,
            'driving_capture_span_ms': driving[-1]['time']-driving[0]['time'] if driving else None,
            'ticks_between_10s_and_18s': bisect.bisect_right(times, 18000)-bisect.bisect_right(times, 10000) if times[-1] >= 18000 else None,
            'ticks_at_elapsed_ms': {str(ms): bisect.bisect_right(times, ms) for ms in (5000, 10000, 15000, 18000)
                                    if times[-1] >= ms}}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('reference', type=Path)
    p.add_argument('candidates', nargs='+', help='label=recording.bin[.gz]')
    p.add_argument('--schema', required=True, type=Path)
    p.add_argument('--output', required=True, type=Path)
    p.add_argument('--clock-origin', type=int, default=100000)
    args = p.parse_args()
    if args.output.exists():
        raise ValueError('report exists; choose a new output')
    schema = json.loads(args.schema.read_text())
    reference = list(ticks(args.reference, schema))
    ref_clock = clock_summary(reference, args.clock_origin)
    report = {'schema': 1, 'scope': 'explicit vehicle/control/damage/checkpoint fields; clock pacing measured independently; full-state comparisons remain separate',
              'clock_origin_ms': args.clock_origin, 'reference_sha256': fingerprint(args.reference), 'reference_clock': ref_clock,
              'car_fields': list(CAR_FIELDS), 'comparisons': {}}
    for item in args.candidates:
        label, path = item.split('=', 1)
        if label in report['comparisons']:
            raise ValueError('duplicate candidate label')
        candidate = list(ticks(Path(path), schema))
        clock = clock_summary(candidate, args.clock_origin)
        report['comparisons'][label] = {'vehicle': compare_vehicle(reference, candidate),
                                       'aligned_driving': compare_driving(reference, candidate), 'clock': clock,
                                       'candidate_sha256': fingerprint(Path(path))}
        print(label, 'vehicle identical:', report['comparisons'][label]['vehicle']['identical'],
              'last tick ms:', clock['last_tick_ms'], 'ticks at 18s:', clock['ticks_at_elapsed_ms'].get('18000'))
    args.output.write_text(json.dumps(report, indent=2)+'\n')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError) as error:
        print(f'physics report failed: {error}', file=sys.stderr)
        sys.exit(2)
