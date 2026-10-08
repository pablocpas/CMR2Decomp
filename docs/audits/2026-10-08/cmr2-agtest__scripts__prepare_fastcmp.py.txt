#!/usr/bin/env python3
"""Write the fastcmp work directory from the current build reports.

`fastcmp.py` reads `scripts/work/{cur,ent}.json`. Regenerate them after
`build.py` and `measure.py` so a single translation unit can be compared in
about a second without a full reccmp run.
"""

import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "scripts/work")
    args = parser.parse_args()
    report = json.loads((ROOT / "CMR2PROGRESS/summary.json").read_text())
    audit = json.loads((ROOT / "CMR2PROGRESS/bytes.json").read_text())
    entities = json.loads((ROOT / "CMR2PROGRESS/entities.json").read_text())
    for entry in report["data"]:
        if entry["address"] in audit:
            entry["name"] = audit[entry["address"]]["symbol_name"]
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "cur.json").write_text(json.dumps(report))
    (args.output / "ent.json").write_text(
        json.dumps([{"o": int(a, 16), "r": v[0], "n": v[1]} for a, v in entities.items()])
    )
    print("fastcmp metadata written to", args.output)


if __name__ == "__main__":
    main()
