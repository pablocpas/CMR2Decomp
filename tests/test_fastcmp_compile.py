#!/usr/bin/env python3
"""Regression checks for fresh-object enforcement, without running Wine."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import fastcmp


class CompileTests(unittest.TestCase):
    def test_failed_compiler_cannot_reuse_existing_object(self):
        with tempfile.TemporaryDirectory() as directory:
            obj = Path(directory) / 'old.obj'
            obj.write_bytes(b'stale')
            result = subprocess.CompletedProcess([], -31, '', 'Wine failed')
            with patch.object(fastcmp.subprocess, 'run', return_value=result):
                with self.assertRaisesRegex(RuntimeError, 'Wine failed'):
                    fastcmp.compile_tu('StageObjects.cpp', str(obj))
            self.assertFalse(obj.exists())

    def test_nonzero_exit_rejects_even_a_new_object(self):
        with tempfile.TemporaryDirectory() as directory:
            obj = Path(directory) / 'new.obj'

            def run(*args, **kwargs):
                obj.write_bytes(b'incomplete')
                return subprocess.CompletedProcess([], 1, '', 'compiler failed')

            with patch.object(fastcmp.subprocess, 'run', side_effect=run):
                with self.assertRaisesRegex(RuntimeError, 'compiler failed'):
                    fastcmp.compile_tu('StageObjects.cpp', str(obj))

    def test_success_requires_a_new_object(self):
        with tempfile.TemporaryDirectory() as directory:
            obj = Path(directory) / 'new.obj'
            with patch.object(fastcmp.subprocess, 'run', return_value=subprocess.CompletedProcess([], 0, '', '')):
                with self.assertRaisesRegex(RuntimeError, 'no fresh object'):
                    fastcmp.compile_tu('StageObjects.cpp', str(obj))

            def run(*args, **kwargs):
                obj.write_bytes(b'fresh')
                return subprocess.CompletedProcess([], 0, '', '')

            with patch.object(fastcmp.subprocess, 'run', side_effect=run):
                fastcmp.compile_tu('StageObjects.cpp', str(obj))
            self.assertEqual(obj.read_bytes(), b'fresh')


if __name__ == '__main__':
    unittest.main()
