#!/usr/bin/env python3
"""Capture/compare real-stage traces. Uses only Python's standard library."""
import argparse
import csv
import hashlib
import itertools
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    with path.open("rb") as file:
        return hashlib.file_digest(file, "sha256").hexdigest() if hasattr(hashlib, "file_digest") else hashlib.sha256(file.read()).hexdigest()


def difference(a, b, path=""):
    if type(a) is not type(b):
        return {"field": path, "reference": a, "actual": b}
    if isinstance(a, dict):
        if a.keys() != b.keys():
            return {"field": path + ".keys", "reference": list(a), "actual": list(b)}
        for key in a:
            result = difference(a[key], b[key], f"{path}.{key}" if path else key)
            if result:
                return result
    elif isinstance(a, list):
        if len(a) != len(b):
            return {"field": path + ".length", "reference": len(a), "actual": len(b)}
        for index, (left, right) in enumerate(zip(a, b)):
            result = difference(left, right, f"{path}[{index}]")
            if result:
                return result
    elif a != b:
        return {"field": path, "reference": a, "actual": b}
    return None


def compare(reference, actual):
    first = first_car = None
    count = 0
    with Path(reference).open() as left, Path(actual).open() as right:
        for count, (a, b) in enumerate(itertools.zip_longest(left, right), 1):
            if a is None or b is None:
                return {"ticks": count - 1, "first_difference": first or {"tick": count - 1, "field": "trace.length"}, "first_car_difference": first_car, "length_mismatch": True}
            a, b = json.loads(a), json.loads(b)
            if a.get("tick") != count - 1 or b.get("tick") != count - 1:
                raise ValueError(f"trace tick sequence is invalid at row {count}")
            if not first:
                found = difference(a, b)
                if found:
                    first = {"tick": count - 1, **found}
            if not first_car:
                found = difference(a["cars"], b["cars"], "cars")
                if found:
                    first_car = {"tick": count - 1, **found}
    if not count:
        raise ValueError("empty trace")
    return {"ticks": count, "first_difference": first, "first_car_difference": first_car}


def describe(result):
    first = result["first_difference"]
    if first is None:
        return f"identical: {result['ticks']} ticks"
    detail = f"tick {first['tick']}, {first['field']}: {first.get('reference')} -> {first.get('actual')}"
    car = result["first_car_difference"]
    if car and car != first:
        detail += f"; car first differs at tick {car['tick']}, {car['field']}"
    return detail


def export_inputs(trace, path):
    with trace.open() as source, path.open("w", newline="") as destination:
        writer = csv.writer(destination, lineterminator="\n")
        writer.writerow("tick,state,left,right,throttle,brake,handbrake,steering,pedal,shift0,shift1,shift2,shift3".split(","))
        for line in source:
            record = json.loads(line)
            controls = record["input"]
            writer.writerow([record["tick"], record["state"], *controls["digital"], controls["handbrake"], controls["steering"], controls["pedal"], *controls["shift"]])


def run(args):
    data, probe, output = args.data.resolve(), args.probe.resolve(), args.output.resolve()
    if not data.is_dir() or not probe.is_file():
        raise ValueError("data directory or stage_probe executable missing; build the linux-x86 preset first")
    if not 126 <= args.ticks <= 100000:
        raise ValueError("use 126..100000 ticks (the first 125 normally cover the countdown)")
    rates = args.rates.split(",")
    if any(rate != "variable" and (not rate.isdecimal() or not 10 <= int(rate) <= 1000) for rate in rates):
        raise ValueError("rates must be comma-separated integers 10..1000 or variable")
    output.mkdir(parents=True, exist_ok=True)
    if (output / "report.json").exists() or any(output.glob("*.jsonl")):
        raise ValueError("output contains an earlier baseline; choose a new output directory")
    names = [("reference", "60"), ("repeat", "60"), ("playback", "60")] + [(f"fps-{rate}", rate) for rate in dict.fromkeys(rates)]
    for name, rate in names:
        print(f"Running {name}: {rate} FPS, {args.ticks} ticks", flush=True)
        with tempfile.TemporaryDirectory(prefix="opencmr2-probe-") as user:
            env = dict(os.environ, XDG_DATA_HOME=user, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy", OPENCMR2_HEADLESS="1")
            with (output / f"{name}.log").open("w") as log:
                command = [str(probe), str(data), str(output / f"{name}.jsonl"), rate, str(args.ticks)]
                if name == "playback" or name.startswith("fps-"):
                    command.append(str(output / "inputs.csv"))
                result = subprocess.run(command, env=env,
                                        stdout=log, stderr=subprocess.STDOUT, timeout=args.timeout)
            if result.returncode:
                raise ValueError(f"{name} failed ({result.returncode}); inspect {output / (name + '.log')}")
            if name == "reference":
                export_inputs(output / "reference.jsonl", output / "inputs.csv")
    report = {"schema": 1, "scenario": "Finland stage 1; fresh settings; default 4WD; automatic gearbox; tick input script v1",
              "scope": "Selected car/control/contact/suspension/drivetrain/damage/checkpoint fields and legacy RNG; no original-executable equivalence claim",
              "ticks": args.ticks, "probe_sha256": digest(probe), "data_directory": str(data), "comparisons": {}}
    reference = output / "reference.jsonl"
    report["reference_sha256"] = digest(reference)
    for name, _ in names[1:]:
        result = compare(reference, output / f"{name}.jsonl")
        report["comparisons"][name] = result
        print(f"{name}: {describe(result)}")
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"Traces and report: {output}")
    if any(report["comparisons"][name]["first_difference"] for name in ("repeat", "playback")):
        return 1
    return int(args.require_cadence and any(result["first_difference"] for result in report["comparisons"].values()))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    capture = commands.add_parser("run", help="load Finland through the real game; compare repeat and render cadences")
    capture.add_argument("--data", type=Path, required=True)
    capture.add_argument("--probe", type=Path, default=ROOT / "build/linux-x86/tests/stage_probe")
    capture.add_argument("--output", type=Path, required=True)
    capture.add_argument("--ticks", type=int, default=500)
    capture.add_argument("--rates", default="30,60,120,144,240,variable")
    capture.add_argument("--timeout", type=int, default=120)
    capture.add_argument("--require-cadence", action="store_true", help="fail on any cross-rate difference; otherwise report these baseline findings")
    comparison = commands.add_parser("compare", help="compare traces and return nonzero on the first differing field")
    comparison.add_argument("reference", type=Path)
    comparison.add_argument("actual", type=Path)
    args = parser.parse_args()
    try:
        if args.command == "run":
            return run(args)
        result = compare(args.reference, args.actual)
        print(describe(result))
        return int(result["first_difference"] is not None)
    except (OSError, ValueError, subprocess.TimeoutExpired) as error:
        print(f"baseline failed: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
