#!/usr/bin/env python3
"""Run every registered behavior harness against one verified build."""
import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--jobs", type=int, default=3)
    parser.add_argument(
        "--output", type=Path, default=ROOT / "CMR2PROGRESS/logic-tests.json"
    )
    parser.add_argument("--timeout", type=int, default=240)
    parser.add_argument("--coverage-disassembly", type=Path,
                        help="Record original branch edges in Unicorn harnesses using audit disassembly.")
    args = parser.parse_args()
    if min(args.jobs, args.timeout) < 1:
        parser.error("Worker count and timeout must be positive.")
    manifest = json.loads((ROOT / "build/manifest.json").read_text())
    provenance = json.loads((ROOT / "CMR2PROGRESS/provenance.json").read_text())
    if provenance["build"] != manifest or manifest["windowed"]:
        parser.error("Build and measure the default matching executable first.")
    for suffix in ("exe", "pdb"):
        if sha(ROOT / ("build/CMR2." + suffix)) != manifest[suffix + "_sha256"]:
            parser.error("Build artifacts changed since measurement.")
    for path, expected in manifest["source_sha256"].items():
        if sha(ROOT / path) != expected:
            parser.error("Sources changed since measurement: " + path)
    suitepath = ROOT / "tests/logic-targets.json"
    suite = json.loads(suitepath.read_text())
    actual = {p.name for p in (ROOT / "tests").glob("differential_*.py")}
    if actual != set(suite):
        parser.error(
            "Register all harnesses in logic-targets.json: " + str(actual ^ set(suite))
        )
    report = ROOT / "CMR2PROGRESS/summary.json"
    entities = ROOT / "CMR2PROGRESS/entities.json"
    started = time.monotonic()
    result = {
        "exe_sha256": manifest["exe_sha256"],
        "pdb_sha256": manifest["pdb_sha256"],
        "original_sha256": sha(ROOT / "cmr2bin/CMR2.exe"),
        "manifest_sha256": sha(ROOT / "build/manifest.json"),
        "report_sha256": sha(report),
        "entities_sha256": sha(entities),
        "suite_sha256": sha(suitepath),
        "test_source_sha256": {
            p.name: sha(p) for p in sorted((ROOT / "tests").glob("*.py"))
        },
        "results": [],
    }
    disassembly = None
    coverage_dir = args.output.parent / "logic-coverage"
    if args.coverage_disassembly:
        disassembly = json.loads(args.coverage_disassembly.read_text())
        coverage_dir.mkdir(parents=True, exist_ok=True)
        result["coverage_disassembly_sha256"] = sha(args.coverage_disassembly)

    def run(item):
        name, entry = item
        paths = {
            "entities": [entities],
            "report": [report],
            "both": [report, entities],
        }[entry["arguments"]]
        command = [sys.executable, str(ROOT / "tests" / name), *map(str, paths)]
        coverage_path = None
        if disassembly is not None:
            functions = {}
            for address in entry["targets"]:
                if address not in disassembly:
                    continue
                rows = disassembly[address]["original"]
                functions[address] = {
                    "begin": int(address, 16),
                    "end": max(a + size for a, size, _ in rows),
                    "branches": {str(a): text for a, _, text in rows
                                 if text.split(" ", 1)[0].startswith("j")
                                 or text.split(" ", 1)[0].startswith("loop")},
                }
            specpath = coverage_dir / (Path(name).stem + ".spec.json")
            coverage_path = coverage_dir / (Path(name).stem + ".json")
            coverage_path.unlink(missing_ok=True)
            specpath.write_text(json.dumps({
                "original": str(ROOT / "cmr2bin/CMR2.exe"), "functions": functions
            }, indent=2) + "\n")
            command = [sys.executable, str(ROOT / "tests/record_unicorn_coverage.py"),
                       str(specpath), str(coverage_path), *command[1:]]
        begin = time.monotonic()
        try:
            process = subprocess.run(
                command,
                cwd=ROOT,
                capture_output=True,
                text=True,
                timeout=args.timeout,
                env=dict(os.environ, WINEDEBUG="-all"),
            )
            code, output = process.returncode, process.stdout + process.stderr
        except subprocess.TimeoutExpired as error:
            code, output = -1, "Timeout: " + str(error)
        item = {
            "test": name,
            "exit_code": code,
            "output": output.strip(),
            "seconds": time.monotonic() - begin,
            "targets": entry["targets"],
        }
        if coverage_path and coverage_path.exists():
            item["coverage"] = json.loads(coverage_path.read_text())
            item["coverage_sha256"] = sha(coverage_path)
        return item

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(run, item) for item in sorted(suite.items())]
        for future in concurrent.futures.as_completed(futures):
            item = future.result()
            result["results"].append(item)
            result["elapsed_seconds"] = time.monotonic() - started
            args.output.parent.mkdir(parents=True, exist_ok=True)
            temp = args.output.with_suffix(".tmp")
            temp.write_text(json.dumps(result, indent=2) + "\n")
            temp.replace(args.output)
            print(
                ("PASS" if item["exit_code"] == 0 else "FAIL"), item["test"], flush=True
            )
            if item["exit_code"]:
                print(item["output"][-2000:], flush=True)
    failures = [item["test"] for item in result["results"] if item["exit_code"]]
    print(
        f'{len(result["results"])} harnesses, {len(failures)} failures; {args.output}',
        flush=True,
    )
    return bool(failures)


if __name__ == "__main__":
    sys.exit(main())
