#!/usr/bin/env python3
"""Move annotated functions out of a translation unit into a new one.

  split_tu.py SOURCE.cpp NEW.cpp 0xADDR [0xADDR ...] [--dry-run]

MSVC6 carries state from earlier functions of a TU into later ones (for
example, any floating-point code changes register allocation in some later
integer-only functions). The original game was built from about 190 C++
objects, so a function may need a different neighbourhood than the large TUs
here give it.

NEW.cpp starts with SOURCE's #include lines and the functions (in address
order). It is compiled, and for every identifier the compiler reports as
undeclared, the declaration that precedes the first moved function in SOURCE
is copied over: prototypes as they are, global definitions as extern
declarations, types and macros whole. The functions are then removed from
SOURCE. Check the result with match.py and a full build before committing.
"""
import argparse
import os
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fastcmp as F  # noqa: E402
from permute_batch import func_region  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
UNDECLARED = re.compile(r"error C20(?:65|61|46|39|27|11|04): '([A-Za-z_][\w:]*)'")


def statement_end(text, start):
    """End of the top-level declaration starting at start (after ';' or '};')."""
    depth = 0
    k = start
    while k < len(text):
        ch = text[k]
        if ch in "{(":
            depth += 1
        elif ch in "})":
            depth -= 1
        elif ch == ";" and depth == 0:
            return k + 1
        elif ch == "\n" and depth == 0 and text[start:k].lstrip().startswith("#") and not text[k - 1] == "\\":
            return k + 1
        k += 1
    return k


def top_level_starts(text):
    """Offsets of top-level statements (depth 0), skipping comments."""
    starts, depth, k, line_start = [], 0, 0, True
    while k < len(text):
        if text.startswith("//", k):
            k = text.index("\n", k) if "\n" in text[k:] else len(text)
            continue
        if text.startswith("/*", k):
            k = text.index("*/", k) + 2
            continue
        ch = text[k]
        if ch == '"' or ch == "'":
            q = ch
            k += 1
            while k < len(text) and text[k] != q:
                k += 2 if text[k] == "\\" else 1
            k += 1
            continue
        if depth == 0 and not ch.isspace() and line_start:
            starts.append(k)
        if ch == "\n":
            line_start = True
        elif not ch.isspace():
            line_start = depth == 0 and ch in ";}" or (depth == 0 and ch == "\n")
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                line_start = depth == 0
            elif ch == ";" and depth == 0:
                line_start = True
        k += 1
    return starts


def declaration_of(prefix, name, starts):
    """Text to copy for the last top-level declaration of name in prefix."""
    plain = name.split("::")[-1]
    best = None
    for s in starts:
        e = statement_end(prefix, s)
        stmt = prefix[s:e]
        head = stmt.split("{")[0] if not stmt.lstrip().startswith(("struct", "class", "union", "enum", "typedef", "#")) else stmt
        if re.search(r"(?<![\w:])" + re.escape(plain) + r"\b", head):
            best = (s, e, stmt)
    if best is None:
        return None
    s, e, stmt = best
    body_start = stmt.find("{")
    if stmt.lstrip().startswith(("#", "struct", "class", "union", "enum", "typedef", "extern")) or "(" in stmt.split("=")[0] and body_start < 0:
        return stmt.strip()
    if body_start >= 0 and "(" in stmt[:body_start] and "=" not in stmt[:body_start]:
        return stmt[:body_start].rstrip() + ";"        # a function definition: its prototype
    decl = stmt.split("=")[0].rstrip().rstrip(";")      # a global definition: extern it
    decl = re.sub(r"^\s*static\s+", "", decl)
    return "extern " + decl.strip() + ";"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("source")
    ap.add_argument("new")
    ap.add_argument("addresses", nargs="+")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()
    src = Path(args.source)
    text = src.read_text(encoding="latin1")
    addrs = sorted(int(a, 16) for a in args.addresses)
    regions = []
    for a in addrs:
        b, e = func_region(text, a)
        start = text.rfind("// FUNCTION: CMR2", 0, b)
        # take the comment block above the annotation too
        while True:
            prev = text.rfind("\n", 0, start - 1) + 1
            if text[prev:start].lstrip().startswith("//") and prev < start:
                start = prev
            else:
                break
        regions.append((start, e))
    first = min(s for s, _ in regions)
    prefix = text[:first]
    starts = top_level_starts(prefix)
    includes = list(dict.fromkeys(l for l in prefix.splitlines() if l.startswith("#include")))
    funcs = "\n\n".join(text[s:e].strip() for s, e in sorted(regions))
    decls = []
    new = Path(args.new)
    obj = str(Path(F.HERE) / ("split_" + new.stem + ".obj"))
    for _ in range(60):
        body = "\n".join(includes) + "\n\n" + "\n\n".join(decls) + ("\n\n" if decls else "") + funcs + "\n"
        tmp = Path(F.HERE) / new.name
        tmp.write_text(body, encoding="latin1")
        try:
            F.compile_tu(str(tmp), obj)
            break
        except RuntimeError as error:
            missing = list(dict.fromkeys(UNDECLARED.findall(str(error))))
            if not missing:
                sys.exit("cannot resolve:\n" + str(error))
            added = False
            for name in missing:
                d = declaration_of(prefix, name, starts)
                if d and d not in decls:
                    decls.append(d)
                    added = True
            if not added:
                sys.exit("no declaration found for " + ", ".join(missing) + "\n" + str(error))
    else:
        sys.exit("gave up")
    # Declarations may depend on each other: keep the source order.
    decls.sort(key=lambda d: prefix.find(d.replace("extern ", "", 1).rstrip(";")[:40]))
    body = "\n".join(includes) + "\n\n" + "\n\n".join(decls) + "\n\n" + funcs + "\n"
    if args.dry_run:
        print(body)
        return
    new.write_text(body, encoding="latin1")
    for s, e in sorted(regions, reverse=True):
        text = text[:s] + text[e:].lstrip("\n")
    src.write_text(text, encoding="latin1")
    print(f"moved {len(addrs)} function(s) to {new} with {len(decls)} declarations")


if __name__ == "__main__":
    main()
