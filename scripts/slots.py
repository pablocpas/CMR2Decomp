#!/usr/bin/env python3
"""Show how our stack slots map onto the original's for one function.

  slots.py 0xADDR

Aligns the two disassemblies with registers, branch targets and stack offsets
blanked out, then, for each aligned pair that differs only in a stack offset,
records ours -> original. A clean one-to-one table means the code is right and
only the slot assignment differs: MSVC6 orders slots by how often each
variable is referenced (most used nearest the frame base), so a variable
sitting too far out is used too rarely in our source (or the original reuses
it for something else), and ties depend on the names (rename_search.py).
"""
import collections
import difflib
import os
from pathlib import Path
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fastcmp as F  # noqa: E402
import match as M  # noqa: E402

FRAME = re.compile(r"\[(ebp|esp)(?: ([+-]) (0x[0-9a-f]+|\d+))?\]")


def offsets(text):
    out = []
    for m in FRAME.finditer(text):
        v = int(m.group(3), 0) if m.group(3) else 0
        out.append((m.group(1), -v if m.group(2) == "-" else v))
    return out


def main():
    addr = int(sys.argv[1], 16)
    src = Path(F.src_for(addr))
    base = M.json.loads(M.BASE.read_text())
    row = base.get(hex(addr))
    _, sizes = F.load_meta()
    obj = str(Path(F.HERE) / (src.stem + "_slots.obj"))
    F.compile_tu(str(src), obj)
    coff = F.COFF(obj)
    name = row["symbol_name"] if row else None
    _, _, (oi, ri, _), _ = M.compare(addr, obj, str(src), name, sizes.get(addr), coff)
    norm = lambda t: M.REGS.sub("R", M.JUMP.sub(lambda m: m.group(1) + " L", FRAME.sub("[S]", t)))
    a = [norm(t) for _, _, t in oi]
    b = [norm(t) for _, _, t in ri]
    pairs = collections.Counter()
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes():
        if tag != "equal":
            continue
        for k in range(i2 - i1):
            for (rb, ro), (_, oo) in zip(offsets(ri[j1 + k][2]), offsets(oi[i1 + k][2])):
                pairs[(rb, ro, oo)] += 1
    by_ours = collections.defaultdict(collections.Counter)
    for (reg, ro, oo), n in pairs.items():
        by_ours[(reg, ro)][oo] += n
    print(f"{'ours':>12} -> original (count)")
    for (reg, ro), cnt in sorted(by_ours.items(), key=lambda kv: (kv[0][0], kv[0][1])):
        mark = "" if list(cnt) == [ro] else "   *"
        targets = ", ".join(f"{reg}{o:+#x} ({n})" for o, n in cnt.most_common())
        print(f"{reg}{ro:+#x}".rjust(12) + f" -> {targets}{mark}")


if __name__ == "__main__":
    main()
