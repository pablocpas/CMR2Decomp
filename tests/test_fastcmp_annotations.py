"""A renamed global must resolve while another comparison scans annotations."""
import concurrent.futures
from pathlib import Path
import sys
import tempfile
import threading
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
import fastcmp


class AnnotationPublicationTest(unittest.TestCase):
    def test_concurrent_reader_gets_complete_map(self):
        paused = threading.Event()
        release = threading.Event()

        def delayed_files():
            yield 'A.cpp'
            paused.set()
            if not release.wait(5):
                raise RuntimeError('Timed out waiting to resume annotation scan')
            yield 'Z.cpp'

        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'CMR2Decomp'
            source.mkdir()
            (source / 'A.cpp').write_text('// GLOBAL: CMR2 0x00500000\nint first;\n')
            (source / 'Z.cpp').write_text('// GLOBAL: CMR2 0x00500004\nint renamed;\n')
            with patch.object(fastcmp, 'REPO', directory), \
                    patch.object(fastcmp, '_ANNOT', None), \
                    patch.object(fastcmp.os, 'listdir', side_effect=[delayed_files(), ['A.cpp', 'Z.cpp']]):
                with concurrent.futures.ThreadPoolExecutor(1) as pool:
                    writer = pool.submit(fastcmp.annotated_globals)
                    try:
                        self.assertTrue(paused.wait(5))
                        self.assertEqual(fastcmp.annotated_globals(),
                                         {'first': 0x500000, 'renamed': 0x500004})
                    finally:
                        release.set()
                    self.assertEqual(writer.result(),
                                     {'first': 0x500000, 'renamed': 0x500004})


if __name__ == '__main__':
    unittest.main()
