import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from validation_queue import branch_coverage, review_gaps, REVIEW_FIELDS


class BranchCoverageTests(unittest.TestCase):
    def test_one_outcome_is_insufficient(self):
        rows = [(0x100, 2, 'je 0x105'), (0x102, 1, 'ret'), (0x105, 1, 'ret')]
        result = branch_coverage(rows, [(0x100, 0x102)])
        self.assertFalse(result['complete_known_edges'])
        self.assertEqual(result['missing_edges'], [(0x100, 0x105)])

    def test_both_outcomes_are_complete(self):
        rows = [(0x100, 2, 'je 0x105'), (0x102, 1, 'ret'), (0x105, 1, 'ret')]
        result = branch_coverage(rows, [(0x100, 0x102), (0x100, 0x105)])
        self.assertTrue(result['complete_known_edges'])

    def test_indirect_dispatch_cannot_be_certified(self):
        result = branch_coverage([(0x100, 6, 'jmp dword ptr [eax*4 + 0x150]')], [(0x100, 0x108)])
        self.assertTrue(result['cfg_partial'])
        self.assertFalse(result['complete_known_edges'])

    def test_unknown_tail_target_is_partial(self):
        result = branch_coverage([(0x100, 5, 'jmp 0x900')], [(0x100, 0x900)])
        self.assertTrue(result['cfg_partial'])

    def test_unreachable_padding_is_excluded(self):
        result = branch_coverage([(0x100, 1, 'ret'), (0x101, 2, 'je 0x100')], [])
        self.assertTrue(result['complete_known_edges'])
        self.assertEqual(result['known_conditional_edges'], 0)


class ConfidenceReviewTests(unittest.TestCase):
    def setUp(self):
        self.function = {'file': 'FixedPoint.cpp', 'tests_passed': ['differential_fixed_matrix_ops.py']}
        self.manifest = {'source_sha256': {'CMR2Decomp/FixedPoint.cpp': 'cpp', 'CMR2Decomp/FixedPoint.h': 'header'},
                         'compiler_sha256': 'compiler', 'flags': ['/O2'], 'qifist_files': ['FixedPoint.cpp']}
        self.tests = {'test_source_sha256': {'differential_fixed_matrix_ops.py': 'harness'}}
        self.review = dict.fromkeys(REVIEW_FIELDS, 'explicit evidence')
        self.review.update(reviewed=True, original_sha256='original', compiler_sha256='compiler',
                           compiler_flags=['/O2'], qifist_files=['FixedPoint.cpp'], source_sha256='cpp',
                           header_sha256={'CMR2Decomp/FixedPoint.h': 'header'},
                           reviewed_harnesses=['differential_fixed_matrix_ops.py'],
                           harness_sha256={'differential_fixed_matrix_ops.py': 'harness'})

    def gaps(self):
        return review_gaps(self.review, self.function, self.manifest, self.tests, 'original')

    def test_current_explicit_review_is_accepted(self):
        self.assertEqual(self.gaps(), [])

    def test_changed_header_invalidates_review(self):
        self.manifest['source_sha256']['CMR2Decomp/FixedPoint.h'] = 'changed'
        self.assertIn('refresh_behavior_review_for_current_source_and_compiler', self.gaps())

    def test_changed_harness_invalidates_review(self):
        self.tests['test_source_sha256']['differential_fixed_matrix_ops.py'] = 'changed'
        self.assertIn('refresh_behavior_review_for_current_harnesses', self.gaps())

    def test_reviewed_harness_must_pass_for_this_function(self):
        self.function['tests_passed'] = []
        self.assertIn('refresh_behavior_review_for_current_harnesses', self.gaps())

    def test_matching_or_coverage_alone_cannot_replace_review(self):
        self.assertTrue(review_gaps(None, self.function, self.manifest, self.tests, 'original'))
        self.review['input_domain'] = ' '
        self.assertIn('complete_behavior_review', self.gaps())


if __name__ == '__main__':
    unittest.main()
