#!/usr/bin/env python3
"""Regression checks for reachable machine-code evidence, without Wine."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from audit_logic import instructions


class EvidenceTests(unittest.TestCase):
    def test_adjacent_function_is_not_counted_after_return(self):
        rows = [(0x100, 1, "ret"), (0x101, 5, "call 0x900"), (0x106, 3, "ret 4")]
        result = instructions(rows)
        self.assertEqual(result["returns"], ["ret"])
        self.assertEqual(result["calls"], {})

    def test_both_conditional_paths_are_inspected(self):
        rows = [
            (0x100, 2, "je 0x108"),
            (0x102, 3, "ret 4"),
            (0x108, 5, "call 0x900"),
            (0x10D, 3, "ret 4"),
        ]
        result = instructions(rows)
        self.assertEqual(result["returns"], ["ret 4"])
        self.assertEqual(result["calls"], {"0x900": 1})

    def test_jump_table_is_explicitly_partial(self):
        rows = [(0x100, 6, "jmp dword ptr [eax*4 + 0x150]"), (0x106, 3, "ret 8")]
        result = instructions(rows)
        self.assertTrue(result["indirect_dispatch_not_fully_traversed"])
        self.assertEqual(result["returns"], [])

    def test_unreachable_padding_is_not_a_branch(self):
        result = instructions([(0x100, 1, "ret"), (0x101, 2, "je 0x103")])
        self.assertEqual(result["branches"], {})


if __name__ == "__main__":
    unittest.main()
