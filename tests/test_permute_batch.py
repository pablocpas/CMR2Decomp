#!/usr/bin/env python3
"""Check function boundaries and mutation hazards without running Wine."""
import importlib.util
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
import permute_batch as batch


class FunctionRegionTests(unittest.TestCase):
    def test_hex_case_and_leading_zero_are_irrelevant(self):
        for annotation in ("0x00401AB0", "0x401ab0"):
            source = "// FUNCTION: CMR2 " + annotation + "\nvoid f() { return; }"
            begin, end = batch.func_region(source, 0x401AB0)
            self.assertEqual(source[begin:end], "{ return; }")

    def test_inline_brace_stays_in_annotated_function(self):
        source = """// FUNCTION: CMR2 0x00401000
void first() { if (a) { call(); } }
// FUNCTION: CMR2 0x00401010
void second()
{
    other();
}
"""
        begin, end = batch.func_region(source, 0x401000)
        self.assertEqual(source[begin:end], "{ if (a) { call(); } }")

    def test_comment_and_string_braces_do_not_end_body(self):
        source = """// FUNCTION: CMR2 0x00401000
void first()
{
    // }
    /* { } } */
    puts("escaped \\\" } {");
    char c = '}';
    if (a) { call(); }
}
void second() { other(); }
"""
        begin, end = batch.func_region(source, 0x401000)
        self.assertTrue(source[begin:end].endswith("    if (a) { call(); }\n}"))
        self.assertNotIn("second", source[begin:end])

    def test_unterminated_body_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "Unterminated"):
            batch.func_region("// FUNCTION: CMR2 0x00401000\nvoid f() {", 0x401000)


class AddressFilterTests(unittest.TestCase):
    def test_accepts_plain_and_prefixed_addresses(self):
        self.assertEqual(
            batch.parse_addresses("0x46b440, 0x0048ce80, 1234"),
            {0x46B440, 0x48CE80, 1234},
        )

    def test_rejects_non_numeric_entries(self):
        with self.assertRaisesRegex(ValueError, "comma-separated"):
            batch.parse_addresses("0x46b440,zz")


class MutationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        mutator = batch.ROOT.parent / "tools/fastcmp/permute.py"
        if not mutator.is_file():
            raise unittest.SkipTest("Optional external mutator is not installed")
        spec = importlib.util.spec_from_file_location("test_mutators", mutator)
        batch.P = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(batch.P)
        batch.P.SAFE = True
        batch.P.negate = lambda expression: "!(" + expression + ")"

    def kinds(self, body):
        return {kind for kind, _, _ in batch.mutations(body)}

    def test_wrapped_branch_calls_are_swapped_as_whole_statements(self):
        body = "{\n    if (flag)\n        a(1,\n          2);\n    else\n        b(3,\n          4);\n    after();\n}"
        variants = batch.multiline_ifswaps(body)
        self.assertEqual(len(variants), 1)
        self.assertEqual(variants[0][2], "{\n    if (!(flag))\n        b(3,\n          4);\n    else\n        a(1,\n          2);\n    after();\n}")

    def test_nested_unbraced_if_is_not_rebound_to_outer_else(self):
        body = "{\n    if (outer)\n        if (inner)\n            a();\n    else\n        b();\n}"
        self.assertEqual(batch.multiline_ifswaps(body), [])

    def test_semicolons_and_braces_in_strings_are_not_syntax(self):
        body = '{\n    if (flag)\n        a(";{}",\n          2);\n    else\n        b();\n}'
        self.assertEqual(len(batch.multiline_ifswaps(body)), 1)

    def test_two_statements_on_one_line_are_rejected(self):
        body = "{\n    if (flag)\n        a(); b();\n    else\n        c();\n}"
        self.assertEqual(batch.multiline_ifswaps(body), [])

    def test_parentheses_must_balance_before_else(self):
        body = "{\n    if (flag)\n        a(1;\n    else\n        b();\n}"
        self.assertEqual(batch.multiline_ifswaps(body), [])

    def test_dependent_statements_are_not_moved(self):
        body = "{\n    int x;\n    int y;\n    x = 1;\n    y = x + 2;\n}"
        self.assertNotIn("move", self.kinds(body))

    def test_independent_statements_can_move(self):
        body = "{\n    int x;\n    int y;\n    x = 1;\n    y = 2;\n}"
        self.assertIn("move", self.kinds(body))

    def test_call_and_memory_store_are_not_moved(self):
        for statement in ("x = call();", "p->x = 1;"):
            body = "{\n    int x;\n    int y;\n    " + statement + "\n    y = 2;\n}"
            self.assertNotIn("move", self.kinds(body))

    def test_float_expression_order_is_preserved_including_multi_declarations(self):
        for expression in ("other", "(int)(__int64)(value * 65536.0)"):
            body = "{\n    float value, other;\n    int x;\n    int y;\n    " + ("other = value;" if expression == "other" else "x = " + expression + ";") + "\n    y = 2;\n}"
            self.assertNotIn("move", self.kinds(body))

    def test_integer_reordering_in_float_function_remains_available(self):
        body = "{\n    float value;\n    int x;\n    int y;\n    x = 1;\n    y = 2;\n}"
        self.assertIn("move", self.kinds(body))

    def test_address_taken_local_is_not_moved(self):
        body = "{\n    int x;\n    int y;\n    consume(&x);\n    x = 1;\n    y = 2;\n}"
        self.assertNotIn("move", self.kinds(body))

    def test_side_effecting_operands_are_not_swapped(self):
        self.assertNotIn(
            "swap", self.kinds("{\n    int x;\n    int y;\n    x = y++ + x;\n}")
        )
        self.assertNotIn("swap", self.kinds("{\n    int x;\n    x = a() + b();\n}"))

    def test_narrow_nested_helper_does_not_truncate_intermediate(self):
        narrow = "{\n    short x;\n    x = FixMul(FixDiv(123, 456), 789);\n}"
        wide = narrow.replace("short x", "int x")
        self.assertNotIn("unnest", self.kinds(narrow))
        self.assertIn("unnest", self.kinds(wide))

    def test_initialized_declarations_preserve_dependencies(self):
        dependent = "{\n    int a = 1;\n    int b = a;\n}"
        independent = dependent.replace("b = a", "b = 2")
        self.assertNotIn("init", {k for k, _, _ in batch.forms(dependent)})
        self.assertIn("init", {k for k, _, _ in batch.forms(independent)})

    def test_volatile_function_has_no_candidates(self):
        body = "{\n    volatile int x = 1;\n    int y = 2;\n    x = 3;\n    y = 4;\n}"
        self.assertEqual(batch.mutations(body) + batch.forms(body), [])

    def test_layout_never_moves_initializer_or_constructor(self):
        for statement in ("int x = call();", "Widget x;", "int x[n];"):
            body = "{\n    " + statement + "\n    int y;\n}"
            self.assertEqual(batch.layout_forms(body), [])

    def test_layout_stays_at_function_entry(self):
        body = "{\n    Car *p;\n    FixVector v;\n    int n;\n    n = 0;\n    int later;\n}"
        choices = batch.layout_forms(body)
        self.assertTrue(choices)
        for _, _, candidate in choices:
            self.assertIn("    n = 0;\n    int later;", candidate)
            self.assertEqual(sorted(body.splitlines()), sorted(candidate.splitlines()))

    def test_pack_has_one_variant_per_function_and_covers_every_choice(self):
        variants = {0x401000: ["a", "b", "c"], 0x401010: ["x"], 0x401020: []}
        plan = batch.packed_plan(variants)
        self.assertEqual(plan, [[(0x401000, "a"), (0x401010, "x")], [(0x401000, "b")], [(0x401000, "c")]])

    def test_multiple_replacements_preserve_neighbour_body_and_signatures(self):
        source = "// FUNCTION: CMR2 0x401000\nvoid a() { first(); }\n// FUNCTION: CMR2 0x401010\nint b(int x) { return x; }\n// FUNCTION: CMR2 0x401020\nvoid c() { third(); }"
        changed = batch.replace_bodies(source, {0x401000: "{ longer_first(); }", 0x401020: "{ last(); }"})
        self.assertIn("int b(int x) { return x; }", changed)
        self.assertIn("void a() { longer_first(); }", changed)
        self.assertIn("void c() { last(); }", changed)


if __name__ == "__main__":
    unittest.main()
