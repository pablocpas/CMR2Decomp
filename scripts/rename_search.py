#!/usr/bin/env python3
"""Try renaming a function's local variables to fix stack-slot ties.

  rename_search.py 0xADDR [--limit N] [--apply]

MSVC6 orders stack slots by reference count, but variables used equally often
are ordered by something that depends on their names (declaration order does
not matter). When a diff only swaps two equally used slots, renaming the
locals can fix it. The candidates are swaps of two locals declared with the
same type, and then renames of single locals to name+"2", name+"_", ...;
both keep the program the same. The best variant is printed (and written
with --apply). Struct members (after "." or "->") are never touched.
"""
import argparse
import itertools
import os
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fastcmp as F  # noqa: E402
from permute_batch import func_region  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
DECL = re.compile(r"^\s*((?:unsigned |signed |const )?[A-Za-z_]\w*(?:\s*\*+)?)\s+(\*?\s*[A-Za-z_]\w*(?:\s*\[[^\]]*\])?"
                  r"(?:\s*=\s*[^,;]+)?(?:\s*,\s*\*?\s*[A-Za-z_]\w*(?:\s*\[[^\]]*\])?(?:\s*=\s*[^,;]+)?)*)\s*;", re.M)
KEYWORDS = {"return", "goto", "else", "case", "delete", "new", "typedef", "struct"}


def locals_of(body):
    """(type, name) of the variables declared in the function body."""
    out = []
    for m in DECL.finditer(body):
        typ = m.group(1).strip()
        if typ in KEYWORDS:
            continue
        for part in m.group(2).split(","):
            part = part.split("=")[0].strip()
            stars = part.count("*")
            name = re.sub(r"[\s*]|\[.*", "", part)
            if name and re.fullmatch(r"[A-Za-z_]\w*", name):
                out.append((typ + "*" * stars, name))
    return out


def rename(body, mapping):
    pat = re.compile(r"(?<![\w.>])(" + "|".join(map(re.escape, mapping)) + r")\b")
    return pat.sub(lambda m: mapping[m.group(1)], body)


def score(addr, src):
    out = subprocess.run([sys.executable, str(ROOT / "scripts/match.py"), hex(addr), "--no-diff", "--all"],
                         capture_output=True, text=True).stdout
    for line in out.splitlines():
        if f"0x{addr:x} " in line or f"{hex(addr)} " in line:
            if "-> exact" in line or line.strip().startswith("EXACT") or "exact" == line.split()[-1]:
                return 1.0
            nums = re.findall(r"([\d.]+)%", line)
            if nums:
                return float(nums[-1]) / 100
    raise RuntimeError(out)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("address")
    ap.add_argument("--limit", type=int, default=200)
    ap.add_argument("--apply", action="store_true")
    args = ap.parse_args()
    addr = int(args.address, 16)
    path = Path(F.src_for(addr))
    text = path.read_text(encoding="latin1")
    b, e = func_region(text, addr)
    body = text[b:e]
    names = locals_of(body)
    taken = set(re.findall(r"\b[A-Za-z_]\w*\b", text))
    candidates = []
    by_type = {}
    for typ, name in names:
        by_type.setdefault(typ, []).append(name)
    for typ, group in by_type.items():
        for x, y in itertools.combinations(group, 2):
            candidates.append({x: y, y: x})
    for _, name in names:
        for suffix in ("2", "_", "0", "1", "Value", "Tmp"):
            new = name + suffix
            if new not in taken:
                candidates.append({name: new})
    base = score(addr, path)
    print(f"{len(names)} locals, {len(candidates)} candidates, baseline {100 * base:.2f}%")
    best, best_map = base, None
    try:
        for mapping in candidates[:args.limit]:
            path.write_text(text[:b] + rename(body, mapping) + text[e:], encoding="latin1")
            s = score(addr, path)
            if s > best + 1e-9:
                best, best_map = s, mapping
                print(f"  {100 * s:.2f}%  {mapping}")
                if s >= 1.0:
                    break
    finally:
        path.write_text(text, encoding="latin1")
    if best_map and args.apply:
        path.write_text(text[:b] + rename(body, best_map) + text[e:], encoding="latin1")
        print(f"applied {best_map}")
    elif not best_map:
        print("no rename helps")


if __name__ == "__main__":
    main()
