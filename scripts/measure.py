"""Measure a successfully built source snapshot and save reproducible reports."""

import argparse
import csv
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "CMR2PROGRESS")
    args = parser.parse_args()
    manifest = json.loads((ROOT / "build/manifest.json").read_text())
    if manifest["windowed"]:
        parser.error("Matching reports require the default build without --windowed.")
    current_sources = {str(p.relative_to(ROOT)): sha256(p)
                       for p in sorted((ROOT / "CMR2Decomp").rglob("*"))
                       if p.suffix in (".cpp", ".h")}
    if current_sources != manifest["source_sha256"]:
        parser.error("Sources changed since the build; rebuild before measuring.")
    for name in ["exe", "pdb"]:
        if sha256(ROOT / ("build/CMR2." + name)) != manifest[name + "_sha256"]:
            parser.error("Build artifacts changed; rebuild before measuring.")
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    html = ROOT / "index.html" if out == ROOT / "CMR2PROGRESS" else out / "index.html"
    subprocess.run([sys.executable, str(ROOT / "scripts/check_dupes.py"), str(ROOT)], check=True)
    subprocess.run(["reccmp-reccmp", "--target", "CMR2", "--no-color", "--silent",
                    "--json", str(out / "summary.json"), "--json-diet",
                    "--html", str(html)], cwd=ROOT, check=True)
    from reccmp.compare import Compare
    from reccmp.project.detect import argparse_parse_project_target

    comparison = Compare.from_target(argparse_parse_project_target(argparse.Namespace(target="CMR2")))
    entities = {hex(e.orig_addr): [e.recomp_addr, e.name] for e in comparison.get_all()
                if e.orig_addr is not None and e.recomp_addr is not None}
    (out / "entities.json").write_text(json.dumps(entities, indent=2) + "\n")
    data = subprocess.run(["reccmp-datacmp", "--target", "CMR2"], cwd=ROOT,
                          capture_output=True, text=True, check=True)
    (out / "datacmp.log").write_text(data.stdout + data.stderr)
    match = re.search(r"Variables:\s*(\d+)\.\s*Issues:\s*(\d+)", data.stdout)
    if match is None or int(match[2]) != 0:
        raise RuntimeError("Global data comparison failed; see " + str(out / "datacmp.log"))
    subprocess.run([sys.executable, str(ROOT / "audit_byte_matching.py"),
                    str(out / "summary.json"), str(out / "entities.json"),
                    str(ROOT / "build"), str(out / "bytes.json")], cwd=ROOT, check=True)
    report = json.loads((out / "summary.json").read_text())
    functions = [f for f in report["data"] if f.get("type") == 1]
    byte_results = json.loads((out / "bytes.json").read_text())
    if any("err" in v for v in byte_results.values()):
        raise RuntimeError("Byte audit failed to inspect functions; see bytes.json.")
    with (ROOT / "scripts/functions.tsv").open() as handle:
        inventory = {int(r["addr"], 16): r for r in csv.DictReader(handle, delimiter="\t")}
    pending = [(a, v) for a, v in byte_results.items() if not v["x"]]
    scores = {f["address"]: f.get("matching", 0) for f in functions}
    pending.sort(key=lambda row: (scores.get(row[0], 0),
                                 -int(inventory.get(int(row[0], 16), {}).get("size", 0))))
    with (out / "nonmatching.tsv").open("w", newline="") as handle:
        writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
        writer.writerow(["address", "name", "file", "original_bytes", "reccmp_score", "byte_audit_score"])
        for address, result in pending:
            writer.writerow([address, result["n"], result["f"],
                             inventory.get(int(address, 16), {}).get("size", 0),
                             scores.get(address, 0), result["s"]])
    progress = {"build": manifest, "original_sha256": sha256(ROOT / "cmr2bin/CMR2.exe"),
                "inventory_entries": len(inventory), "measured_functions": len(functions),
                "reccmp_exact": sum(f.get("matching") == 1 for f in functions),
                "audited_source_functions": len(byte_results),
                "relocated_byte_exact": sum(v["x"] for v in byte_results.values()),
                "nonmatching_source_functions": len(pending),
                "unresolved_source_functions": [a for a, v in byte_results.items() if v.get("unknown")],
                "data_variables": int(match[1]), "data_issues": int(match[2]),
                "byte_auditor_sha256": sha256(ROOT / "scripts/fastcmp.py")}
    (out / "provenance.json").write_text(json.dumps(progress, indent=2) + "\n")
    print("Source functions:", progress["audited_source_functions"],
          "relocated byte-exact:", progress["relocated_byte_exact"],
          "remaining:", len(pending))


if __name__ == "__main__":
    main()
