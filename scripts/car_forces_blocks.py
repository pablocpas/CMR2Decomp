"""Diagnostic alignment of the force functions, compiled as whole functions.

The diagnostic alignment masks branch destinations and EBP displacements only.
It never changes the real byte comparison or considers a masked match exact.
Ranges belong to the ORIGINAL body; repeated instructions can make alignment
ambiguous, so always inspect the saved address-bearing assembly before editing.
"""
from bisect import bisect_right
from collections import Counter, defaultdict
import difflib
import re


RANGES = {
    0x4387a0: (
        (0x4387a0, "initialisation / brakes"),
        (0x43893c, "project / normalise wheel axes"),
        (0x43914a, "time / grip scales"),
        (0x439196, "grounded wheel spin loop"),
        (0x439682, "longitudinal / lateral slip"),
        (0x4398c7, "combine / normalise forces"),
        (0x43a252, "friction ellipse"),
        (0x43a67b, "rolling drag / grounded return"),
        (0x43a7de, "airborne wheel spin loop"),
    ),
    0x43b100: (
        (0x43b100, "contact / acceleration"),
        (0x43b277, "lean / geometry / steering"),
    ),
    0x441500: (
        (0x441500, "axle brakes"),
        (0x441624, "wheel axes / rear normalisation"),
        (0x4417e7, "time / grip scales"),
        (0x4418a9, "wheel spin loop"),
        (0x441d89, "mirror spin / force loop setup"),
        (0x441e63, "slip / combine / normalise forces"),
        (0x442839, "friction ellipse / force scale"),
        (0x442ce2, "rolling drag / loop end"),
        (0x442e24, "mirror forces / return"),
    ),
}

STACK = re.compile(r"\[(ebp(?: \+ [a-z]{3}(?:\*\d+)?)?) - (0x[0-9a-f]+|\d+)\]")
BRANCH = re.compile(r"^(j\w+) 0x[0-9a-f]+$")


def shape(instruction):
    return STACK.sub(r"[\1 - <slot>]", BRANCH.sub(r"\1 <target>", instruction))


def block_report(address, original_size, original, rebuilt):
    original = [i for i in original if i[0] < address + original_size]
    regions = RANGES[address]
    starts = [r[0] for r in regions]
    counts = [Counter() for _ in regions]
    examples = [[] for _ in regions]
    instruction_examples = [[] for _ in regions]
    slots = [defaultdict(Counter) for _ in regions]

    def region(index):
        at = original[min(index, len(original) - 1)][0]
        return max(0, bisect_right(starts, at) - 1)

    def record(index, kind, left=None, right=None):
        n = region(index)
        counts[n][kind] += 1
        if kind != "identical_text" and len(examples[n]) < 12:
            examples[n].append(dict(kind=kind,
                original=None if left is None else dict(address=hex(left[0]), instruction=left[2]),
                rebuilt=None if right is None else dict(address=hex(right[0]), instruction=right[2])))
        if kind == "instructions" and len(instruction_examples[n]) < 16:
            instruction_examples[n].append(dict(
                original=None if left is None else dict(address=hex(left[0]), instruction=left[2]),
                rebuilt=None if right is None else dict(address=hex(right[0]), instruction=right[2])))

    matcher = difflib.SequenceMatcher(None,
        [shape(i[2]) for i in original], [shape(i[2]) for i in rebuilt], autojunk=False)
    for tag, a, b, c, d in matcher.get_opcodes():
        if tag == "equal":
            for i, j in zip(range(a, b), range(c, d)):
                left, right = original[i], rebuilt[j]
                if left[2] == right[2]:
                    kind = "identical_text"
                elif BRANCH.match(left[2]):
                    kind = "branch_address"
                else:
                    kind = "stack_operand"
                record(i, kind, left, right)
                for x, y in zip(STACK.finditer(left[2]), STACK.finditer(right[2])):
                    slots[region(i)][x[2]][y[2]] += 1
        else:
            # Pair changed runs in order for diagnostics, without asserting
            # those instructions are semantically or byte equivalent.
            for k in range(max(b - a, d - c)):
                left = original[a + k] if a + k < b else None
                right = rebuilt[c + k] if c + k < d else None
                record(a + k if left else min(b, len(original) - 1),
                       "instructions", left, right)
    blocks = []
    for n, (at, label) in enumerate(regions):
        blocks.append(dict(label=label, original_start=hex(at),
            original_end=hex(starts[n + 1] if n + 1 < len(starts) else address + original_size),
            counts=dict(counts[n]), examples=examples[n],
            instruction_examples=instruction_examples[n],
            stack_correspondences={x: dict(y.most_common()) for x, y in slots[n].items()}))
    return dict(diagnostic_only=True, function=hex(address),
                masked_instruction_similarity=matcher.ratio(), blocks=blocks)


def print_blocks(report):
    print("  Diagnostic ranges: text equal / stack operand / branch address / instructions changed")
    for block in report["blocks"]:
        c = block["counts"]
        print(f"    {block['original_start']}: {block['label']}: "
              f"{c.get('identical_text', 0)} / {c.get('stack_operand', 0)} / "
              f"{c.get('branch_address', 0)} / {c.get('instructions', 0)}", flush=True)
