#!/usr/bin/env python3
"""List what in game/ breaks in a 64-bit build, by original function.

Checks every game/*.cpp as x86-64 (syntax only, the build's flags plus the
pointer-truncation warnings) and groups the findings by the enclosing
`// FUNCTION: CMR2 0x...` annotation, which CMR2Decomp shares:

- casts: pointer <-> integer conversions (truncate addresses on 64-bit);
- layouts: 32-bit layout assertions that fail because the type holds
  pointers (fine if nothing reads it through raw offsets or from disk).

    tools/audit_64bit.py [--json out.json] [--jobs N]

Raw offset accesses compile silently on 64-bit; CMR2Decomp's
scripts/audit_readability.py lists those.
"""
import argparse
import collections
import concurrent.futures
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FLAGS = ["-m64", "-fsyntax-only", "-std=gnu++17", "-fwrapv", "-fno-strict-aliasing", "-fms-extensions",
         f"-I{ROOT / 'game'}", f"-I{ROOT / 'src'}", "-include", str(ROOT / "src/port/types.h"),
         "-include", str(ROOT / "src/port/msvcrt.h"), "-Wno-everything", "-Wpointer-to-int-cast",
         "-Wint-to-pointer-cast", "-Wvoid-pointer-to-int-cast", "-Wint-to-void-pointer-cast",
         "-Wshorten-64-to-32", "-ferror-limit=0", "-fno-color-diagnostics"]
DIAG = re.compile(r"^(?P<file>[^:]+):(?P<line>\d+):\d+: (?P<kind>warning|error): (?P<msg>.*?)(?: \[(?P<flag>-W[\w-]+)\])?$")
ANNOTATION = re.compile(r"^// (?:FUNCTION|LIBRARY): CMR2 (0x[0-9a-fA-F]+)")
LAYOUT = re.compile(r"'(\w+)' declared as an array with a negative size")


def check(path):
    result = subprocess.run(["clang++", *FLAGS, str(path)], capture_output=True, text=True)
    return result.stderr.splitlines()


def enclosing_functions(path):
    """Line number -> (address, name) of the annotated function the line is in."""
    lines = path.read_text(errors="replace").splitlines()
    owner, current = {}, None
    for i, line in enumerate(lines, 1):
        m = ANNOTATION.match(line)
        if m:
            name = next((l for l in lines[i:i + 3] if l and not l.startswith("//")), "")
            name = re.search(r"([\w:~]+)\s*\(", name)
            current = (m.group(1).lower(), name.group(1) if name else "?")
        owner[i] = current
    return owner


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--json", type=Path)
    ap.add_argument("--jobs", type=int, default=os.cpu_count())
    args = ap.parse_args()

    sources = sorted((ROOT / "game").glob("*.cpp"))
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        output = [line for lines in pool.map(check, sources) for line in lines]

    owners = {}
    functions = collections.defaultdict(lambda: collections.Counter())
    names, files = {}, {}
    layouts = collections.Counter()
    seen = set()
    for line in output:
        m = DIAG.match(line)
        if not m or (m["file"], m["line"], m["msg"]) in seen:
            continue
        seen.add((m["file"], m["line"], m["msg"]))
        if m["kind"] == "error":
            layout = LAYOUT.search(m["msg"])
            if layout:
                layouts[layout.group(1)] += 1
            continue
        path = Path(m["file"]).resolve()
        if path.suffix != ".cpp":
            functions[("header", path.name)][m["flag"]] += 1
            continue
        if path not in owners:
            owners[path] = enclosing_functions(path)
        owner = owners[path].get(int(m["line"]))
        key = owner[0] if owner else f"{path.name}:global"
        names[key] = owner[1] if owner else "(file scope)"
        files[key] = path.name
        functions[key][m["flag"]] += 1

    rows = sorted(functions.items(), key=lambda kv: -sum(kv[1].values()))
    total = sum(sum(c.values()) for c in functions.values())
    print(f"{total} pointer/integer casts in {len(functions)} functions; "
          f"{len(layouts)} layout assertions assume 32-bit pointers")
    by_file = collections.Counter()
    for key, counts in rows:
        by_file[files.get(key, key[1] if isinstance(key, tuple) else key)] += sum(counts.values())
    for name, count in by_file.most_common(12):
        print(f"  {name:28} {count:5}")
    if args.json:
        args.json.write_text(json.dumps({
            "casts": [{"function": key if isinstance(key, str) else None, "name": names.get(key, ""),
                       "file": files.get(key, key[1] if isinstance(key, tuple) else ""),
                       "counts": dict(counts)} for key, counts in rows],
            "layout_assertions": sorted(layouts),
        }, indent=1) + "\n")
        print(f"wrote {args.json}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
