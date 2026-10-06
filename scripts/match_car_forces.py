#!/usr/bin/env python3
"""Compile Car.cpp once and score all three force functions.

Examples:
  python3 scripts/match_car_forces.py --diff
  python3 scripts/match_car_forces.py --source scripts/work/car-forces/candidate.cpp

Candidates are compiled separately and never overwrite the working source.
Each run saves scores, address-bearing assembly and diffs under --output.
The structural score ignores branch destinations only; EXACT uses real bytes.
"""
import argparse
import contextlib
import difflib
import json
from pathlib import Path
import re
import subprocess
import sys
import time

import fastcmp as F
from car_forces_blocks import block_report, print_blocks

TARGETS = (0x4387a0, 0x43b100, 0x441500)


def normalize(instruction):
    return re.sub(r"^(j\w+) 0x[0-9a-f]+$", r"\1 <branch>", instruction)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=Path(F.REPO) / "CMR2Decomp/Car.cpp")
    parser.add_argument("--output", type=Path, default=Path(F.HERE) / "car-forces/latest")
    parser.add_argument("--diff", action="store_true")
    parser.add_argument("--no-compile", action="store_true")
    parser.add_argument("--test", action="store_true", help="Run differential execution after compilation")
    parser.add_argument("--test-functions", default="tyre,suspension,wheel",
                        help="Bodies to execute with --test; scores always cover all three")
    parser.add_argument("--blocks", action="store_true", help="Show diagnostic differences per original address range")
    parser.add_argument("--cases", type=int, default=300, help="Differential cases per function")
    parser.add_argument("--stack-pattern", choices=("uniform", "mixed", "both"), default="both",
                        help="Stack inputs for --test; both exposes misplaced uninitialised reads")
    parser.add_argument("--vectors", choices=("identity", "rotated", "both"), default="both",
                        help="Vector inputs for --test")
    args = parser.parse_args()
    if not set(args.test_functions.split(",")).issubset({"tyre", "suspension", "wheel"}):
        parser.error("--test-functions must contain tyre, suspension or wheel, separated by commas")
    args.output.mkdir(parents=True, exist_ok=True)
    obj = str(args.output.resolve() / "Car.obj")
    start = time.monotonic()
    if not args.no_compile:
        # The basename selects the original translation unit's /QIfist flags.
        source = args.output.resolve() / "Car.cpp"
        if source != args.source.resolve():
            source.write_bytes(args.source.read_bytes())
        F.compile_tu(str(source), obj)
    coff = F.COFF(obj)
    names, sizes = F.load_meta()
    rows = []
    for address in TARGETS:
        score, exact, (original, rebuilt, matcher), unknown = F.compare(
            address, obj, str(Path(F.REPO) / "CMR2Decomp/Car.cpp"),
            names[address], sizes[address], coff=coff)
        structural = difflib.SequenceMatcher(None,
            [normalize(i[2]) for i in original],
            [normalize(i[2]) for i in rebuilt], autojunk=False).ratio()
        row = dict(address=hex(address), name=names[address], score=score,
                   structural=structural, exact=exact, unknown=sorted(unknown),
                   original_size=sizes[address],
                   rebuilt_size=sum(i[1] for i in rebuilt))
        rows.append(row)
        print(f"{names[address]}: {score:.2%}; structure {structural:.2%}; "
              f"{row['rebuilt_size']} bytes" + ("; EXACT" if exact else ""), flush=True)
        if unknown:
            print("  Unmapped symbols:", ", ".join(sorted(unknown)))
        for suffix, instructions in (("orig", original), ("rebuilt", rebuilt)):
            (args.output / f"{names[address]}.{suffix}.asm").write_text(
                "".join(f"{a:08x} {t}\n" for a, _, t in instructions))
        with (args.output / f"{names[address]}.diff").open("w") as stream:
            with contextlib.redirect_stdout(stream):
                F.show_diff(original, rebuilt, matcher)
        if args.diff:
            F.show_diff(original, rebuilt, matcher)
        report = block_report(address, sizes[address], original, rebuilt)
        (args.output / f"{names[address]}.blocks.json").write_text(
            json.dumps(report, indent=2) + "\n")
        if args.blocks:
            print_blocks(report)
    (args.output / "scores.json").write_text(json.dumps(rows, indent=2) + "\n")
    if args.test:
        patterns = ("uniform", "mixed") if args.stack_pattern == "both" else (args.stack_pattern,)
        vectors = ("identity", "rotated") if args.vectors == "both" else (args.vectors,)
        for pattern in patterns:
            for basis in vectors:
                subprocess.run([sys.executable, str(Path(F.REPO) / "tests/differential_car_forces.py"),
                                obj, "--cases", str(args.cases), "--stack-pattern", pattern,
                                "--vectors", basis, "--functions", args.test_functions], check=True)
    print(f"Compile + comparison: {time.monotonic() - start:.2f}s; {args.output}")


if __name__ == "__main__":
    main()
