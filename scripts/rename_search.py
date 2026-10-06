#!/usr/bin/env python3
"""Try renaming a function's local variables to fix stack-slot ties.

  rename_search.py 0xADDR [--limit N] [--apply]

MSVC6 orders stack slots by reference count, but variables used equally often
are ordered by something that depends on their names (declaration order does
not matter); the same goes for which dead parameter slot an inline-asm
argument is homed in. When a diff only swaps two equally used slots, renaming the
locals can fix it. The candidates are swaps of two locals (or parameters)
declared with the same type, and then renames of single locals to name+"2", name+"_", ...;
both keep the program the same. The best variant is printed (and written
with --apply). Struct members (after "." or "->") are never touched.
"""
import argparse
import itertools
import os
from pathlib import Path
import re
import signal
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


def params_of(before):
    """(type, name) of the parameters in the signature that ends the text
    before the function's body."""
    inner = before[before.rindex("(", 0, before.rindex(")")) + 1:before.rindex(")")]
    out = []
    for part in inner.split(","):
        m = re.match(r"\s*(.*?)\s*\b([A-Za-z_]\w*)\s*$", part.replace("*", " * "))
        if m and m.group(1) and m.group(1).strip() not in ("", "void"):
            out.append((re.sub(r"\s+", "", m.group(1)), m.group(2)))
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
    if "compile failed" in out:
        return 0.0
    raise RuntimeError(out)


def candidates_for(text, b, names):
    taken = set(re.findall(r"\b[A-Za-z_]\w*\b", text))
    out = []
    by_type = {}
    for typ, name in names:
        by_type.setdefault(typ, []).append(name)
    for typ, group in by_type.items():
        for x, y in itertools.combinations(group, 2):
            out.append({x: y, y: x})
    for _, name in names:
        for suffix in ("2", "_", "0", "1", "Value", "Tmp"):
            if name + suffix not in taken:
                out.append({name: name + suffix})
    return out


def merges_for(names):
    """Merging x into y (same type): the same program only if their lifetimes
    do not overlap, so these are reported for review, never taken blindly."""
    out = []
    by_type = {}
    for typ, name in names:
        by_type.setdefault(typ, []).append(name)
    for typ, group in by_type.items():
        for x, y in itertools.permutations(group, 2):
            out.append({x: y, "__merge__": x})
    return out


def merged(body, mapping):
    x = mapping["__merge__"]
    decl = re.compile(r"^[ \t]*[\w\s*]+?\b" + re.escape(x) + r"\s*;[ \t]*\n", re.M)
    if not decl.search(body):
        return None
    body = decl.sub("", body, count=1)
    return rename(body, {k: v for k, v in mapping.items() if k != "__merge__"})


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("address")
    ap.add_argument("--limit", type=int, default=400, help="candidates per round")
    ap.add_argument("--rounds", type=int, default=3, help="keep the best rename and search again")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--merge", action="store_true",
                    help="also try merging two same-type locals (check lifetimes before keeping one)")
    args = ap.parse_args()
    addr = int(args.address, 16)
    # a kill must still restore the source (the finally below)
    signal.signal(signal.SIGTERM, lambda *_: sys.exit(1))
    path = Path(F.src_for(addr))
    original = text = path.read_text(encoding="latin1")
    base = best = score(addr, path)
    applied = []
    try:
        for rnd in range(args.rounds):
            b, e = func_region(text, addr)
            names = params_of(text[:b]) + locals_of(text[b:e])
            # rename from the signature's line on, so parameters change everywhere
            b = text.rfind("\n", 0, text.rindex("(", 0, text.rindex(")", 0, b))) + 1
            body = text[b:e]
            cands = candidates_for(text, b, names)
            if args.merge:
                cands += merges_for(locals_of(text[text.index("{", b):e]))
            if rnd == 0:
                print(f"{len(names)} locals and parameters, {len(cands)} candidates, baseline {100 * base:.2f}%")
            round_best, round_text, round_map = best, None, None
            for mapping in cands[:args.limit]:
                new_body = merged(body, mapping) if "__merge__" in mapping else rename(body, mapping)
                if new_body is None:
                    continue
                trial = text[:b] + new_body + text[e:]
                path.write_text(trial, encoding="latin1")
                s = score(addr, path)
                if s > round_best + 1e-9:
                    round_best, round_text, round_map = s, trial, mapping
                    print(f"  round {rnd + 1}: {100 * s:.2f}%  {mapping}"
                          + ("  <- merge: check the lifetimes" if "__merge__" in mapping else ""))
                    if s >= 1.0:
                        break
            if round_text is None:
                break
            best, text = round_best, round_text
            applied.append(round_map)
            if best >= 1.0:
                break
    finally:
        path.write_text(original, encoding="latin1")
    if applied and args.apply:
        path.write_text(text, encoding="latin1")
        print(f"applied {applied}: {100 * base:.2f}% -> {100 * best:.2f}%")
    elif not applied:
        print("no rename helps")
    else:
        print(f"best {applied}: {100 * base:.2f}% -> {100 * best:.2f}% (use --apply)")


if __name__ == "__main__":
    main()
