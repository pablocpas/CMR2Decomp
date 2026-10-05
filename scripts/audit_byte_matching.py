#!/usr/bin/env python3
"""Audit annotated functions against original bytes, using a frozen build.

Usage: report.json entities.json build-directory output.json

Each COFF relocation is resolved from that build's PE and entity map. Source
identifiers disambiguate generic reccmp CRT names; constructors use their real
MSVC symbol. This script uses private metadata and leaves shared reports and
fastcmp code untouched. Supply the matching report/entities/build together.
"""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import sys
import tempfile

ROOT = Path(__file__).resolve().parent.parent
TOOLS = ROOT / "scripts"


def source_identifier(source, marker):
    start = source.index("\n", marker.end())+1
    end = source.index("{", start)
    match = re.search(r"([A-Za-z_][\w:~]*)\s*\(", source[start:end])
    if not match:
        raise ValueError("Cannot identify annotated source function")
    return match.group(1)


def main():
    report_path, entity_path, build_path, output_path = map(Path, sys.argv[1:5])
    build_path = build_path.resolve()
    report = json.loads(report_path.read_text())
    entities = json.loads(entity_path.read_text())
    rows = [{"o": int(a, 16), "r": v[0], "n": v[1]} for a, v in entities.items()]
    os.environ["CMR2_REPO"] = str(ROOT)
    module_path = Path(os.environ.get("CMR2_FASTCMP_PATH", TOOLS / "fastcmp.py"))
    spec = importlib.util.spec_from_file_location("cmr2_snapshot_fastcmp", module_path)
    f = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(f)
    original_find = f.find_func

    def find(c, name):
        result = original_find(c, name)
        if result[1] is not None:
            return result
        parts = name.split("::")
        if len(parts) >= 2 and parts[-1] == parts[-2]:
            prefix = "??0"+"@".join(reversed(parts[:-1]))+"@@"
            candidates = [(i, s) for i, s in enumerate(c.syms)
                          if s and s["sec"] > 0 and s["typ"] & 0x20
                          and s["name"].startswith(prefix)
                          and c.secs[s["sec"]-1]["name"] == ".text"]
            if len(candidates) == 1:
                return candidates[0]
        return result

    f.find_func = find
    linked = f.PE(str(build_path / "CMR2.exe"))
    r2o = {v[0]: int(a, 16) for a, v in entities.items()}
    symbol_cache = {}

    def symbols(source):
        stem = Path(source).stem
        if stem not in symbol_cache:
            symbol_cache[stem] = f.learn_symbols(str(build_path / (stem+".obj")), linked, r2o)
        return symbol_cache[stem]

    f.symmap_for = symbols
    results = {}
    with tempfile.TemporaryDirectory(prefix="cmr2-byte-audit-") as tmp:
        f.HERE = tmp
        f._NM = None
        Path(tmp, "cur.json").write_text(json.dumps(report))
        Path(tmp, "ent.json").write_text(json.dumps(rows))
        names, sizes = f.load_meta()
        for path in sorted((ROOT / "CMR2Decomp").glob("*.cpp")):
            source = path.read_text()
            obj = str(build_path / (path.stem+".obj"))
            for marker in re.finditer(r"// FUNCTION: CMR2 (0x[0-9a-fA-F]+)", source):
                address = int(marker.group(1), 16)
                if address not in names:
                    continue
                name = names[address]
                symbol_name = name
                try:
                    if find(f.COFF(obj), name)[1] is None:
                        symbol_name = source_identifier(source, marker)
                    score, exact, _, unknown = f.compare(
                        address, obj, str(path), symbol_name, sizes.get(address))
                    results[hex(address)] = {
                        "n": name, "s": score, "x": exact and not unknown,
                        "unknown": sorted(unknown), "f": path.name,
                        "symbol_name": symbol_name,
                    }
                except Exception as error:
                    results[hex(address)] = {
                        "n": name, "s": 0, "x": False, "err": str(error),
                        "f": path.name, "symbol_name": symbol_name,
                    }
    output_path.write_text(json.dumps(results, indent=2)+"\n")
    print("fastcmp_sha256", hashlib.sha256(module_path.read_bytes()).hexdigest())
    print("functions", len(results), "byte_exact", sum(v["x"] for v in results.values()))
    print("errors", [(a, v["err"]) for a, v in results.items() if "err" in v])


if __name__ == "__main__":
    main()
