#!/usr/bin/env python3
"""Classify every non-exact function by the kind of residual difference.

Compiles nothing: reuses the objects left by `scripts/build.py` and the reports
from `scripts/measure.py`, disassembles original and rebuilt bodies and groups
them:

  A_reg_only   identical instruction sequence once registers are normalised
  B_reordered  same instruction multiset, different order (scheduling)
  C_same_len   same instruction count, different instructions
  C_longer     the rebuilt body has more instructions
  C_shorter    the rebuilt body has fewer instructions (missing code)

Writes `pending-classes.tsv` sorted by reccmp score, for planning waves.
"""

import argparse
import collections
import csv
import json
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
import fastcmp as F  # noqa: E402

REG = re.compile(r"\b(e?[abcd]x|[abcd][lh]|e?[sd]i|e?[bp]|e?sp|st\(?\d\)?|mm\d|xmm\d)\b")


def normalise(text):
    return re.sub(r"\s+", " ", REG.sub("R", text)).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path,
                        default=ROOT.parent / "tools/agents/pending-classes.tsv")
    args = parser.parse_args()
    F.REPO = str(ROOT)
    F.HERE = str(ROOT / "scripts/work")
    names, sizes = F.load_meta()
    pending = list(csv.DictReader((ROOT / "CMR2PROGRESS/nonmatching.tsv").open(),
                                  delimiter="\t"))
    by_file = collections.defaultdict(list)
    for row in pending:
        by_file[row["file"]].append(row)
    rows = []
    for filename, entries in by_file.items():
        obj = ROOT / "build" / (filename[:-4] + ".obj")
        if not obj.is_file():
            continue
        coff = F.COFF(str(obj))
        for entry in entries:
            address = int(entry["address"], 16)
            try:
                _, _, (original, rebuilt, _), _ = F.compare(
                    address, str(obj), str(ROOT / "CMR2Decomp" / filename),
                    entry["name"], sizes.get(address), coff=coff)
            except Exception:
                continue
            ot = [normalise(text) for _, _, text in original]
            rt = [normalise(text) for _, _, text in rebuilt]
            if ot == rt:
                kind = "A_reg_only"
            elif sorted(ot) == sorted(rt):
                kind = "B_reordered"
            elif len(ot) == len(rt):
                kind = "C_same_len"
            elif len(rt) > len(ot):
                kind = "C_longer"
            else:
                kind = "C_shorter"
            rows.append((entry["address"], filename, entry["name"],
                         int(entry["original_bytes"]), float(entry["reccmp_score"]),
                         float(entry["byte_audit_score"]), kind, len(ot), len(rt)))
    rows.sort(key=lambda row: (-row[4], row[0]))
    with args.output.open("w") as handle:
        handle.write("address\tfile\tname\tbytes\treccmp\tbyte_audit\tclass\tinsn_orig\tinsn_ours\n")
        for row in rows:
            handle.write("\t".join(map(str, row)) + "\n")
    counts = collections.Counter(row[6] for row in rows)
    print(f"{len(rows)} pending -> {args.output}")
    for kind, count in counts.most_common():
        print(f"  {kind}: {count}")


if __name__ == "__main__":
    main()
