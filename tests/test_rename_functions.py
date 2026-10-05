#!/usr/bin/env python3
"""Validate batch selection, reference updates and rollback without Wine."""
import csv
import hashlib
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
import rename_functions as rename


class RenameTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.cpp = self.root / "CMR2Decomp/Game.cpp"
        self.header = self.root / "CMR2Decomp/Game.h"
        self.test = self.root / "tests/differential_game.py"
        self.inventory = self.root / "scripts/functions.tsv"
        self.write(self.cpp, b'''// Read the selected car.
// FUNCTION: CMR2 0x00401000
int CGame::FUN_00401000(void) { return 7; }
// FUNCTION: CMR2 0x00401010
void FUN_00401010() { CGame::FUN_00401000(); }
// FUNCTION: CMR2 0x00401020
void Game_FUN_00401020() { FUN_00401010(); }
''')
        self.write(self.header, b'''class CGame { static int FUN_00401000(void); };
// FUN_00401000 is the getter.
const char *text = "FUN_00401000";
int prefix_FUN_00401000, FUN_00401000_suffix;
void Game_FUN_00401020();
''')
        self.write(self.test, b'''names = {"CGame::FUN_00401000": 0x401000, "Game_FUN_00401020": 0x401020}
''')
        self.write(self.inventory, b'''addr\tsize\tname\tcallers
00401000\t12\tFUN_00401000\t1
00401010\t13\tFUN_00401010\t2
00401020\t14\tGame_FUN_00401020\t3
''')
        self.audit = {"0x401000": {"x": True, "s": 1.0},
                      "0x401010": {"x": False, "s": 0.9},
                      "0x401020": {"x": True, "s": 1.0}}
        self.write(self.root / "CMR2PROGRESS/bytes.json", json.dumps(self.audit).encode())
        self.row = {"address": 0x401000, "old_name": "CGame::FUN_00401000", "new_name": "GetSelectedCar"}

    def write(self, path, data):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)

    def plan(self, rows=None):
        return rename.make_plan(self.root, rows or [self.row])

    def test_names_calls_headers_tests_and_inventory_change_together(self):
        changes, renames = self.plan()
        self.assertEqual(renames, {"FUN_00401000": "GetSelectedCar"})
        self.assertEqual(set(changes), {self.cpp, self.header, self.test, self.inventory})
        self.assertIn(b"int CGame::GetSelectedCar(void)", changes[self.cpp][1])
        self.assertIn(b"CGame::GetSelectedCar();", changes[self.cpp][1])
        self.assertIn(b'"CGame::GetSelectedCar": 0x401000', changes[self.test][1])
        self.assertIn(b"00401000\t12\tGetSelectedCar\t1", changes[self.inventory][1])
        self.assertIn(b"// GetSelectedCar is the getter.", changes[self.header][1])
        self.assertIn(b'"FUN_00401000"', changes[self.header][1])
        self.assertIn(b"prefix_FUN_00401000, FUN_00401000_suffix", changes[self.header][1])
        self.assertIn(b"// FUNCTION: CMR2 0x00401000", changes[self.cpp][1])

    def test_prefixed_free_function_is_a_complete_identifier(self):
        row = dict(address=0x401020, old_name="Game_FUN_00401020", new_name="Game_UpdateCar")
        changes, _ = self.plan([row])
        for path in (self.cpp, self.header, self.test, self.inventory):
            self.assertIn(b"Game_UpdateCar", changes[path][1])
            self.assertNotIn(b"Game_FUN_00401020", changes[path][1])

    def test_tool_fixtures_are_not_game_symbol_references(self):
        fixture = self.root / "tests/test_rename_functions.py"
        original = b'fixture = "CGame::FUN_00401000"\n'
        self.write(fixture, original)
        changes, _ = self.plan()
        self.assertNotIn(fixture, changes)
        self.assertEqual(fixture.read_bytes(), original)
        self.assertIn(self.test, changes)

    def test_rejects_nonexact_and_stale_names(self):
        cases = [({"address": 0x401010, "old_name": "FUN_00401010", "new_name": "Game_Update"}, "not byte-exact"),
                 (dict(self.row, old_name="Wrong"), "Stale map"),
                 (dict(self.row, address=0x500000), "Stale map")]
        for row, error in cases:
            with self.subTest(row=row), self.assertRaisesRegex(ValueError, error):
                self.plan([row])

    def test_rejects_scope_moves_keywords_and_collisions(self):
        for name, error in [("Other::GetSelectedCar", "between scopes"),
                            ("return", "Reserved"), ("__getter", "Reserved"),
                            ("FUN_00401010", "already in use"), ("bad-name", "Invalid"),
                            ("CGame::~CGame", "Invalid")]:
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, error):
                self.plan([dict(self.row, new_name=name)])

    def test_rejects_duplicate_addresses_targets_and_ambiguous_methods(self):
        with self.assertRaisesRegex(ValueError, "Duplicate map address"):
            self.plan([self.row, self.row])
        second = dict(address=0x401020, old_name="Game_FUN_00401020", new_name="GetSelectedCar")
        with self.assertRaisesRegex(ValueError, "already in use"):
            self.plan([self.row, second])
        with self.cpp.open("ab") as handle:
            handle.write(b"// FUNCTION: CMR2 0x00401030\nint Other::FUN_00401000() { return 1; }\n")
        with self.assertRaisesRegex(ValueError, "Ambiguous identifier"):
            self.plan()

    def test_map_skips_blank_rows_and_accepts_qualified_new_names(self):
        path = self.root / "map.tsv"
        path.write_text("address\told_name\tnew_name\tevidence\n"
                        "0x00401000\tCGame::FUN_00401000\tCGame::GetSelectedCar\tgetter\n"
                        "0x00401010\tFUN_00401010\t\t\n")
        rows = rename.read_map(path)
        self.assertEqual(len(rows), 1)
        self.plan(rows)
        path.write_text("address\told_name\tnew_name\n0x401000\tCGame::FUN_00401000\t\n")
        with self.assertRaisesRegex(ValueError, "nothing to rename"):
            rename.read_map(path)

    def test_function_pointers_braces_in_literals_and_preprocessor_branches(self):
        self.cpp.write_text('''// FUNCTION: CMR2 0x00401000
void Callback_Set(void (*callback)(void)) {
    puts("}"); // } {
#ifdef FORCE
    if (a) {
#else
    if (b) {
#endif
        callback();
    }
}
// FUNCTION: CMR2 0x00401010
__declspec(noinline) int Next() { return 1; }
''')
        functions = rename.functions(self.root)
        first = functions[0x401000]
        self.assertEqual(first.name, "Callback_Set")
        self.assertTrue(rename.read_text(self.cpp)[first.begin:first.end].endswith("    }\n}"))
        self.assertEqual(functions[0x401010].name, "Next")

    def test_export_includes_only_exact_unnamed_and_preserves_existing_map(self):
        output = self.root / "map.tsv"
        with patch.object(rename, "require_fresh"):
            rename.export_map(self.root, output)
            original = output.read_bytes()
            with self.assertRaises(FileExistsError):
                rename.export_map(self.root, output)
        self.assertEqual(output.read_bytes(), original)
        rows = list(csv.DictReader(io.StringIO(output.read_text()), delimiter="\t"))
        self.assertEqual([r["address"] for r in rows], ["0x00401000", "0x00401020"])
        self.assertEqual(rows[0]["comment"], "Read the selected car.")

    def test_duplicate_annotations_are_rejected(self):
        self.cpp.write_bytes(self.cpp.read_bytes() + b"// FUNCTION: CMR2 0x00401000\nvoid Dup() {}\n")
        with self.assertRaisesRegex(ValueError, "Duplicate FUNCTION"):
            rename.functions(self.root)

    def test_cpp_literal_escapes_and_legacy_bytes_are_preserved(self):
        text = 'void FUN_00401000() { puts("escaped \\\" FUN_00401000"); }\r\n// FUN_00401000\r\n'
        changed = rename.replace_cpp(text, {"FUN_00401000": "Game_Get"})
        self.assertIn('puts("escaped \\\" FUN_00401000")', changed)
        self.assertEqual(changed.count("\r\n"), 2)
        self.write(self.header, b"// legacy \xe9 FUN_00401000\r\n")
        changes, _ = self.plan()
        self.assertEqual(changes[self.header][1], b"// legacy \xe9 GetSelectedCar\r\n")

    def test_audit_detects_lost_exact_score_changes_and_missing_functions(self):
        before = rename.load_audit(self.root)
        rename.check_audit(before, before)
        for after in [dict(before, **{}), {0x401000: before[0x401000]}]:
            if after == before:
                after = dict(after)
                after[0x401000] = {"x": False, "s": 0.99}
            with self.assertRaises(ValueError):
                rename.check_audit(before, after)
        after = dict(before)
        after[0x401010] = {"x": False, "s": 0.8}
        with self.assertRaisesRegex(ValueError, "lower byte scores"):
            rename.check_audit(before, after)

    def test_freshness_rejects_source_changes_and_stale_report(self):
        for suffix in ("exe", "pdb"):
            self.write(self.root / ("build/CMR2." + suffix), b"binary")
        self.write(self.root / "cmr2bin/CMR2.exe", b"original")
        manifest = {"windowed": False, "source_sha256": {
            str(p.relative_to(self.root)): hashlib.sha256(p.read_bytes()).hexdigest()
            for p in rename.source_paths(self.root)},
            "exe_sha256": hashlib.sha256(b"binary").hexdigest(),
            "pdb_sha256": hashlib.sha256(b"binary").hexdigest()}
        provenance = {"build": manifest, "original_sha256": hashlib.sha256(b"original").hexdigest()}
        self.write(self.root / "build/manifest.json", json.dumps(manifest).encode())
        self.write(self.root / "CMR2PROGRESS/provenance.json", json.dumps(provenance).encode())
        rename.require_fresh(self.root)
        self.header.write_bytes(self.header.read_bytes() + b"// edit\n")
        with self.assertRaisesRegex(ValueError, "Sources changed"):
            rename.require_fresh(self.root)

    def test_failed_batch_restores_dirty_input_and_artifacts(self):
        changes, names = self.plan()
        build = self.root / "build/CMR2.exe"
        report = self.root / "CMR2PROGRESS/bytes.json"
        self.write(build, b"old executable")
        self.write(self.root / "build/Game.obj", b"old object")
        self.write(self.root / "build/scratch/valuable.txt", b"experiment")
        original = {p: p.read_bytes() for p in set(changes) | {build, report, self.root / "build/Game.obj"}}

        def failed_run(root, script, *args):
            build.unlink()
            self.write(self.root / "build/New.obj", b"failed build output")
            report.write_text("corrupted measurement")
            raise subprocess.CalledProcessError(1, [script])

        with patch.object(rename, "require_fresh"), patch.object(rename, "object_code", return_value={}), \
                patch.object(rename, "run", side_effect=failed_run):
            with self.assertRaises(subprocess.CalledProcessError):
                rename.apply_plan(self.root, changes, names, 3)
        for path, data in original.items():
            self.assertEqual(path.read_bytes(), data)
        self.assertFalse((self.root / "build/New.obj").exists())
        self.assertEqual((self.root / "build/scratch/valuable.txt").read_bytes(), b"experiment")

    def test_success_runs_build_measure_suite_and_refresh(self):
        changes, names = self.plan()
        with patch.object(rename, "require_fresh"), patch.object(rename, "object_code", return_value={}), \
                patch.object(rename, "run") as run:
            rename.apply_plan(self.root, changes, names, 3)
        self.assertEqual([call.args[1] for call in run.call_args_list],
                         ["scripts/build.py", "scripts/measure.py", "tests/run_differential_suite.py", "scripts/prepare_fastcmp.py"])
        for path, (_, new) in changes.items():
            self.assertEqual(path.read_bytes(), new)

    def test_object_code_difference_rolls_back_before_measurement(self):
        changes, names = self.plan()
        with patch.object(rename, "require_fresh"), \
                patch.object(rename, "object_code", side_effect=[{"Game.obj": b"old"}, {"Game.obj": b"changed"}]), \
                patch.object(rename, "run") as run:
            with self.assertRaisesRegex(ValueError, "Object code"):
                rename.apply_plan(self.root, changes, names, 3)
        self.assertEqual(len(run.call_args_list), 1)
        for path, (old, _) in changes.items():
            self.assertEqual(path.read_bytes(), old)

    def test_late_suite_failure_restores_new_measurements_and_metadata(self):
        changes, names = self.plan()
        report = self.root / "CMR2PROGRESS/bytes.json"
        original_report = report.read_bytes()
        metadata = self.root / "scripts/work/cur.json"
        self.write(metadata, b"old metadata")

        def run(root, script, *args):
            if script == "scripts/measure.py":
                measured = dict(self.audit)
                measured["0x401000"] = dict(measured["0x401000"], n="CGame::GetSelectedCar")
                report.write_text(json.dumps(measured))
                metadata.write_bytes(b"changed metadata")
                self.write(self.root / "CMR2PROGRESS/summary.json", b"new summary")
            if script == "tests/run_differential_suite.py":
                raise subprocess.CalledProcessError(1, [script])

        with patch.object(rename, "require_fresh"), patch.object(rename, "object_code", return_value={}), \
                patch.object(rename, "run", side_effect=run):
            with self.assertRaises(subprocess.CalledProcessError):
                rename.apply_plan(self.root, changes, names, 3)
        self.assertEqual(report.read_bytes(), original_report)
        self.assertEqual(metadata.read_bytes(), b"old metadata")
        self.assertFalse((self.root / "CMR2PROGRESS/summary.json").exists())
        for path, (old, _) in changes.items():
            self.assertEqual(path.read_bytes(), old)


if __name__ == "__main__":
    unittest.main()
