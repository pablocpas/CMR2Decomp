#!/usr/bin/env python3
"""One-command matching loop: compile, score every function, show the diff.

  match.py 0x4a5e40                 compile its TU, diff it, check the whole TU
  match.py Sprite.cpp [0x... ...]   check a TU (and diff the listed functions)
  match.py --changed                check every TU edited since HEAD (pre-commit)
  match.py --list [--file F] [-n N] best candidates: non-exact, highest score first

Each TU is compiled once (in parallel when there are several) and every
annotated function in it is byte-compared with the original through
fastcmp.compare, the same check measure.py's byte audit uses. Results are
reported against CMR2PROGRESS/bytes.json: new exact matches, score changes
and regressions. The exit status is 1 when any function regressed or a
compile failed, so the command also works as a gate before committing.

Needs a full build + measure + prepare_fastcmp.py first (for the symbol maps).
"""
import argparse
import concurrent.futures
import difflib
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fastcmp as F  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "CMR2Decomp"
BASE = ROOT / "CMR2PROGRESS/bytes.json"
TSV = ROOT / "CMR2PROGRESS/nonmatching.tsv"
F_find_original = F.find_func
ANNOT = re.compile(r"// FUNCTION: CMR2 (0x[0-9a-fA-F]+)")

G, R, Y, B, D, X = "\033[32m", "\033[31m", "\033[33m", "\033[1m", "\033[2m", "\033[0m"


def color(on):
    global G, R, Y, B, D, X
    if not on:
        G = R = Y = B = D = X = ""


def find_func(c, name):
    """fastcmp.find_func plus constructors (??0Cls@@...), as the byte audit does."""
    result = F_find_original(c, name)
    if result[1] is not None:
        return result
    parts = name.split("::")
    if len(parts) >= 2 and parts[-1] == parts[-2]:
        prefix = "??0" + "@".join(reversed(parts[:-1])) + "@@"
        hits = [(i, s) for i, s in enumerate(c.syms)
                if s and s["sec"] > 0 and s["typ"] & 0x20 and s["name"].startswith(prefix)
                and c.secs[s["sec"] - 1]["name"] == ".text"]
        if len(hits) == 1:
            return hits[0]
    return result


def compare(addr, obj, src, name, size, coff):
    F.find_func = find_func
    try:
        return F.compare(addr, obj, src, name, size, coff=coff)
    finally:
        F.find_func = F_find_original


def tu_functions(path):
    return [int(m[1], 16) for m in ANNOT.finditer(path.read_text(encoding="latin1"))]


def resolve_tu(arg):
    p = Path(arg)
    for cand in (p, SRC / p.name, SRC / (p.name + ".cpp")):
        if cand.is_file() and cand.suffix == ".cpp":
            return cand.resolve()
    raise SystemExit(f"no such translation unit: {arg}")


def changed_tus():
    out = subprocess.run(["git", "status", "--porcelain", "--", "CMR2Decomp"], cwd=ROOT,
                         capture_output=True, text=True, check=True).stdout
    files = {Path(l[3:].split(" -> ")[-1]) for l in out.splitlines()}
    tus = {ROOT / f for f in files if f.suffix == ".cpp"}
    if any(f.suffix == ".h" for f in files):
        # A header change can move any TU that includes it.
        names = {f.name for f in files if f.suffix == ".h"}
        for cpp in SRC.glob("*.cpp"):
            text = cpp.read_text(encoding="latin1")
            if any(f'"{n}"' in text for n in names):
                tus.add(cpp)
    return sorted(tus)


def check_tu(src, base, sizes, workdir):
    obj = str(workdir / ("m_" + src.stem + ".obj"))
    t0 = time.time()
    try:
        F.compile_tu(str(src), obj)
    except Exception as error:  # compile error: report, keep going with other TUs
        return src, None, str(error), time.time() - t0
    coff = F.COFF(obj)
    results = {}
    for addr in tu_functions(src):
        row = base.get(hex(addr))
        if row is None:
            continue
        try:
            score, exact, detail, unknown = compare(addr, obj, str(src), row["symbol_name"],
                                                    sizes.get(addr), coff)
            results[addr] = dict(s=score, x=exact and not unknown, detail=detail, unknown=unknown)
        except Exception as error:
            results[addr] = dict(s=0.0, x=False, err=str(error))
    return src, results, None, time.time() - t0


REGS = re.compile(r"\b(?:e?[abcd]x|[abcd][lh]|e?[sd]i|e?[bs]p|[sd]il|[bs]pl)\b")
JUMP = re.compile(r"^(j\w+|call|loop\w*) 0x[0-9a-f]+$")


def shape_score(oi, ri):
    """Similarity with register names and branch targets blanked out: close to
    100% means only register allocation / branch offsets differ."""
    def norm(t):
        t = REGS.sub("R", t)
        return JUMP.sub(lambda m: m.group(1) + " L", t)
    a = [norm(t) for _, _, t in oi]
    b = [norm(t) for _, _, t in ri]
    return difflib.SequenceMatcher(None, a, b, autojunk=False).ratio()


def report(src, results, base, focus, args):
    regress = 0
    old_x = sum(base[hex(a)]["x"] for a in results)
    new_x = sum(r["x"] for r in results.values())
    lines = []
    for addr, r in results.items():
        b = base[hex(addr)]
        name = b["n"]
        old, new = b["s"], r["s"]
        if "err" in r:
            lines.append(f"  {R}ERROR{X}    {addr:#x} {name}: {r['err']}")
            regress += 1
        elif b["x"] and not r["x"]:
            lines.append(f"  {R}{B}LOST{X}     {addr:#x} {name}: exact -> {100 * new:.2f}%")
            regress += 1
        elif r["x"] and not b["x"]:
            lines.append(f"  {G}{B}EXACT{X}    {addr:#x} {name}: {100 * old:.2f}% -> exact")
        elif abs(new - old) > 1e-9 and not r["x"]:
            tag = f"{G}better{X}  " if new > old else f"{R}worse{X}   "
            if new < old:
                regress += 1
            lines.append(f"  {tag} {addr:#x} {name}: {100 * old:.2f}% -> {100 * new:.2f}%")
        elif addr in focus or args.all:
            state = f"{G}exact{X}" if r["x"] else f"{100 * new:.2f}%"
            lines.append(f"  {D}same{X}     {addr:#x} {name}: {state}")
    print(f"{B}{src.name}{X}: {new_x}/{len(results)} exact (baseline {old_x})")
    for line in lines:
        print(line)
    for addr in focus:
        r = results.get(addr)
        if not r or "detail" not in r:
            continue
        b = base[hex(addr)]
        oi, ri, sm = r["detail"]
        hdr = "EXACT" if r["x"] else f"{100 * r['s']:.2f}%  (ignoring registers {100 * shape_score(oi, ri):.2f}%)"
        print(f"\n{B}{addr:#x} {b['n']}{X}  orig {len(oi)}i / ours {len(ri)}i  {hdr}")
        if r["unknown"]:
            print(f"  {Y}unmapped symbols:{X}", ", ".join(sorted(r["unknown"])))
        if not r["x"]:
            F.show_diff(oi, ri, sm, ctx=args.context, full=args.full)
    return regress


def list_candidates(args):
    rows = []
    with TSV.open() as handle:
        next(handle)
        for line in handle:
            addr, name, file, size, rscore, bscore = line.rstrip("\n").split("\t")
            if args.file and file not in (Path(args.file).name, args.file + ".cpp"):
                continue
            if args.max_size and int(size) > args.max_size:
                continue
            rows.append([float(bscore), int(size), addr, name, file, None, None])
    if args.shape:
        # From the full build's objects: no compile, a few ms per function.
        base = json.loads(BASE.read_text())
        coffs = {}
        for row in rows:
            b = base.get(row[2])
            if not b or b.get("unknown"):
                continue
            obj = str(ROOT / "build" / (row[4][:-4] + ".obj"))
            coff = coffs.setdefault(obj, F.COFF(obj))
            try:
                _, _, (oi, ri, sm), _ = compare(int(row[2], 16), obj, str(SRC / row[4]),
                                                b["symbol_name"], row[1], coff)
            except Exception:
                continue
            row[5] = shape_score(oi, ri)
            row[6] = sum(max(i2 - i1, j2 - j1) for t, i1, i2, j1, j2 in sm.get_opcodes() if t != "equal")
        rows.sort(key=lambda r: (-(r[5] or 0), -r[0], r[1]))
    else:
        rows.sort(key=lambda r: (-r[0], r[1]))
    print(f"{'score':>7} {'shape':>7} {'diff':>5} {'bytes':>6}  {'address':10} {'file':22} name")
    for score, size, addr, name, file, shape, changed in rows[:args.n]:
        sh = f"{100 * shape:6.2f}%" if shape is not None else "      -"
        ch = f"{changed:5}" if changed is not None else "    -"
        print(f"{100 * score:6.2f}% {sh} {ch} {size:6}  {addr:10} {file:22} {name}")
    print(f"{D}{len(rows)} non-exact functions match the filter"
          f"{'; shape = similarity ignoring registers, diff = differing instructions' if args.shape else ''}{X}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("targets", nargs="*", help="function addresses and/or TU names")
    ap.add_argument("--changed", action="store_true", help="check TUs modified since HEAD")
    ap.add_argument("--list", action="store_true", help="list best candidates to match")
    ap.add_argument("--file", help="with --list: only this TU")
    ap.add_argument("--shape", action="store_true",
                    help="with --list: rank by similarity ignoring registers (regalloc-only first)")
    ap.add_argument("--max-size", type=int, help="with --list: only functions up to N bytes")
    ap.add_argument("-n", type=int, default=30, help="with --list: rows to show")
    ap.add_argument("--all", action="store_true", help="also list unchanged functions")
    ap.add_argument("--full", action="store_true", help="diff without collapsing equal runs")
    ap.add_argument("-C", "--context", type=int, default=3, help="diff context lines")
    ap.add_argument("--no-diff", action="store_true", help="never print diffs")
    ap.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--no-color", action="store_true")
    args = ap.parse_args()
    if hasattr(signal, "SIGPIPE"):
        signal.signal(signal.SIGPIPE, signal.SIG_DFL)  # quiet when piped into head
    color(sys.stdout.isatty() and not args.no_color)
    if args.list:
        return list_candidates(args)

    base = json.loads(BASE.read_text())
    _, sizes = F.load_meta()
    focus, tus = [], []
    for t in args.targets:
        if re.fullmatch(r"(0x)?[0-9a-fA-F]{6,8}", t):
            addr = int(t, 16)
            focus.append(addr)
            row = base.get(hex(addr))
            tus.append(SRC / row["f"] if row else Path(F.src_for(addr)))
        else:
            tus.append(resolve_tu(t))
    if args.changed:
        tus += changed_tus()
    tus = list(dict.fromkeys(p.resolve() for p in tus))
    if not tus and args.changed:
        print("no TU changed since HEAD")
        return 0
    if not tus:
        ap.error("nothing to check (give addresses, TUs or --changed)")
    if args.no_diff:
        focus = []

    workdir = Path(F.HERE)
    workdir.mkdir(parents=True, exist_ok=True)
    F.symmap_for(str(tus[0]))  # populate caches before threads race on them
    t0 = time.time()
    regress = 0
    with concurrent.futures.ThreadPoolExecutor(max(1, min(args.jobs, len(tus)))) as pool:
        futures = [pool.submit(check_tu, src, base, sizes, workdir) for src in tus]
        for fut in futures:
            src, results, error, secs = fut.result()
            if error:
                print(f"{R}{B}{src.name}: compile failed{X} ({secs:.1f}s)\n{error}")
                regress += 1
                continue
            regress += report(src, results, base, [a for a in focus if a in results], args)
            print(f"{D}  ({secs:.1f}s){X}")
    if len(tus) > 1:
        print(f"{len(tus)} TUs in {time.time() - t0:.1f}s")
    return 1 if regress else 0


if __name__ == "__main__":
    sys.exit(main())
