#!/usr/bin/env python3
"""Validate strict framing and distinguish real state from relocated pointers."""
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools/reference_capture'))
import physics

MODULE = Path(__file__).resolve().parents[1] / 'tools/reference_capture/compare.py'
spec = importlib.util.spec_from_file_location('reference_compare', MODULE)
compare = importlib.util.module_from_spec(spec)
spec.loader.exec_module(compare)

SCHEMA = {'sizes': {'car': 16}, 'fields': {'car': [{'name': 'position.x', 'offset': 0, 'format': 'i'}]}}


def fixture(position=42, pointer=0x400000, padding=0):
    data = bytearray(b'CMRSTAT1' + struct.pack('<6I', 0x5449434b, 0, 8, 1, 100000, 1))
    for name, block in [('input', struct.pack('<13i', 0, 8, *([0]*11))), ('physics', bytes(12)),
                        ('order', bytes(2)), ('car.0', struct.pack('<4i', position, pointer, padding, 0))]:
        data += struct.pack('<I', len(name)) + name.encode() + struct.pack('<I', len(block)) + block
    return bytes(data + struct.pack('<3I', 0, 0x444f4e45, 1))


class CaptureTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.a, self.b = self.root/'a.bin', self.root/'b.bin'
        self.a.write_bytes(fixture())

    def tearDown(self):
        self.temp.cleanup()

    def test_relocated_addresses_and_padding(self):
        self.b.write_bytes(fixture(pointer=0x11000000, padding=255))
        self.assertTrue(compare.compare(self.a, self.b, SCHEMA)['identical'])

    def test_single_fixed_point_unit_is_a_difference(self):
        self.b.write_bytes(fixture(position=43))
        result = compare.compare(self.a, self.b, SCHEMA)
        self.assertFalse(result['identical'])
        self.assertEqual(result['differences']['car.0.position.x']['first_tick'], 0)
        self.assertEqual(result['differences']['car.0.position.x']['candidate'], 43)

    def test_truncated_capture_rejected(self):
        for removed in (1, 8, 12, 50):
            self.b.write_bytes(fixture()[:-removed])
            with self.assertRaises(ValueError):
                list(compare.ticks(self.b, SCHEMA))

    def test_wrong_footer_count_and_trailing_bytes(self):
        for data in (fixture()[:-4] + struct.pack('<I', 2), fixture()+b'x'):
            self.b.write_bytes(data)
            with self.assertRaises(ValueError):
                list(compare.ticks(self.b, SCHEMA))

    def test_invalid_car_index_rejected(self):
        data = fixture().replace(b'order\x02\x00\x00\x00\x00\x00', b'order\x02\x00\x00\x00\x08\x00')
        self.b.write_bytes(data)
        with self.assertRaisesRegex(ValueError, 'car indices'):
            list(compare.ticks(self.b, SCHEMA))

    def test_compressed_recording(self):
        import gzip
        path = self.root/'a.bin.gz'
        path.write_bytes(gzip.compress(fixture(), mtime=0))
        self.assertEqual(list(compare.ticks(self.a, SCHEMA)), list(compare.ticks(path, SCHEMA)))


class PhysicsReportTest(unittest.TestCase):
    @staticmethod
    def records(interval):
        result = []
        for tick in range(601):
            row = {'tick': tick, 'time': tick*interval, 'state': 8, 'driving_tick': tick+1,
                   'order': [0], 'physics.step': 2621, 'physics.scale': 65536,
                   'input.tick': tick, 'input.state': 8, 'car.0.speed': tick, 'car.0.gear': 1}
            for field in ('position', 'velocity'):
                for axis in ('x', 'y', 'z'):
                    row[f'car.0.{field}.{axis}'] = tick
            result.append(row)
        return result

    def test_equal_states_do_not_prove_equal_game_speed(self):
        normal, fast = self.records(40), self.records(35)
        self.assertTrue(physics.compare_vehicle(normal, fast)['identical'])
        self.assertEqual(physics.clock_summary(normal, 0)['ticks_between_10s_and_18s'], 200)
        self.assertNotEqual(physics.clock_summary(fast, 0)['ticks_between_10s_and_18s'], 200)

    def test_visual_difference_ignored_but_velocity_difference_detected(self):
        normal, candidate = self.records(40), self.records(40)
        candidate[5]['rng'] = 999
        candidate[5]['transforms.0.body.position.x'] = 999
        self.assertTrue(physics.compare_vehicle(normal, candidate)['identical'])
        candidate[5]['car.0.velocity.x'] += 1
        difference = physics.compare_vehicle(normal, candidate)['first_difference']
        self.assertEqual(difference['tick'], 5)
        self.assertEqual(difference['field'], 'car.0.velocity.x')


if __name__ == '__main__':
    unittest.main()
