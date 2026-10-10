#!/usr/bin/env python3
"""Summarise OPENCMR2_PROFILE CSV CPU timings; no external dependencies."""
import argparse
import csv
import math
from pathlib import Path
import sys


def percentile(values, fraction):
    return values[max(0, math.ceil(len(values) * fraction) - 1)]


def summarise(path, budget=16.667, start_frame=0):
    with path.open(newline="") as file:
        rows = [row for row in csv.DictReader(file) if int(row["frame"]) >= start_frame]
    if not rows:
        raise ValueError("no frames in selection")
    result = {"frames": len(rows), "ticks": sum(int(row["tick_count"]) for row in rows), "budget_ms": budget, "metrics": {}}
    for name in ("interval_ns", "work_ns", "tick_ns", "present_ns"):
        values = sorted(int(row[name]) / 1e6 for row in rows if int(row[name]) > 0)
        if not values:
            continue
        result["metrics"][name] = {"samples": len(values), "median_ms": percentile(values, .5), "p95_ms": percentile(values, .95),
                                   "p99_ms": percentile(values, .99), "max_ms": values[-1], "over_budget": sum(value > budget for value in values)}
    if result["ticks"]:
        result["mean_tick_ms"] = sum(int(row["tick_ns"]) for row in rows) / result["ticks"] / 1e6
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path)
    parser.add_argument("--budget-ms", type=float, default=1000 / 60)
    parser.add_argument("--from-frame", type=int, default=0, help="skip startup/loading frames")
    args = parser.parse_args()
    if not math.isfinite(args.budget_ms) or args.budget_ms <= 0:
        parser.error("budget must be positive and finite")
    try:
        result = summarise(args.csv, args.budget_ms, args.from_frame)
    except (OSError, ValueError, KeyError) as error:
        print(f"profile failed: {error}", file=sys.stderr)
        return 1
    print(f"{result['frames']} frames, {result['ticks']} simulation steps; budget {args.budget_ms:.3f} ms")
    print("CPU metric       median      p95      p99      max   over budget")
    for name, values in result["metrics"].items():
        print(f"{name[:-3]:15} {values['median_ms']:7.3f} {values['p95_ms']:8.3f} {values['p99_ms']:8.3f} {values['max_ms']:8.3f} {values['over_budget']:7}")
    if "mean_tick_ms" in result:
        print(f"Mean CPU time per simulation step: {result['mean_tick_ms']:.3f} ms")
    print("present includes CPU executor work/waits; GPU time and input latency are not measured.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
