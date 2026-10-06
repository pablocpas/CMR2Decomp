#!/usr/bin/env python3
"""Find non-exact functions that use a FixedPoint.h inline-asm helper a different
number of times than the original.

  helper_hints.py [--file X.cpp] [0xADDR ...]

Each inline helper (FixMul, FixDiv, FixVecLength, FixVecScaleRecip, ...) leaves
a fixed instruction sequence, since its registers are written in the asm. The
helpers are compiled once in a scratch TU to get those sequences; then every
remaining function of the last full build is scanned in the original and in
our object. A helper the original uses more often than our source usually
means the source spells the same maths another way (FixSqrt of a FixMul sum
instead of FixVecLength, a C expression instead of FixMulShift32, ...), which
changes register allocation even when the results agree.
"""
import argparse
import collections
import json
import os
from pathlib import Path
import re
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fastcmp as F  # noqa: E402
import match as M  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]

WRAPPERS = """
#include <windows.h>
#include "FixedPoint.h"
int w_FixMul(int a, int b) { return FixMul(a, b); }
int w_FixMulShift32(int a, int b) { return FixMulShift32(a, b); }
int w_FixDiv(int a, int b) { return FixDiv(a, b); }
int w_FixVecLength(FixVector *v) { return FixVecLength(v); }
int w_FixSqrt(int v) { return FixSqrt(v); }
void w_FixVecScale(FixVector *o, FixVector *s, int t) { FixVecScale(o, s, t); }
int w_FixVecDot(FixVector *a, FixVector *b) { return FixVecDot(a, b); }
void w_FixVecCross(FixVector *o, FixVector *a, FixVector *b) { FixVecCross(o, a, b); }
short w_FixAtan2(int y, int x) { return FixAtan2(y, x); }
void w_FixVecScaleRecip(FixVector *o, FixVector *s, int l) { FixVecScaleRecip(o, s, l); }
"""
JUMP = re.compile(r"^(j\w+|loop\w*) 0x[0-9a-f]+$")


def norm(text):
    return JUMP.sub(lambda m: m.group(1) + " L", text)


def body(instrs):
    """Instruction texts without frame/parameter traffic, prologue or epilogue."""
    out = []
    for _, _, t in instrs:
        if re.search(r"\b[er]?[bs]p\b", t) or t.startswith(("ret", "push", "pop", "leave")):
            continue
        out.append(norm(t))
    return out


def fingerprints():
    with tempfile.TemporaryDirectory() as tmp:
        src = Path(tmp) / "Helpers.cpp"
        src.write_text(WRAPPERS)
        obj = str(src.with_suffix(".obj"))
        F.compile_tu(str(src), obj)
        c = F.COFF(obj)
        prints = {}
        for name in re.findall(r"\bw_(\w+)\(", WRAPPERS):
            i, sym = F.find_func(c, "w_" + name)
            st, en = F.func_extent(c, sym)
            data = c.secs[sym["sec"] - 1]["data"][st:en]
            seq = body(F.dis(F.strip_pad(bytes(data)), 0))
            prints[name] = seq
    # FixMul's two-instruction core also appears inside the longer helpers;
    # count it only where it stands alone.
    return prints


def count(seq, pat):
    n, k = 0, 0
    while k + len(pat) <= len(seq):
        if seq[k:k + len(pat)] == pat:
            n += 1
            k += len(pat)
        else:
            k += 1
    return n


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("addresses", nargs="*")
    ap.add_argument("--file")
    args = ap.parse_args()
    prints = fingerprints()
    longest_first = sorted(prints, key=lambda n: -len(prints[n]))
    base = json.loads((ROOT / "CMR2PROGRESS/bytes.json").read_text())
    _, sizes = F.load_meta()
    wanted = {int(a, 16) for a in args.addresses}
    coffs = {}
    for a, v in sorted(base.items()):
        addr = int(a, 16)
        if v["x"] or v.get("unknown") or (wanted and addr not in wanted):
            continue
        if args.file and v["f"] not in (args.file, args.file + ".cpp"):
            continue
        obj = str(ROOT / "build" / (v["f"][:-4] + ".obj"))
        coff = coffs.setdefault(obj, F.COFF(obj))
        try:
            _, _, (oi, ri, _), _ = M.compare(addr, obj, str(ROOT / "CMR2Decomp" / v["f"]),
                                             v["symbol_name"], sizes.get(addr), coff)
        except Exception:
            continue
        so, sr = body(oi), body(ri)
        diffs = []
        for name in longest_first:
            pat = prints[name]
            co, cr = count(so, pat), count(sr, pat)
            if co != cr:
                diffs.append(f"{name} orig {co} / ours {cr}")
        if diffs:
            print(f"{a} {v['n']} ({v['f']}, {100 * v['s']:.1f}%): " + "; ".join(diffs))


if __name__ == "__main__":
    main()
