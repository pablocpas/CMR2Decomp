#!/usr/bin/env python3
"""iterate_match.py: Iterative compiler, diff tool, and optimizer for matching functions.

Usage:
  # 1. View score and disassembly diff:
  python3 scripts/iterate_match.py 0x4387a0 --diff
  python3 scripts/iterate_match.py 0x4387a0 --diff --full

  # 2. Test a new function body (file or stdin):
  #    If score improves, keeps the change; otherwise automatically reverts!
  python3 scripts/iterate_match.py 0x4387a0 --test new_body.c

  # 3. Run automated hill-climbing permutations:
  python3 scripts/iterate_match.py 0x4387a0 --permute [--rounds 10] [--max 50]
"""

import sys, os, re, difflib, argparse, shutil, tempfile, random
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO / "scripts"))
import fastcmp as F

TOKENS = re.compile(
    r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]'
)

def func_region(text, address):
    """Find start and end of function definition (including { ... }) by address."""
    marker = next(
        (
            m
            for m in re.finditer(r"//\s*FUNCTION:\s*CMR2\s+(0x[\da-fA-F]+)", text)
            if int(m[1], 16) == address
        ),
        None,
    )
    if marker is None:
        raise ValueError(f"Missing function annotation {address:#x}")
    start = text.index("\n", marker.end()) + 1
    depth = 0
    begin = None
    for token in TOKENS.finditer(text, start):
        if token[0] == "{":
            if begin is None:
                begin = token.start()
            depth += 1
        elif token[0] == "}":
            depth -= 1
            if depth == 0 and begin is not None:
                return begin, token.end()
    raise ValueError(f"Unterminated function {address:#x}")

def measure_func(addr, src=None, obj=None, compile_first=True):
    if src is None:
        src = F.src_for(addr)
    if obj is None:
        obj = F.HERE + "/fc_" + os.path.basename(src)[:-4] + ".obj"
    names, sizes = F.load_meta()
    name = names.get(addr, hex(addr))
    size = sizes.get(addr)
    if compile_first:
        F.compile_tu(src, obj)
    score, exact, (oi, ri, sm), unknown = F.compare(addr, obj, src, name, size, verbose=False)
    return {
        "score": score,
        "exact": exact,
        "oi": oi,
        "ri": ri,
        "sm": sm,
        "unknown": unknown,
        "name": name,
        "size": size,
        "src": src,
        "obj": obj,
    }

def print_diff(info, ctx=2, full=False):
    oi, ri, sm = info["oi"], info["ri"], info["sm"]
    print(f"\nDiff for {info['name']} @ {hex(addr_global)} (score: {info['score']*100:.2f}%{' EXACT' if info['exact'] else ''}):")
    print(f"{'ORIGINAL':<46} | {'OURS'}")
    print("-" * 46 + "-+-" + "-" * 46)
    F.show_diff(oi, ri, sm, ctx=ctx, full=full)

def try_replacement(addr, new_body_or_code, is_full_def=False):
    """Try replacing function body (or whole definition). Reverts if not improved."""
    src = F.src_for(addr)
    original_text = open(src, encoding="latin1").read()
    s, e = func_region(original_text, addr)
    
    # Measure baseline first
    base_info = measure_func(addr, src=src, compile_first=True)
    base_score = base_info["score"]
    print(f"Current baseline score: {base_score * 100:.2f}%{' EXACT' if base_info['exact'] else ''}")
    
    if is_full_def:
        # replace from declaration down to }
        # find declaration start after marker
        marker = next(
            m for m in re.finditer(r"//\s*FUNCTION:\s*CMR2\s+(0x[\da-fA-F]+)", original_text)
            if int(m[1], 16) == addr
        )
        decl_start = original_text.index("\n", marker.end()) + 1
        new_text = original_text[:decl_start] + new_body_or_code.strip() + "\n" + original_text[e+1:]
    else:
        new_body = new_body_or_code.strip()
        if not new_body.startswith("{"):
            new_body = "{\n" + new_body + "\n}"
        new_text = original_text[:s] + new_body + original_text[e:]

    open(src, "w", encoding="latin1").write(new_text)
    
    try:
        new_info = measure_func(addr, src=src, compile_first=True)
        new_score = new_info["score"]
        print(f"Candidate score: {new_score * 100:.2f}%{' EXACT' if new_info['exact'] else ''}")
        
        if new_score > base_score or (new_info["exact"] and not base_info["exact"]):
            delta = (new_score - base_score) * 100
            print(f"\033[92m[+] IMPROVEMENT: +{delta:.2f}% ({base_score*100:.2f}% -> {new_score*100:.2f}%)\033[0m")
            print("Changes KEPT in source.")
            return True, new_info
        else:
            delta = (new_score - base_score) * 100
            print(f"\033[91m[-] NO IMPROVEMENT: {delta:+.2f}%. Reverting back to original source.\033[0m")
            open(src, "w", encoding="latin1").write(original_text)
            return False, base_info
    except Exception as exc:
        print(f"\033[91m[-] Compilation or comparison failed: {exc}\033[0m")
        print("Reverting back to original source.")
        open(src, "w", encoding="latin1").write(original_text)
        return False, base_info

# Helper mutations for auto-permute
DECL_RE = re.compile(r"^    (?:unsigned |signed |const |struct )*[A-Za-z_]\w*[\s\*]+[A-Za-z_]\w*(?:\[[^\]]*\])?;\s*$")
SIMPLE_RE = re.compile(r"^\s+[^\s{}#/][^{}]*;\s*$")

def get_permutations(body):
    lines = body.split("\n")
    candidates = []
    
    # 1. Swap adjacent declarations
    decl_indices = [i for i, l in enumerate(lines) if DECL_RE.match(l)]
    for idx_a, idx_b in zip(decl_indices[:-1], decl_indices[1:]):
        if idx_b == idx_a + 1:
            nl = list(lines)
            nl[idx_a], nl[idx_b] = nl[idx_b], nl[idx_a]
            candidates.append(("decl_swap", f"{lines[idx_a].strip()} <-> {lines[idx_b].strip()}", "\n".join(nl)))
            
    # 2. if (x == 0) <-> if (!x)
    for i, l in enumerate(lines):
        if " == 0)" in l:
            nl = list(lines)
            nl[i] = l.replace(" == 0)", " == 0)") # placeholder
            # !cond
            m = re.search(r"if \(([\w.\->\[\]]+) == 0\)", l)
            if m:
                nl[i] = l[:m.start()] + f"if (!{m.group(1)})" + l[m.end():]
                candidates.append(("if_zero", f"line {i}", "\n".join(nl)))
        elif " != 0)" in l:
            m = re.search(r"if \(([\w.\->\[\]]+) != 0\)", l)
            if m:
                nl = list(lines)
                nl[i] = l[:m.start()] + f"if ({m.group(1)})" + l[m.end():]
                candidates.append(("if_nonzero", f"line {i}", "\n".join(nl)))

    # 3. Simple independent statement moves
    for i in range(len(lines) - 1):
        if SIMPLE_RE.match(lines[i]) and SIMPLE_RE.match(lines[i+1]):
            # only if same indentation and neither is a control structure
            if len(lines[i]) - len(lines[i].lstrip()) == len(lines[i+1]) - len(lines[i+1].lstrip()):
                nl = list(lines)
                nl[i], nl[i+1] = nl[i+1], nl[i]
                candidates.append(("stmt_swap", f"lines {i}<->{i+1}", "\n".join(nl)))
                
    return candidates

def run_permute(addr, rounds=5, max_cand=40):
    src = F.src_for(addr)
    original_text = open(src, encoding="latin1").read()
    s, e = func_region(original_text, addr)
    cur_info = measure_func(addr, src=src, compile_first=True)
    cur_score = cur_info["score"]
    print(f"Starting permute for {cur_info['name']} @ {hex(addr)}: {cur_score*100:.2f}%")
    
    seen = {original_text[s:e]}
    body = original_text[s:e]
    
    for r in range(rounds):
        muts = [m for m in get_permutations(body) if m[2] not in seen]
        if not muts:
            print("No more permutations to explore.")
            break
        random.shuffle(muts)
        muts = muts[:max_cand]
        print(f"Round {r+1}/{rounds}: testing {len(muts)} candidates...")
        improved = False
        for kind, desc, cand_body in muts:
            seen.add(cand_body)
            new_text = original_text[:s] + cand_body + original_text[e:]
            open(src, "w", encoding="latin1").write(new_text)
            try:
                info = measure_func(addr, src=src, compile_first=True)
                if info["score"] > cur_score:
                    delta = (info["score"] - cur_score) * 100
                    print(f"  \033[92m[+] Better: {cur_score*100:.2f}% -> {info['score']*100:.2f}% (+{delta:.2f}%) via {kind} ({desc})\033[0m")
                    cur_score = info["score"]
                    cur_info = info
                    body = cand_body
                    original_text = new_text
                    improved = True
                    if info["exact"]:
                        print("  \033[92m[***] EXACT MATCH 100%! [***]\033[0m")
                        return cur_info
                    break
            except Exception:
                pass
        if not improved:
            print("  No candidate improved score in this round.")
            open(src, "w", encoding="latin1").write(original_text)
            break
            
    open(src, "w", encoding="latin1").write(original_text)
    print(f"Final score: {cur_score*100:.2f}%")
    return cur_info

addr_global = 0

def main():
    global addr_global
    parser = argparse.ArgumentParser(description="Decompilation Match Optimizer & Diff Runner")
    parser.add_argument("addr", help="Function address (e.g. 0x4387a0)")
    parser.add_argument("--diff", action="store_true", help="Show disassembly diff")
    parser.add_argument("--full", action="store_true", help="Show full disassembly diff without collapsing equal lines")
    parser.add_argument("--context", type=int, default=3, help="Context lines around diffs (default 3)")
    parser.add_argument("--test", help="Test a replacement body file (reverts if no improvement)")
    parser.add_argument("--permute", action="store_true", help="Run automated hill-climbing permutations")
    parser.add_argument("--rounds", type=int, default=5, help="Rounds for permute")
    parser.add_argument("--max", type=int, default=40, help="Max candidates per round")
    
    args = parser.parse_args()
    addr_global = int(args.addr, 16)
    
    if args.test:
        content = Path(args.test).read_text(encoding="latin1")
        try_replacement(addr_global, content)
    elif args.permute:
        run_permute(addr_global, rounds=args.rounds, max_cand=args.max)
    else:
        info = measure_func(addr_global, compile_first=True)
        print(f"{info['name']} @ {args.addr}: orig {info['size']}B  score: {info['score']*100:.2f}%{'  EXACT' if info['exact'] else ''}")
        if info['unknown']:
            print("Unmapped reloc symbols:", info['unknown'])
        if args.diff:
            print_diff(info, ctx=args.context, full=args.full)

if __name__ == "__main__":
    try:
        main()
    except BrokenPipeError:
        sys.stderr.close()
        sys.exit(0)
