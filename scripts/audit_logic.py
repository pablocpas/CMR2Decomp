#!/usr/bin/env python3
"""Inventory local code equivalence and behavioral evidence for every function.

Static signals prioritize inspection; they do not prove a logic defect.
Passing isolated harnesses cover their fixtures and mocked boundaries only.
"""
import argparse
import collections
import csv
import hashlib
import json
from pathlib import Path
import re
import tempfile

import fastcmp as F
from permute_batch import func_region

ROOT = Path(__file__).resolve().parents[1]
CRITICAL = {
    "Car.cpp",
    "CarPhysics.cpp",
    "VehiclePhysics.cpp",
    "TrackCollision.cpp",
    "Collision2D.cpp",
    "FixedPoint.cpp",
    "SurfaceTypes.cpp",
    "RallyData.cpp",
    "NetRace.cpp",
    "NetPlayers.cpp",
    "StageTiming.cpp",
    "Race.cpp",
    "Input.cpp",
}
AREAS = {
    "CarPhysics.cpp": "coche/física",
    "VehiclePhysics.cpp": "coche/física",
    "Car.cpp": "coche/cámara",
    "Collision2D.cpp": "colisión",
    "TrackCollision.cpp": "colisión/pista",
    "SurfaceTypes.cpp": "superficies",
    "FixedPoint.cpp": "matemática",
    "RallyData.cpp": "rutas/carrera",
    "StageTiming.cpp": "tiempos/replay",
    "Race.cpp": "carrera",
    "NetRace.cpp": "red",
    "NetPlayers.cpp": "red",
    "Input.cpp": "entrada",
    "StageObjects.cpp": "objetos/IA/replay",
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def instructions(rows):
    # Reachability excludes neighbouring functions and decoded jump-table data.
    indexed = {row[0]: row for row in rows}
    work = [rows[0][0]] if rows else []
    visited = set()
    indirect = False
    while work:
        address = work.pop()
        if address in visited or address not in indexed:
            continue
        visited.add(address)
        _, size, text = indexed[address]
        mnemonic = text.split(" ", 1)[0]
        if mnemonic in ("ret", "retf", "int3", "hlt"):
            continue
        if mnemonic.startswith("j"):
            target = re.fullmatch(r"j\w* (0x[\da-f]+)", text)
            if target:
                work.append(int(target[1], 16))
            else:
                indirect = True
            if mnemonic == "jmp":
                continue
        work.append(address + size)
    calls = collections.Counter()
    returns = set()
    branches = collections.Counter()
    for address, _, text in rows:
        if address not in visited:
            continue
        if text.startswith("call "):
            calls[text[5:]] += 1
        elif text.startswith("ret"):
            returns.add(text)
        elif text.split(" ", 1)[0].startswith("j"):
            branches[text.split(" ", 1)[0]] += 1
    return {
        "calls": dict(calls),
        "returns": sorted(returns),
        "branches": dict(branches),
        "indirect_dispatch_not_fully_traversed": indirect,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "CMR2PROGRESS")
    parser.add_argument(
        "--disassembly",
        type=Path,
        help="Optional JSON containing original/rebuilt disassembly of pending functions.",
    )
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((ROOT / "build/manifest.json").read_text())
    provenance = json.loads((ROOT / "CMR2PROGRESS/provenance.json").read_text())
    if provenance["build"] != manifest:
        parser.error("Reports and build disagree; rebuild and measure.")
    original_sha = sha(ROOT / "cmr2bin/CMR2.exe")
    if provenance["original_sha256"] != original_sha:
        parser.error("Original executable changed since measurement.")
    for suffix in ("exe", "pdb"):
        if sha(ROOT / ("build/CMR2." + suffix)) != manifest[suffix + "_sha256"]:
            parser.error("Build artifacts changed since measurement.")
    for path, expected in manifest["source_sha256"].items():
        if sha(ROOT / path) != expected:
            parser.error("Sources changed since measurement: " + path)
    audit = json.loads((ROOT / "CMR2PROGRESS/bytes.json").read_text())
    report = json.loads((ROOT / "CMR2PROGRESS/summary.json").read_text())
    entities = json.loads((ROOT / "CMR2PROGRESS/entities.json").read_text())
    suite = json.loads((ROOT / "tests/logic-targets.json").read_text())
    tests = json.loads((ROOT / "CMR2PROGRESS/logic-tests.json").read_text())
    valid_tests = (
        tests["exe_sha256"] == manifest["exe_sha256"]
        and tests["original_sha256"] == original_sha
        and tests["pdb_sha256"] == manifest["pdb_sha256"]
        and tests["report_sha256"] == sha(ROOT / "CMR2PROGRESS/summary.json")
        and tests["entities_sha256"] == sha(ROOT / "CMR2PROGRESS/entities.json")
        and tests["suite_sha256"] == sha(ROOT / "tests/logic-targets.json")
        and {
            name: value
            for name, value in tests["test_source_sha256"].items()
            if name.startswith("differential_") or name == "matching_entities.py"
        }
        == {
            p.name: sha(p)
            for p in sorted((ROOT / "tests").glob("*.py"))
            if (p.name.startswith("differential_") or p.name == "matching_entities.py")
            and p.name in tests["test_source_sha256"]
        }
    )
    if not valid_tests:
        parser.error("Behavior evidence is stale; run tests/run_differential_suite.py.")
    passed = collections.defaultdict(list)
    failed = collections.defaultdict(list)
    results = {r["test"]: r for r in tests["results"]}
    for name, test in suite.items():
        result = results.get(name)
        for address in test["targets"]:
            if result and result["exit_code"] == 0:
                passed[address].append(name)
            elif result:
                failed[address].append(name)
    inventory = list(csv.DictReader(
        (ROOT / "scripts/functions.tsv").open(), delimiter="\t"
    ))
    sizes = {
        int(row["addr"], 16): int(row["size"])
        for row in inventory
    }
    scores = {row["address"]: row.get("matching", 0) for row in report["data"]}
    starts = sorted(int(a, 16) for a in audit)
    following = dict(zip(starts, starts[1:]))
    sources = {
        p.name: p.read_text(encoding="latin1")
        for p in (ROOT / "CMR2Decomp").glob("*.cpp")
    }
    F.REPO = str(ROOT)
    original_find = F.find_func

    def find(coff, name):
        found = original_find(coff, name)
        if found[1] is not None:
            return found
        parts = name.split("::")
        if len(parts) >= 2 and parts[-1] == parts[-2]:
            prefix = "??0" + "@".join(reversed(parts[:-1])) + "@@"
            candidates = [
                (i, symbol)
                for i, symbol in enumerate(coff.syms)
                if symbol
                and symbol["sec"] > 0
                and symbol["typ"] & 0x20
                and symbol["name"].startswith(prefix)
                and coff.secs[symbol["sec"] - 1]["name"] == ".text"
            ]
            if len(candidates) == 1:
                return candidates[0]
        return found

    F.find_func = find
    evidence = {}
    rows = []
    notespath = ROOT / "CMR2PROGRESS/logic-notes.json"
    notes = json.loads(notespath.read_text()) if notespath.exists() else {}
    with tempfile.TemporaryDirectory(prefix="cmr2-logic-meta-") as temporary:
        F.HERE = temporary
        F._NM = None
        for entry in report["data"]:
            if entry["address"] in audit:
                entry["name"] = audit[entry["address"]]["symbol_name"]
        Path(temporary, "cur.json").write_text(json.dumps(report))
        Path(temporary, "ent.json").write_text(
            json.dumps(
                [{"o": int(a, 16), "r": v[0], "n": v[1]} for a, v in entities.items()]
            )
        )
        for address, result in audit.items():
            addr = int(address, 16)
            source = sources[result["f"]]
            extent_error = None
            try:
                begin, end = func_region(source, addr)
            except ValueError as error:
                extent_error = str(error)
                markers = list(
                    re.finditer(r"//\s*FUNCTION:\s*CMR2\s+(0x[\da-fA-F]+)", source)
                )
                marker = next(m for m in markers if int(m[1], 16) == addr)
                begin = source.index("\n", marker.end()) + 1
                end = next(
                    (m.start() for m in markers if m.start() > begin), len(source)
                )
            body = source[begin:end]
            line = source.count("\n", 0, begin) + 1
            context = source[source.rfind("\n}", 0, begin) + 2 : begin]
            description = " ".join(
                l.strip()[2:].strip()
                for l in context.splitlines()
                if l.strip().startswith("//")
                and not re.search(r"FUNCTION:|GLOBAL:|match\s+\d|CONVENCIONES", l)
            )[-500:]
            cleaned = re.sub(r"//[^\n]*|/\*[\s\S]*?\*/", "", body)
            signals = []
            if extent_error:
                signals.append("source_extent_requires_preprocessor")
            if re.search(r"\bTODO\b|\bFIXME\b|UNFINISHED|\bUNIMPLEMENTED\b", body):
                signals.append("source_marker")
            if not result["x"] and re.fullmatch(
                r"\{\s*(?:return\s+(?:0|false|NULL)?\s*;)?\s*\}", cleaned
            ):
                signals.append("trivial_body_requires_review")
            comparison = None
            if not result["x"]:
                score, exact, (original, rebuilt, _), unknown = F.compare(
                    addr,
                    str(ROOT / "build" / (Path(result["f"]).stem + ".obj")),
                    str(ROOT / "CMR2Decomp" / result["f"]),
                    result["symbol_name"],
                    sizes.get(addr),
                )
                if (
                    abs(score - result["s"]) > 1e-9
                    or (exact and not unknown) != result["x"]
                ):
                    raise RuntimeError("Byte audit changed: " + address)
                original = [
                    row for row in original if row[0] < following.get(addr, 0xFFFFFFFF)
                ]
                left, right = instructions(original), instructions(rebuilt)
                comparison = {"original": left, "rebuilt": right}
                partial = (
                    left["indirect_dispatch_not_fully_traversed"]
                    or right["indirect_dispatch_not_fully_traversed"]
                )
                if partial:
                    signals.append("indirect_dispatch_requires_case_tests")
                if not partial and left["calls"] != right["calls"]:
                    signals.append("calls_differ_may_be_inlining")
                if (
                    left["returns"]
                    and right["returns"]
                    and left["returns"] != right["returns"]
                ):
                    signals.append("return_cleanup_diff_requires_manual_check")
                if not partial and left["branches"] != right["branches"]:
                    signals.append("branches_differ_may_be_compiler")
                if args.disassembly:
                    evidence[address] = {
                        "file": result["f"],
                        "name": result["n"],
                        "original": original,
                        "rebuilt": rebuilt,
                    }
            if failed[address]:
                status = "test_failed"
            elif result["x"]:
                status = "local_byte_exact"
            elif passed[address]:
                status = "fixtures_pass_matching_pending"
            elif result.get("unknown"):
                status = "unresolved_and_untested"
            else:
                status = "logic_unverified"
            priority = 0
            if not result["x"] and not passed[address]:
                priority = (40 if result["f"] in CRITICAL else 15) + int(
                    30 * (1 - result["s"])
                )
                priority += 15 * bool(result.get("unknown"))
                priority += 10 * ("calls_differ_may_be_inlining" in signals)
                priority += 30 * (
                    "return_cleanup_diff_requires_manual_check" in signals
                )
                priority += 40 * (
                    "trivial_body_requires_review" in signals
                    or "source_marker" in signals
                )
            if failed[address]:
                priority += 100
            rows.append(
                {
                    "address": address,
                    "name": result["n"],
                    "file": result["f"],
                    "line": line,
                    "area": AREAS.get(result["f"], result["f"].removesuffix(".cpp")),
                    "description": description,
                    "original_bytes": sizes.get(addr, 0),
                    "reccmp_score": scores.get(address, 0),
                    "byte_score": result["s"],
                    "status": status,
                    "priority": priority,
                    "unknown": result.get("unknown", []),
                    "signals": signals,
                    "tests_passed": passed[address],
                    "tests_failed": failed[address],
                    "machine_features": comparison,
                    "review": notes.get(address),
                }
            )
    rows.sort(key=lambda r: (-r["priority"], r["reccmp_score"], -r["original_bytes"]))
    counts = dict(collections.Counter(row["status"] for row in rows))
    pending = [row for row in rows if row["status"] != "local_byte_exact"]
    # Inventory entries without annotated source are not automatically missing
    # game logic: they include linked libraries, import stubs and inner entries.
    measured = {entry["address"]: entry for entry in report["data"]}
    original_pe = F.PE(str(ROOT / "cmr2bin/CMR2.exe"))
    outside = []
    for item in inventory:
        address = hex(int(item["addr"], 16))
        if address in audit:
            continue
        code = F.dis(original_pe.read(int(address, 16), 16), int(address, 16))
        first = code[0][2] if code else ""
        entity = entities.get(address)
        measurement = measured.get(address)
        category = "unclassified_without_annotated_source"
        if first.startswith("jmp "):
            category = "entry_jump_requires_target_review"
        if measurement and measurement.get("library"):
            category = "measured_linked_library"
        if address == "0x41f788":
            # Conditional branches in 0x41f560 enter this stage-path block.
            category = "internal_entry_of_0x41f560"
        outside.append({
            "address": address,
            "inventory_name": item["name"],
            "original_bytes": int(item["size"]),
            "inventory_thunk": item["thunk"],
            "matched_name": entity[1] if entity else "",
            "category": category,
            "reccmp_score": measurement.get("matching", "") if measurement else "",
            "first_instruction": first,
        })
    output = {
        "build": manifest,
        "original_sha256": original_sha,
        "inventory_sha256": sha(ROOT / "scripts/functions.tsv"),
        "test_evidence_sha256": sha(ROOT / "CMR2PROGRESS/logic-tests.json"),
        "audit_source_sha256": sha(Path(__file__)),
        "counts": counts,
        "outside_source_counts": dict(collections.Counter(r["category"] for r in outside)),
        "limits": [
            "Byte equivalence is local: callees may still be pending.",
            "Harnesses cover listed entry points, valid fixtures and controlled providers.",
            "Static call/branch/return differences require manual inspection; they are not defect proofs.",
            "No complete interactive race or network session was validated.",
        ],
        "functions": rows,
        "outside_annotated_source": outside,
    }
    (out / "logic-audit.json").write_text(
        json.dumps(output, indent=2, ensure_ascii=False) + "\n"
    )
    columns = [
        "address",
        "name",
        "file",
        "line",
        "area",
        "status",
        "priority",
        "reccmp_score",
        "byte_score",
        "original_bytes",
        "tests_passed",
        "unknown",
        "signals",
    ]
    for filename, entries in [("logic-all.tsv", rows), ("logic-pending.tsv", pending)]:
        with (out / filename).open("w", newline="") as handle:
            writer = csv.DictWriter(
                handle, fieldnames=columns, delimiter="\t", lineterminator="\n"
            )
            writer.writeheader()
            for row in entries:
                writer.writerow(
                    {
                        key: (
                            ";".join(row[key])
                            if isinstance(row[key], list)
                            else row[key]
                        )
                        for key in columns
                    }
                )
    with (out / "logic-outside-source.tsv").open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(outside[0]), delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(outside)
    if args.disassembly:
        args.disassembly.parent.mkdir(parents=True, exist_ok=True)
        args.disassembly.write_text(json.dumps(evidence))
    print(
        "Functions", len(rows), "statuses", counts, "remaining matching", len(pending)
    )
    print(
        "Static signals",
        dict(collections.Counter(s for r in pending for s in r["signals"])),
    )


if __name__ == "__main__":
    main()
