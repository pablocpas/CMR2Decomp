#!/usr/bin/env python3
"""Search equivalent source variants on an isolated snapshot of a matching build.

Uses the mutators in scripts/permute_mutators.py (or --mutator PATH),
MSVC6/Wine and the current build/CMR2PROGRESS reports. Compilers run across
several functions simultaneously. Results stay in --output; changes.patch is
for review and is never applied to the main source tree by this command.

Example:
    python3 scripts/permute_batch.py --output scripts/work/search --limit 140
"""
import argparse
import collections
import concurrent.futures
import difflib
import hashlib
import importlib.util
import json
import os
import pathlib
import re
import shutil
import sys
import tempfile
import threading
import time

import fastcmp as F

ROOT = pathlib.Path(__file__).resolve().parents[1]
pure = {"FixMul", "FixDiv", "FixMulShift32", "FixSqrt"}

TOKENS = re.compile(
    r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]'
)


def func_region(text, address):
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


def parse_addresses(text):
    """Comma-separated original addresses for --addresses."""
    try:
        return {int(part, 0) for part in text.split(",") if part.strip()}
    except ValueError as error:
        raise ValueError(
            "--addresses expects comma-separated addresses like 0x46b440"
        ) from error


def reorders_floating(body, changed_lines):
    # MSVC6/x87 can keep temporaries at extended precision. Reordering a float
    # calculation can change spills even when the source expressions are unchanged.
    floating_names = set()
    for declaration in re.findall(r"\b(?:float|double)\s+([^;{}]+);", body):
        for item in declaration.split(","):
            match = re.match(r"\s*\**\s*(\w+)", item)
            if match:
                floating_names.add(match[1])
    for line in changed_lines:
        if floating_names.intersection(P._ID.findall(line)) or re.search(
            r"\b(float|double|__int64)\b|\b\d+\.\d*|g_net|g_oneOver|RAND_|TRAIL_RANDOM|m_oneOver65536|m_65536",
            line,
        ):
            return True
    return False


def mutations(body, signature=""):
    if re.search(r"\bvolatile\b|\b__asm\b", body):
        return []
    integral = set(
        re.findall(
            r"\b(?:unsigned\s+|signed\s+)?(?:int|short|long|char|BYTE|DWORD|bool)\s+(\w+)\b",
            signature + body,
        )
    )
    lines = body.split("\n")
    floating = bool(re.search(r"\b(float|double)\b|\b(g_net|g_oneOver|RAND_)", body))
    result = P.moves(lines) + P.ifswaps(lines) + getattr(P, "idioms", lambda _: [])(lines)
    if not floating:
        result += P.swaps(lines) + P.obos(lines)
    safe = []
    for kind, detail, newlines in result:
        changed = [(a, b) for a, b in zip(lines, newlines) if a != b]
        if kind == "move":
            if reorders_floating(body, [a for a, _ in changed]):
                continue
            if any(P._mem(P._lhs(a)) for a, b in changed):
                continue
            if "__asm" in body or any(P._calls(a) for a, b in changed):
                continue
            if re.search(r"\bvolatile\b", body):
                continue
            if re.search(
                r"(?<!&)\&\s*\b(?:"
                + "|".join(
                    re.escape(v)
                    for a, b in changed
                    for v in P._ID.findall(P._lhs(a) or "")
                )
                + r")\b",
                body,
            ):
                continue
        if kind in ("swap", "flip"):
            if any(re.search(r"\+\+|--", a) for a, b in changed):
                continue
            if kind == "swap" and detail.rsplit(":", 1)[-1] in ("+", "*"):
                identifiers = set(P._ID.findall("\n".join(a for a, b in changed)))
                if (
                    identifiers
                    - integral
                    - pure
                    - {
                        "return",
                        "int",
                        "short",
                        "char",
                        "BYTE",
                        "DWORD",
                        "unsigned",
                        "signed",
                        "if",
                        "else",
                    }
                ):
                    continue
            if any(
                any(c.rstrip(" (") not in pure for c in P._calls(a)) for a, b in changed
            ):
                continue
            # Reassociation of multiplication/addition involving FP is excluded above.
        if kind == "obo":
            identifiers = set(P._ID.findall("\n".join(a for a, b in changed)))
            if (
                identifiers
                - integral
                - {
                    "if",
                    "for",
                    "while",
                    "else",
                    "int",
                    "short",
                    "char",
                    "BYTE",
                    "DWORD",
                    "unsigned",
                    "signed",
                    "return",
                }
            ):
                continue
            oldtext = "\n".join(a for a, b in changed)
            # No opaque floating fields/globals, overflowing bounds or unsigned wrap endpoints.
            if any(x in oldtext for x in ("->", "[", "::", ".", "g_")):
                continue
            if any(
                abs(int(m, 0)) >= 0x7FFFFFFE
                for m in re.findall(r"\b(0x[\da-fA-F]+|\d+)\b", oldtext)
            ):
                continue
        candidate = "\n".join(newlines)
        if candidate != body:
            safe.append((kind, detail, candidate))
    # Only split nested verified integer helpers, without reordering other calls.
    for kind, detail, newlines in P.unnests(lines):
        original = next((a for a, b in zip(lines, newlines) if a != b), "")
        calls = [c.rstrip(" (") for c in P._calls(original)]
        lhs = P._lhs(original)
        wide = set(
            re.findall(
                r"\b(?:unsigned\s+|signed\s+)?(?:int|long|DWORD)\s+(\w+)\b",
                signature + body,
            )
        )
        if lhs and lhs.strip() in wide and calls and all(c in pure for c in calls):
            if P._ID.findall(original).count(lhs.strip()) != 1 or re.search(
                r"\+\+|--", original
            ):
                continue
            safe.append((kind, detail, "\n".join(newlines)))
    return list(dict(((k, t), (k, d, t)) for k, d, t in safe).values())


def multiline_ifswaps(body):
    """Reverse two unbraced expression statements, including wrapped calls."""
    lines = body.split("\n")

    def statement(start, indent):
        for end in range(start, len(lines)):
            line = lines[end]
            if not line.startswith(indent + "    "):
                return None
            if not line.rstrip().endswith(";"):
                continue
            text = "\n".join(lines[start:end + 1])
            cleaned = TOKENS.sub(
                lambda m: m[0] if m[0] in ("{", "}") else "", text
            ).strip()
            if cleaned.count(";") != 1 or any(c in cleaned for c in "{}#"):
                return None
            if re.search(r"\b(if|else|for|while|do|switch|case|default|goto)\b", cleaned):
                return None
            stack = []
            for c in cleaned:
                if c in "([":
                    stack.append(c)
                elif c in ")]":
                    if not stack or stack.pop() != {')': '(', ']': '['}[c]:
                        return None
            return None if stack else end + 1
        return None

    result = []
    for i, line in enumerate(lines):
        match = re.fullmatch(r"(\s*)if \((.*)\)", line)
        if not match:
            continue
        indent, condition = match.groups()
        middle = statement(i + 1, indent)
        if middle is None or middle >= len(lines) or lines[middle] != indent + "else":
            continue
        end = statement(middle + 1, indent)
        if end is None:
            continue
        swapped = (
            lines[:i] + [indent + "if (!(" + condition + "))"]
            + lines[middle + 1:end] + [indent + "else"]
            + lines[i + 1:middle] + lines[end:]
        )
        result.append(("ifswap-multiline", str(i), "\n".join(swapped)))
    return result


def forms(body):
    if re.search(r"\bvolatile\b|\b__asm\b", body):
        return []
    lines = body.split("\n")
    out = multiline_ifswaps(body)
    # Swap independent scalar/pointer initializers; unused plain declarations were unproductive.
    decl = re.compile(
        r"^(\s+)((?:(?:unsigned|signed|const)\s+)*(?:int|short|char|long|BYTE|DWORD|bool)|(?:\w+(?:::\w+)*)\s*\*+)\s+(\w+)\s*=\s*([^;{}]+);\s*$"
    )
    for i in range(len(lines) - 1):
        a, b = decl.fullmatch(lines[i]), decl.fullmatch(lines[i + 1])
        if not a or not b or a[1] != b[1]:
            continue
        if a[3] in P._ID.findall(b[4]) or b[3] in P._ID.findall(a[4]):
            continue
        if P._calls(a[4]) or P._calls(b[4]) or "volatile" in body:
            continue
        if re.search(r"\b(new|delete)\b|\+\+|--|=", a[4] + b[4]):
            continue
        if reorders_floating(body, [lines[i], lines[i + 1]]):
            continue
        n = lines[:]
        n[i], n[i + 1] = n[i + 1], n[i]
        out.append(("init", str(i), "\n".join(n)))
    # Keep loop initialization explicit; its placement can affect instruction scheduling.
    for i, l in enumerate(lines):
        m = re.fullmatch(r"(\s*)for \((\w+) = ([^;,]+); (.*)\) \{", l)
        if not m:
            continue
        ind, var, expr, rest = m.groups()
        if P._calls(expr) or re.search(r"\+\+|--|=", expr):
            continue
        n = (
            lines[:i]
            + [ind + var + " = " + expr + ";", ind + "for (; " + rest + ") {"]
            + lines[i + 1 :]
        )
        out.append(("loop-init", str(i), "\n".join(n)))
    # A selector at a disjoint, explicit struct offset can be read before the value store.
    pattern = re.compile(
        r"(?P<indent>^[ \t]+)(?P<target>[^;{}\n]+\.field_0x(?P<off>[\da-fA-F]+))\s*=\s*(?P<expr>[^;{}]+);\n(?P=indent)switch \((?P<selector>[^\n()]+\.field_0x(?P<soff>[\da-fA-F]+))\) \{",
        re.M,
    )
    for m in pattern.finditer(body):
        target = m["target"].strip()
        selector = m["selector"].strip()
        if re.search(r"\+\+|--|\b(new|delete)\b", target + selector + m["expr"]):
            continue
        if target.rsplit(".field_", 1)[0] != selector.rsplit(".field_", 1)[0]:
            continue
        if abs(int(m["off"], 16) - int(m["soff"], 16)) < 4:
            continue
        expr = " ".join(m["expr"].split())
        calls = [c.rstrip(" (") for c in P._calls(expr)]
        if not calls or any(c not in pure for c in calls):
            continue
        if "perm_value" in body or "perm_selector" in body:
            continue
        ind = m["indent"]
        replacement = (
            ind
            + "int perm_value = "
            + expr
            + ";\n"
            + ind
            + "int perm_selector = "
            + selector
            + ";\n"
            + ind
            + target
            + " = perm_value;\n"
            + ind
            + "switch (perm_selector) {"
        )
        out.append(
            (
                "selector",
                str(m.start()),
                body[: m.start()] + replacement + body[m.end() :],
            )
        )
    return out


def layout_forms(body):
    """Change plain local storage layout without moving initializers or calls."""
    if re.search(r"\bvolatile\b|\b__asm\b", body):
        return []
    lines = body.split("\n")
    # FixVector is a plain aggregate of three ints, without constructors.
    declaration = re.compile(
        r"^[ \t]+(?:(?:unsigned|signed|const)\s+)*"
        r"(?:(?:int|short|long|char|BYTE|DWORD|bool|FixVector)\s+\**\s*"
        r"|[A-Za-z_]\w*(?:::\w+)*\s*\*+\s*)"
        r"\w+(?:\[(?:\d+|0x[\da-fA-F]+)\])?;[ \t]*$"
    )
    # Only declarations at function entry: changing nested scope is unnecessary.
    positions = []
    for i in range(1, len(lines)):
        if not lines[i].strip():
            continue
        if declaration.fullmatch(lines[i]):
            positions.append(i)
        else:
            break
    result = []
    for a, b in zip(positions, positions[1:]):
        changed = lines[:]
        changed[a], changed[b] = changed[b], changed[a]
        result.append(("local-layout", f"{a}:{b}", "\n".join(changed)))
    if len(positions) > 2:
        changed = lines[:]
        for a, b in zip(positions, reversed(positions)):
            changed[a] = lines[b]
        result.append(("local-layout", "reverse", "\n".join(changed)))
    return result


def candidate_forms(body, signature=""):
    return mutations(body, signature) + forms(body) + layout_forms(body)


def packed_plan(variants):
    """Each compilation contains at most one variant of each distinct function."""
    return [
        [(address, choices[i]) for address, choices in variants.items() if i < len(choices)]
        for i in range(max(map(len, variants.values()), default=0))
    ]


def replace_bodies(text, replacements):
    edits = []
    for address, body in replacements.items():
        a, b = func_region(text, address)
        edits.append((a, b, body))
    for a, b, body in sorted(edits, reverse=True):
        text = text[:a] + body + text[b:]
    return text


def score_pack(task):
    filename, text, addresses = task
    with tempfile.TemporaryDirectory(prefix="packed-", dir=out) as directory:
        path = pathlib.Path(directory) / filename
        path.write_text(text, encoding="latin1")
        obj = str(path.with_suffix(".obj"))
        try:
            F.compile_tu(str(path), obj)
        except Exception as error:
            return {a: {"score": -1, "exact": False, "unknown": [], "error": str(error)} for a in addresses}
        result = {}
        coff = F.COFF(obj)
        for address in addresses:
            try:
                sc, ex, (_, ri, _), unknown = F.compare(
                    address, obj, str(path), names[address], sizes[address], coff=coff
                )
                result[address] = {
                    "score": sc, "exact": ex and not unknown,
                    "unknown": sorted(unknown),
                    "fingerprint": digest(str([(s, t) for _, s, t in ri])),
                }
            except Exception as error:
                result[address] = {"score": -1, "exact": False, "unknown": [], "error": str(error)}
        return result


def search_packed(targets, jobs, rounds):
    grouped = collections.defaultdict(list)
    for address, _, _, filename in targets:
        grouped[filename].append(address)
    texts = {f: (root / "CMR2Decomp" / f).read_text(encoding="latin1") for f in grouped}
    baselines = {}
    best = {}
    records = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        files = list(grouped)
        tasks = [(f, texts[f], grouped[f]) for f in files]
        for filename, measurements in zip(files, pool.map(score_pack, tasks)):
            statistics["compiled"] += 1
            for address, measurement in measurements.items():
                if measurement["score"] < 0 or measurement["unknown"]:
                    raise RuntimeError(f"{address:#x}: invalid packed baseline: {measurement}")
                baselines[address] = measurement
                best[address] = measurement
                records[address] = {
                    "address": hex(address), "file": filename, "name": names[address],
                    "before": measurement["score"], "after": measurement["score"],
                    "exact": measurement["exact"], "variants": 0, "path": [],
                }
        seen = collections.defaultdict(set)
        active = set(best)
        for round_index in range(rounds):
            plans = []
            tasks = []
            for filename, addresses in grouped.items():
                variants = {}
                for address in addresses:
                    if address not in active or best[address]["exact"]:
                        continue
                    a, b = func_region(texts[filename], address)
                    body = texts[filename][a:b]
                    seen[address].add(body)
                    choices = []
                    for kind, detail, candidate in candidate_forms(body, texts[filename][texts[filename].rfind("// FUNCTION:", 0, a):a]):
                        if enabled_kinds and kind not in enabled_kinds:
                            continue
                        if candidate == body or candidate in seen[address]:
                            continue
                        seen[address].add(candidate)
                        choices.append((kind, detail, candidate))
                    variants[address] = choices
                    records[address]["variants"] += len(choices)
                for plan in packed_plan(variants):
                    plans.append((filename, plan))
                    tasks.append((filename, replace_bodies(texts[filename], {a: c[2] for a, c in plan}), [a for a, _ in plan]))
            if not tasks:
                break
            print(f"PACK round={round_index+1} compiles={len(tasks)} variants={sum(len(p) for _, p in plans)}", flush=True)
            winners = {}
            for (filename, plan), measurements in zip(plans, pool.map(score_pack, tasks)):
                statistics["compiled"] += 1
                statistics["candidate_functions"] += len(plan)
                for address, candidate in plan:
                    measurement = measurements[address]
                    if measurement["score"] < 0:
                        statistics["errors"] += 1
                        continue
                    if not measurement["unknown"] and measurement["score"] > best[address]["score"] + 1e-9:
                        old = winners.get(address)
                        if old is None or measurement["score"] > old[1]["score"]:
                            winners[address] = candidate, measurement
            active = set(winners)
            for filename, addresses in grouped.items():
                chosen = {a: winners[a][0][2] for a in addresses if a in winners}
                if chosen:
                    texts[filename] = replace_bodies(texts[filename], chosen)
                    (root / "CMR2Decomp" / filename).write_text(texts[filename], encoding="latin1")
            for address, (candidate, measurement) in winners.items():
                best[address] = measurement
                record = records[address]
                record.update(after=measurement["score"], exact=measurement["exact"])
                record["path"].append({"round": round_index+1, "mutation": candidate[:2], "score": measurement["score"]})
                family[candidate[0]] += 1
                print(f"GAIN {address:#x} {record['before']*100:.2f} -> {record['after']*100:.2f} {candidate[0]}", flush=True)
            progress[:] = list(records.values())
            checkpoint()
            if not winners:
                break
    # Verify the combined winning source; a neighbour's inlining can affect code.
    tasks = [(f, texts[f], grouped[f]) for f in grouped]
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        for filename, measurements in zip(grouped, pool.map(score_pack, tasks)):
            statistics["compiled"] += 1
            rejected = {}
            for address, measurement in measurements.items():
                record = records[address]
                if record["after"] <= record["before"]:
                    continue
                if measurement["score"] <= baselines[address]["score"] or measurement["unknown"]:
                    original = (out / "original-source" / filename).read_text(encoding="latin1")
                    a, b = func_region(original, address)
                    rejected[address] = original[a:b]
                    record.update(after=record["before"], exact=baselines[address]["exact"], rejected="combined source did not improve")
                else:
                    record.update(after=measurement["score"], exact=measurement["exact"])
            if rejected:
                texts[filename] = replace_bodies(texts[filename], rejected)
                (root / "CMR2Decomp" / filename).write_text(texts[filename], encoding="latin1")
    progress[:] = list(records.values())
    checkpoint()


def digest(text):
    return hashlib.sha256(text.encode("latin1")).hexdigest()


def score(args):
    addr, src, text, key = args
    d = tempfile.mkdtemp(prefix="candidate-", dir=out)
    try:
        p = pathlib.Path(d) / pathlib.Path(src).name
        p.write_text(text, encoding="latin1")
        obj = d + "/o.obj"
        begin = time.monotonic()
        F.compile_tu(str(p), obj)
        sc, ex, (oi, ri, sm), unknown = F.compare(
            addr, obj, src, names[addr], sizes[addr]
        )
        # Instruction/size fingerprint distinguishes generated implementations.
        fingerprint = digest(str([(size, txt) for _, size, txt in ri]))
        return key, {
            "score": sc,
            "exact": ex and not unknown,
            "unknown": sorted(unknown),
            "fingerprint": fingerprint,
            "seconds": time.monotonic() - begin,
        }
    except Exception as error:
        return key, {"score": -1, "exact": False, "unknown": [], "error": str(error)}
    finally:
        shutil.rmtree(d)


def evaluate(addr, src, texts, pool):
    keys = [digest(identity + hex(addr) + pathlib.Path(src).name + t) for t in texts]
    with cache_lock:
        todo = list(
            dict(
                (k, (addr, src, t, k))
                for k, t in zip(keys, texts)
                if k not in cache or cache[k]["score"] < 0
            ).values()
        )
        statistics["cache_hits"] += len(texts) - len(todo)
    for key, value in pool.map(score, todo):
        with cache_lock:
            cache[key] = value
            statistics["compiled"] += 1
    with cache_lock:
        return [cache[k] for k in keys]


def checkpoint():
    with cache_lock:
        cached = dict(cache)
        counts = dict(statistics)
        improvements = dict(family)
    temporary = cachepath.with_suffix(".tmp")
    temporary.write_text(json.dumps(cached))
    temporary.replace(cachepath)
    (out / "search-results.json").write_text(
        json.dumps(
            {
                "elapsed_seconds": time.monotonic() - start,
                "statistics": counts,
                "mutation_improvements": improvements,
                "mutation_kinds": sorted(enabled_kinds) if enabled_kinds else "all",
                "build_identity": identity,
                "functions": progress,
            },
            indent=2,
        )
    )


def search(addr, pool, max_rounds, max_neutral):
    src = str(root / "CMR2Decomp" / sources[addr])
    path = pathlib.Path(src)
    with source_lock:
        text = path.read_text(encoding="latin1")
    a, b = P.func_region(text, addr)
    initial = text[a:b]
    current = evaluate(addr, src, [text], pool)[0]
    base = current["score"]
    bestbody = initial
    best = current
    seen = {initial}
    frontier = [initial]
    log = []
    neutral = 0
    if base < 0:
        raise RuntimeError(
            f"{addr:#x}: baseline compilation failed: "
            + current.get("error", "unknown error")
        )
    if current["exact"]:
        return {
            "address": hex(addr),
            "before": base,
            "after": base,
            "exact": True,
            "unchanged": True,
        }
    if current["unknown"]:
        return {
            "address": hex(addr),
            "before": base,
            "after": base,
            "error": "unresolved symbols",
        }
    for rnd in range(max_rounds):
        candidates = []
        for body in frontier:
            for kind, detail, t in mutations(
                body, text[text.rfind("// FUNCTION:", 0, a) : a]
            ) + forms(body) + layout_forms(body):
                if enabled_kinds and kind not in enabled_kinds:
                    continue
                if t in seen:
                    continue
                seen.add(t)
                candidates.append((kind, detail, t))
        if not candidates:
            break
        candidates.sort(
            key=lambda r: (
                {
                    "ifswap": 0,
                    "flip": 1,
                    "swap": 2,
                    "obo": 3,
                    "move": 4,
                    "unnest": 5,
                    "decl": 6,
                }.get(r[0], 9),
                r[1],
            )
        )
        results = evaluate(
            addr, src, [text[:a] + t + text[b:] for _, _, t in candidates], pool
        )
        with cache_lock:
            statistics["candidate_functions"] += len(candidates)
            statistics["distinct_code"] += len(
                {r.get("fingerprint") for r in results if r["score"] >= 0}
            )
        valid = [
            (m, r)
            for m, r in zip(candidates, results)
            if r["score"] >= best["score"] and not r["unknown"]
        ]
        if not valid:
            break
        valid.sort(key=lambda pair: (-pair[1]["score"], len(pair[0][2])))
        winner, measurement = valid[0]
        improved = measurement["score"] > best["score"] + 1e-9
        print(
            f'{addr:#x} round={rnd+1} candidates={len(candidates)} score={measurement["score"]*100:.2f} {winner[0]} {"improved" if improved else "neutral"}',
            flush=True,
        )
        if improved:
            with cache_lock:
                family[winner[0]] += 1
            bestbody = winner[2]
            best = measurement
            frontier = [bestbody]
            neutral = 0
            log.append(
                {"round": rnd + 1, "mutation": winner[:2], "score": best["score"]}
            )
            if best["exact"]:
                break
        elif neutral < max_neutral:
            neutral += 1
            frontier = []
            types = set()
            fingerprints = set()
            for mutation, measurement in valid:
                if mutation[0] in types or measurement["fingerprint"] in fingerprints:
                    continue
                types.add(mutation[0])
                fingerprints.add(measurement["fingerprint"])
                frontier.append(mutation[2])
                if len(frontier) == 3:
                    break
            # Equal code can still have a useful alternative source representation.
            if not frontier:
                frontier = [valid[0][0][2]]
        else:
            break
    if best["score"] > base + 1e-9:
        with source_lock:
            now = path.read_text(encoding="latin1")
            i, j = P.func_region(now, addr)
            if now[i:j] != initial:
                raise RuntimeError("Function changed while searching")
            path.write_text(now[:i] + bestbody + now[j:], encoding="latin1")
    return {
        "address": hex(addr),
        "file": path.name,
        "name": names[addr],
        "before": base,
        "after": best["score"],
        "exact": best["exact"],
        "variants": len(seen) - 1,
        "path": log,
    }


def main():
    global P, root, out, names, sizes, sources, identity, cache, statistics, family, progress, cachepath, source_lock, cache_lock, start, enabled_kinds
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=pathlib.Path, required=True)
    parser.add_argument(
        "--mutator", type=pathlib.Path, default=ROOT / "scripts/permute_mutators.py"
    )
    parser.add_argument("--min-score", type=float, default=0.88)
    parser.add_argument(
        "--max-score",
        type=float,
        default=1.0,
        help="Exclusive upper bound; use 0.88 to search the next score band.",
    )
    parser.add_argument("--max-bytes", type=int, default=1000)
    parser.add_argument("--min-bytes", type=int, default=1)
    parser.add_argument("--limit", type=int, default=140)
    parser.add_argument(
        "--addresses",
        help="Only search these comma-separated original addresses, e.g. 0x46b440,0x48ce80.",
    )
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--functions", type=int, default=4)
    parser.add_argument("--rounds", type=int, default=5)
    parser.add_argument("--packed", action="store_true", help="Test one variant per function together in each translation-unit compilation.")
    parser.add_argument(
        "--neutral-rounds", type=int, default=0,
        help="Explore equal-score representations for this many rounds before stopping.",
    )
    parser.add_argument(
        "--mutation-kinds",
        help="Comma-separated families to search, e.g. ifswap-multiline; default: all.",
    )
    parser.add_argument("--resume", action="store_true")
    args = parser.parse_args()
    enabled_kinds = set(args.mutation_kinds.split(",")) if args.mutation_kinds else set()
    known_kinds = {"idiom", "move", "swap", "flip", "ifswap", "obo", "unnest", "init", "loop-init", "selector", "ifswap-multiline", "local-layout"}
    if enabled_kinds - known_kinds:
        parser.error("Unknown mutation families: " + str(enabled_kinds - known_kinds))
    if (
        not 0 <= args.min_score < args.max_score <= 1
        or min(args.jobs, args.functions, args.rounds, args.limit, args.min_bytes, args.max_bytes) < 1
        or args.min_bytes > args.max_bytes
        or args.neutral_rounds < 0
    ):
        parser.error(
            "Scores must be in [0,1]; sizes, limits and worker counts must be positive."
        )
    if args.packed and args.neutral_rounds:
        parser.error("Packed search currently requires --neutral-rounds 0.")
    if args.packed and args.resume:
        parser.error("Packed search requires a fresh snapshot; --resume is supported by per-function search.")
    out = args.output.resolve()
    if out == ROOT or ROOT in out.parents:
        parser.error("Use an output directory outside the repository.")
    if not args.mutator.is_file():
        parser.error("Missing mutator: " + str(args.mutator))
    manifest = json.loads((ROOT / "build/manifest.json").read_text())
    if manifest.get("windowed"):
        parser.error("Use a matching build without --windowed.")
    current_sources = {
        str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest()
        for path in sorted((ROOT / "CMR2Decomp").rglob("*"))
        if path.suffix in (".cpp", ".h")
    }
    if current_sources != manifest["source_sha256"]:
        parser.error("Sources changed since the build; rebuild and measure before searching.")
    compiler = pathlib.Path(F.MSVC) / "Bin/CL.EXE"
    if (
        not compiler.is_file()
        or hashlib.sha256(compiler.read_bytes()).hexdigest()
        != manifest["compiler_sha256"]
    ):
        parser.error("CMR2_MSVC_ROOT must point to the compiler used for this build.")
    F.QIFIST = set(manifest["qifist_files"])
    F.CRT_O1 = set(manifest.get("o1_files", []))
    for suffix in ("exe", "pdb"):
        actual = hashlib.sha256(
            (ROOT / ("build/CMR2." + suffix)).read_bytes()
        ).hexdigest()
        if actual != manifest[suffix + "_sha256"]:
            parser.error("Build artifacts do not match the build manifest.")
    report = json.loads((ROOT / "CMR2PROGRESS/summary.json").read_text())
    entities = json.loads((ROOT / "CMR2PROGRESS/entities.json").read_text())
    audit = json.loads((ROOT / "CMR2PROGRESS/bytes.json").read_text())
    provenance = json.loads((ROOT / "CMR2PROGRESS/provenance.json").read_text())
    if provenance["build"] != manifest:
        parser.error("The reports belong to a different build; run scripts/measure.py.")
    root = out / "search-root"
    meta = out / "meta"
    context = [
        manifest,
        report,
        entities,
        hashlib.sha256(args.mutator.read_bytes()).hexdigest(),
        hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest(),
        hashlib.sha256(pathlib.Path(F.__file__).read_bytes()).hexdigest(),
        os.environ.get("FASTCMP_EXTRA", ""),
        os.environ.get("FASTCMP_SYMS", ""),
        os.environ.get("FASTCMP_QIFIST_ALL", ""),
    ]
    identity = digest(json.dumps(context, sort_keys=True))
    if args.resume:
        if (
            not (out / "identity.txt").is_file()
            or (out / "identity.txt").read_text() != identity
        ):
            parser.error(
                "Resume requires the same build, reports, compiler options and driver."
            )
    elif out.exists() and any(out.iterdir()):
        parser.error("Output directory is not empty; use --resume or a new directory.")
    else:
        out.mkdir(parents=True, exist_ok=True)
        meta.mkdir()
        shutil.copytree(ROOT / "CMR2Decomp", out / "original-source")
        shutil.copytree(ROOT / "CMR2Decomp", root / "CMR2Decomp")
        (root / "build").mkdir()
        for file in (ROOT / "build").iterdir():
            if file.suffix == ".obj" or file.name == "CMR2.exe":
                shutil.copy2(file, root / "build" / file.name)
        for directory in ("third_party", "cmr2bin"):
            (root / directory).symlink_to(ROOT / directory, target_is_directory=True)
        for entry in report["data"]:
            if entry["address"] in audit:
                entry["name"] = audit[entry["address"]]["symbol_name"]
        (meta / "cur.json").write_text(json.dumps(report))
        (meta / "ent.json").write_text(
            json.dumps(
                [{"o": int(a, 16), "r": v[0], "n": v[1]} for a, v in entities.items()]
            )
        )
        (out / "identity.txt").write_text(identity)
    F.REPO = str(root)
    F.HERE = str(meta)
    F._NM = None
    F.ORIG = None
    names, sizes = F.load_meta()
    sources = {int(a, 16): v["f"] for a, v in audit.items()}
    spec = importlib.util.spec_from_file_location("cmr2_mutators", args.mutator)
    P = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(P)
    P.SAFE = True
    P.negate = lambda condition: "!(" + condition + ")"
    P.func_region = func_region
    only = None
    if args.addresses:
        try:
            only = parse_addresses(args.addresses)
        except ValueError as error:
            parser.error(str(error))
    targets = [
        (a, v["s"], sizes.get(a, 0), v["f"])
        for address, v in audit.items()
        if (a := int(address, 16)) in names
        and (only is None or a in only)
        and not v["x"]
        and not v.get("unknown")
        and not v.get("err")
        and args.min_score <= v["s"] < args.max_score
        and args.min_bytes <= sizes.get(a, 0) <= args.max_bytes
    ]
    targets.sort(key=lambda row: (-row[1], row[2]))
    if enabled_kinds == {"ifswap-multiline"}:
        filtered = []
        for row in targets:
            text = (root / "CMR2Decomp" / row[3]).read_text(encoding="latin1")
            try:
                a, b = func_region(text, row[0])
            except ValueError:
                continue
            if multiline_ifswaps(text[a:b]):
                filtered.append(row)
        targets = filtered
    targets = targets[: args.limit]
    cachepath = out / "candidate-cache.json"
    cache = json.loads(cachepath.read_text()) if cachepath.exists() else {}
    statistics = collections.Counter()
    family = collections.Counter()
    progress = []
    source_lock = threading.Lock()
    cache_lock = threading.Lock()
    start = time.monotonic()
    for filename in {target[3] for target in targets}:
        F.symmap_for(str(root / "CMR2Decomp" / filename))
    if args.packed:
        search_packed(targets, args.jobs, args.rounds)
    else:
        with concurrent.futures.ThreadPoolExecutor(
            max_workers=args.jobs
        ) as compilers, concurrent.futures.ThreadPoolExecutor(
            max_workers=args.functions
        ) as functions:
            pending = [
                functions.submit(search, a, compilers, args.rounds, args.neutral_rounds)
                for a, _, _, _ in targets
            ]
            for future in pending:
                result = future.result()
                progress.append(result)
                checkpoint()
                print("RESULT", json.dumps(result), flush=True)
    patch = []
    for file in sorted((root / "CMR2Decomp").glob("*.cpp")):
        old = (out / "original-source" / file.name).read_text(encoding="latin1")
        new = file.read_text(encoding="latin1")
        if old != new:
            patch.append(
                "".join(
                    difflib.unified_diff(
                        old.splitlines(True),
                        new.splitlines(True),
                        "a/CMR2Decomp/" + file.name,
                        "b/CMR2Decomp/" + file.name,
                    )
                )
            )
    (out / "changes.patch").write_text("".join(patch), encoding="latin1")
    print(
        "TOTAL",
        dict(statistics),
        "improved",
        sum(v["after"] > v["before"] for v in progress),
        "exact",
        sum(v.get("exact", False) for v in progress),
        "seconds",
        time.monotonic() - start,
        flush=True,
    )
    print("Review patch:", out / "changes.patch", flush=True)


if __name__ == "__main__":
    main()
