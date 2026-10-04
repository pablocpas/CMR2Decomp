#!/usr/bin/env python3
"""Run every registered behavior harness against one verified build, printing results only."""
import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--jobs", type=int, default=3)
    parser.add_argument("--timeout", type=int, default=240)
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
    def run(item):
        name, entry = item
        paths = {
            "entities": [entities],
            "report": [report],
            "both": [report, entities],
        }[entry["arguments"]]
        command = [sys.executable, str(ROOT / "tests" / name), *map(str, paths)]
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
        }
        return item

    failures = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(run, item) for item in sorted(suite.items())]
        for future in concurrent.futures.as_completed(futures):
            item = future.result()
            print(
                ("PASS" if item["exit_code"] == 0 else "FAIL"), item["test"], flush=True
            )
            if item["exit_code"]:
                failures.append(item["test"])
                print(item["output"][-2000:], flush=True)
    print(
        f'{len(suite)} harnesses, {len(failures)} failures',
        flush=True,
    )
    return bool(failures)


if __name__ == "__main__":
    sys.exit(main())
