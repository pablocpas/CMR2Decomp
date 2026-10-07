#!/usr/bin/env python3
"""Convert the reccmp accuracy summary into an objdiff report for decomp.dev.

decomp.dev reads progress from a GitHub Actions artifact named
``<version>_report`` that contains a ``report.json`` in objdiff's report
format (objdiff-core/protos/report.proto, version 2). reccmp has no such
output, so this builds one from:

  * CMR2PROGRESS/summary.json  - reccmp ``--json`` output (match ratio per function)
  * CMR2PROGRESS/bytes.json    - relocated byte audit from scripts/measure.py
  * scripts/functions.tsv      - original function sizes
  * CMR2Decomp/*.cpp           - ``// FUNCTION: CMR2 0x...`` annotations, giving
                                 the translation unit each function belongs to

When bytes.json is present it is authoritative: only the source functions it
audits are reported (statically linked LIBRARY entries are left out) and a
function counts as matched when it is byte-exact. reccmp alone misses
byte-identical functions whose operands resolve to a neighbouring symbol.
Without it, a reccmp ratio of exactly 1.0 counts as matched.
"""

import argparse
import bisect
import csv
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ANNOTATION = re.compile(r"^//\s*(?:FUNCTION|STUB|LIBRARY|TEMPLATE|SYNTHETIC):\s*CMR2\s+(0x[0-9a-fA-F]+)")
REPORT_VERSION = 2


def load_sizes(path):
    sizes = {}
    with open(path, newline="") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            sizes[int(row["addr"], 16)] = int(row["size"])
    return sizes


def load_units(src_dir):
    units = {}
    for path in sorted(src_dir.rglob("*.cpp")):
        unit = path.relative_to(src_dir).with_suffix("").as_posix()
        with open(path, encoding="utf-8", errors="replace") as f:
            for line in f:
                m = ANNOTATION.match(line)
                if m:
                    units.setdefault(int(m.group(1), 16), unit)
    return units


def measures(funcs):
    total_code = sum(f["size"] for f in funcs)
    matched = [f for f in funcs if f["fuzzy_match_percent"] >= 100.0]
    matched_code = sum(f["size"] for f in matched)
    fuzzy = sum(f["size"] * f["fuzzy_match_percent"] for f in funcs) / total_code if total_code else 0.0

    def pct(n, d):
        return n * 100.0 / d if d else 0.0

    return {
        "fuzzy_match_percent": fuzzy,
        "total_code": str(total_code),
        "matched_code": str(matched_code),
        "matched_code_percent": pct(matched_code, total_code),
        "total_functions": len(funcs),
        "matched_functions": len(matched),
        "matched_functions_percent": pct(len(matched), len(funcs)),
    }


def complete(unit_measures, total_code):
    """objdiff's "complete" (linked) code: units whose every function is exact."""
    code = int(unit_measures["total_code"]) if unit_measures["complete_units"] else 0
    return {"complete_code": str(code), "complete_code_percent": code * 100.0 / total_code if total_code else 0.0}


def build_report(summary, sizes, units, audit, per_function=False):
    starts = sorted(set(sizes) | {int(f["address"], 16) for f in summary})
    by_unit = {}
    for entry in summary:
        addr = int(entry["address"], 16)
        if audit is not None and addr not in audit:
            continue
        size = sizes.get(addr)
        if size is None:
            # Not in the function list: assume it runs up to the next known start.
            i = bisect.bisect_right(starts, addr)
            size = starts[i] - addr if i < len(starts) else 0
        unit = units.get(addr, "unassigned")
        if per_function:
            unit = f"{unit}/{entry['name']}"
            if unit in by_unit:  # overloads share a name
                unit += f"@{addr:x}"
        by_unit.setdefault(unit, []).append({
            "name": entry["name"],
            "size": size,
            # Byte audit: 100 when exact, else the similarity with registers
            # ignored ("fz"); reccmp's score only when there is no audit.
            "fuzzy_match_percent": (100.0 if audit[addr].get("x") else audit[addr].get("fz", audit[addr]["s"]) * 100.0)
                                   if audit else float(entry["matching"]) * 100.0,
            "address": addr,
        })

    report_units = []
    all_funcs = []
    for name in sorted(by_unit):
        funcs = sorted(by_unit[name], key=lambda f: f["address"])
        all_funcs.extend(funcs)
        unit_measures = measures(funcs)
        unit_measures["total_units"] = 1
        unit_measures["complete_units"] = int(unit_measures["matched_functions"] == len(funcs))
        unit_measures.update(complete(unit_measures, int(unit_measures["total_code"])))
        metadata = {"complete": unit_measures["complete_units"] == 1}
        source = name.split("/")[0]
        if source != "unassigned":
            metadata["source_path"] = f"CMR2Decomp/{source}.cpp"
        report_units.append({
            "name": name,
            "measures": unit_measures,
            "functions": [{
                "name": f["name"],
                "size": str(f["size"]),
                "fuzzy_match_percent": f["fuzzy_match_percent"],
                "metadata": {"virtual_address": str(f["address"])},
            } for f in funcs],
            "metadata": metadata,
        })

    total = measures(all_funcs)
    total["total_units"] = len(report_units)
    total["complete_units"] = sum(u["measures"]["complete_units"] for u in report_units)
    complete_code = sum(int(u["measures"]["complete_code"]) for u in report_units)
    total["complete_code"] = str(complete_code)
    total["complete_code_percent"] = complete_code * 100.0 / int(total["total_code"]) if int(total["total_code"]) else 0.0
    return {"measures": total, "units": report_units, "version": REPORT_VERSION, "categories": []}


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--summary", type=Path, default=ROOT / "CMR2PROGRESS/summary.json")
    parser.add_argument("--bytes", type=Path, default=ROOT / "CMR2PROGRESS/bytes.json")
    parser.add_argument("--functions", type=Path, default=ROOT / "scripts/functions.tsv")
    parser.add_argument("--src", type=Path, default=ROOT / "CMR2Decomp")
    parser.add_argument("--units", choices=["file", "function"], default="file",
                        help="one report unit per source file, or per function")
    parser.add_argument("-o", "--output", type=Path, default=ROOT / "build/report.json")
    args = parser.parse_args()

    with open(args.summary) as f:
        summary = json.load(f)["data"]
    audit = None
    if args.bytes.exists():
        with open(args.bytes) as f:
            audit = {int(a, 16): v for a, v in json.load(f).items()}
    report = build_report(summary, load_sizes(args.functions), load_units(args.src), audit,
                          per_function=args.units == "function")

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with open(args.output, "w") as f:
        json.dump(report, f, indent=1)
    m = report["measures"]
    print(f"{args.output}: {m['total_units']} units, {m['matched_functions']}/{m['total_functions']} functions, "
          f"{m['matched_code_percent']:.2f}% code matched, {m['complete_code_percent']:.2f}% complete, "
          f"{m['fuzzy_match_percent']:.2f}% fuzzy")


if __name__ == "__main__":
    main()
