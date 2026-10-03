#!/usr/bin/env python3
"""Alignment must not extend the original body into adjacent functions."""
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
import fastcmp


class PaddingTests(unittest.TestCase):
    def compare(self, rebuilt, original, size, symbols=None, relocations=None, parsed=False):
        obj = SimpleNamespace(
            secs=[{"name": ".text", "data": rebuilt, "rels": relocations or []}],
            syms=symbols or [],
        )
        reads = []

        def read(address, length):
            self.assertEqual(address, 0x400100)
            reads.append(length)
            return original[:length]

        with patch.object(fastcmp, "COFF", return_value=obj) as parser, \
             patch.object(fastcmp, "find_func", return_value=(0, {"sec": 1})), \
             patch.object(fastcmp, "func_extent", return_value=(0, len(rebuilt))), \
             patch.object(fastcmp, "symmap_for", return_value={}), \
             patch.object(fastcmp, "ORIG", SimpleNamespace(read=read)):
            score, exact, _, unknown = fastcmp.compare(
                0x400100, "test.obj", "test.cpp", "test", size, coff=obj if parsed else None
            )
            if parsed:
                parser.assert_not_called()
            self.assertEqual(obj.secs[0]["data"], rebuilt)
        self.assertFalse(unknown)
        return score, exact, reads

    def test_rebuilt_padding_does_not_read_the_next_original_function(self):
        body = bytes.fromhex("31c0c3")
        score, exact, reads = self.compare(
            body + b"\xcc" * 13, body + b"\xc3\xcc\xcc\xcc\xcc\xff\x25", 3
        )
        self.assertEqual(score, 1)
        self.assertTrue(exact)
        self.assertEqual(reads, [3])

    def test_a_longer_wrong_rebuilt_body_still_fails(self):
        _, exact, reads = self.compare(
            bytes.fromhex("31c040c3") + b"\xcc" * 12,
            bytes.fromhex("31c0c3cc"), 3
        )
        self.assertFalse(exact)
        self.assertEqual(reads, [4])

    def test_missing_original_instructions_still_fail(self):
        _, exact, reads = self.compare(
            b"\xc3" + b"\xcc" * 15, bytes.fromhex("31c0c3"), 3
        )
        self.assertFalse(exact)
        self.assertEqual(reads, [3])

    def test_unknown_original_size_compares_every_real_instruction(self):
        body = bytes.fromhex("31c040c3")
        score, exact, reads = self.compare(body + b"\xcc" * 12, body, None)
        self.assertEqual(score, 1)
        self.assertTrue(exact)
        self.assertEqual(reads, [len(body)])

    def switch(self, swapped=False, parsed=False):
        import struct
        # jmp [eax*4+table], case 0 returns 0, case 1 returns 1, table.
        body = bytes.fromhex("ff24850000000031c0c3b801000000c3")
        symbols = [
            {"name": "$table", "sec": 1, "cls": 3, "val": 16},
            {"name": "$zero", "sec": 1, "cls": 6, "val": 7},
            {"name": "$one", "sec": 1, "cls": 6, "val": 10},
        ]
        original = bytearray(body + bytes(8))
        struct.pack_into("<I", original, 3, 0x400110)
        struct.pack_into("<II", original, 16, 0x400107, 0x40010a)
        order = (2, 1) if swapped else (1, 2)
        relocations = [(3, 0, 6), (16, order[0], 6), (20, order[1], 6)]
        return self.compare(body + bytes(8), bytes(original), 16, symbols, relocations, parsed)

    def test_reusing_object_preserves_relocations_switch_data_and_exactness(self):
        for swapped in (False, True):
            self.assertEqual(self.switch(swapped), self.switch(swapped, parsed=True))

    def test_equal_instructions_with_swapped_switch_targets_are_not_exact(self):
        score, exact, reads = self.switch(swapped=True)
        self.assertFalse(exact)
        self.assertLess(score, 1)
        self.assertEqual(reads, [24])

    def test_equal_switch_targets_and_instructions_are_exact(self):
        score, exact, reads = self.switch()
        self.assertTrue(exact)
        self.assertEqual(score, 1)
        self.assertEqual(reads, [24])


if __name__ == "__main__":
    unittest.main()
