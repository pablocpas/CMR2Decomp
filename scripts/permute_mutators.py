"""Source mutators for scripts/permute_batch.py.

Each generator takes the function body as a list of lines and returns
(kind, detail, new_lines) tuples. new_lines has the same length as lines (one
element may hold several physical lines) so the driver can pair old and new
lines for its safety filters. permute_batch.py rejects every candidate that
could change behaviour (calls, memory stores, floating point reordering); the
generators here only produce syntactically valid, locally equivalent forms.

  move     swap two adjacent independent simple statements
  ifswap   if (c) {A} else {B}  ->  if (negate(c)) {B} else {A}
  swap     a OP b  ->  b OP a   for == != + * & | ^
  flip     a < b   ->  b > a    (and the other relational operators)
  obo      x < 10  ->  x <= 9   (integer constants only)
  unnest   v = f(g(x));  ->  v = g(x); v = f(v);
"""
import os
import re

SAFE = True
_ID = re.compile(r"\b[A-Za-z_]\w*\b")
KEYWORDS = {"if", "while", "for", "switch", "return", "sizeof", "else", "do", "case",
            "goto", "break", "continue", "default", "new", "delete"}
_CALL = re.compile(r"\b([A-Za-z_]\w*(?:::\w+)*)\s*\(")
_ATOM = r"(?:[A-Za-z_]\w*(?:(?:->|\.)[A-Za-z_]\w*|\[[^\[\]]+\])*|0x[0-9a-fA-F]+|\d+)"
_ASSIGN = re.compile(r"^\s*(?!(?:return|if|while|for|switch|case)\b)((?:->|[^=;{}()!<>])+?)\s*(?:[-+*/%&|^]|<<|>>)?=(?!=)")
_DECL = re.compile(r"^\s*(?:(?:unsigned|signed|const|static|struct)\s+)*[A-Za-z_]\w*(?:\s*\*)*\s+\**\s*[A-Za-z_]\w*\s*(?:\[[^\]]*\])?\s*(?:=|;)")


def negate(condition):
    return "!(" + condition + ")"


def func_region(text, address):  # replaced by permute_batch.func_region
    raise NotImplementedError


def _calls(text):
    return [m.group(0) for m in _CALL.finditer(text) if m.group(1) not in KEYWORDS]


def _lhs(line):
    if _DECL.match(line) and "=" not in line.split(";")[0]:
        return None
    m = _ASSIGN.match(line)
    if not m:
        return None
    lhs = m.group(1)
    # "int x = ..." declares x: the assigned name is the last identifier.
    decl = re.match(r"^\s*(?:(?:unsigned|signed|const)\s+)*[A-Za-z_]\w*(?:\s*\*)*\s+\**\s*([A-Za-z_]\w*)\s*$", lhs)
    return decl.group(1) if decl else lhs.strip()


def _mem(lhs):
    return bool(lhs) and bool(re.search(r"->|\.|\[|^\s*\*", lhs))


def _simple(line):
    s = line.strip()
    return (s.endswith(";") and "{" not in s and "}" not in s
            and not re.match(r"(return|break|continue|goto|case|default)\b", s)
            and not s.startswith(("//", "/*", "#")))


def _indent(line):
    return len(line) - len(line.lstrip())


def _bounded(line, m):
    """The match is a whole operand: nothing binds tighter on either side."""
    before = line[:m.start()].rstrip()
    after = line[m.end():].lstrip()
    left_ok = (before == "" or before.endswith(("(", ",", "&&", "||", "?", ":", "return", "{"))
               or (before.endswith("=") and not before.endswith(("==", "!=", "<=", ">="))))
    right_ok = after.startswith((")", ";", ",", "&&", "||", "?", ":"))
    return left_ok and right_ok and not before.endswith(("(*", "(&"))


def moves(lines):
    out = []
    for i in range(len(lines) - 1):
        a, b = lines[i], lines[i + 1]
        if not (_simple(a) and _simple(b)) or _indent(a) != _indent(b):
            continue
        la, lb = _lhs(a), _lhs(b)
        if la is None or lb is None:
            continue
        ida, idb = set(_ID.findall(a)), set(_ID.findall(b))
        wa, wb = set(_ID.findall(la)), set(_ID.findall(lb))
        # Neither statement may read or write what the other writes.
        if wa & idb or wb & ida:
            continue
        if re.search(r"\+\+|--", a + b):
            continue
        new = list(lines)
        new[i], new[i + 1] = b, a
        out.append(("move", f"{i}", new))
    return out


def _block_end(lines, start):
    """Index of the line holding the '}' that closes the block opened on lines[start]."""
    depth = re.sub(r"//.*", "", lines[start]).count("{")
    if depth != 1:
        return None
    for j in range(start + 1, len(lines)):
        code = re.sub(r"//.*", "", lines[j])
        for ch in code:
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return j
    return None


def ifswaps(lines):
    out = []
    for i, line in enumerate(lines):
        m = re.match(r"^(\s*)(\}\s*else\s+)?if \((.*)\) \{\s*$", line)
        if not m or m.group(2):
            continue
        end = _block_end(lines, i)
        if end is None:
            continue
        e = re.match(r"^(\s*)\} else \{\s*$", lines[end])
        if not e or e.group(1) != m.group(1):
            continue
        end2 = _block_end(lines, end)
        if end2 is None or lines[end2].strip() != "}":
            continue
        then_body, else_body = lines[i + 1:end], lines[end + 1:end2]
        block = [f"{m.group(1)}if ({negate(m.group(3))}) {{"] + else_body + \
                [lines[end]] + then_body + [lines[end2]]
        # One element holds the whole block; the merged lines become empty
        # so old and new lines stay aligned for the driver's filters.
        new = lines[:i] + ["\n".join(block)] + [""] * (end2 - i) + lines[end2 + 1:]
        out.append(("ifswap", f"{i}", new))
    return out


def swaps(lines):
    out = []
    pat = re.compile(rf"(?<![\w\]>.])({_ATOM})\s*(==|!=|\+|\*|&|\||\^)\s*({_ATOM})(?![\w\[(.]|->)")
    for i, line in enumerate(lines):
        if line.lstrip().startswith(("//", "#")):
            continue
        for m in pat.finditer(line):
            a, op, b = m.groups()
            if a == b:
                continue
            if not _bounded(line, m):
                continue
            new = list(lines)
            new[i] = line[:m.start()] + f"{b} {op} {a}" + line[m.end():]
            out.append(("swap", f"{i}:{m.start()}:{op}", new))
    return out


FLIP = {"<": ">", ">": "<", "<=": ">=", ">=": "<="}


def flips(lines):
    out = []
    pat = re.compile(rf"(?<![\w\]>.])({_ATOM})\s*(<=|>=|<|>)\s*({_ATOM})(?![\w\[(.]|->)")
    for i, line in enumerate(lines):
        if line.lstrip().startswith(("//", "#")) or "<<" in line or ">>" in line:
            continue
        for m in pat.finditer(line):
            a, op, b = m.groups()
            if not _bounded(line, m):
                continue
            new = list(lines)
            new[i] = line[:m.start()] + f"{b} {FLIP[op]} {a}" + line[m.end():]
            out.append(("flip", f"{i}:{m.start()}:{op}", new))
    return out


def obos(lines):
    out = []
    pat = re.compile(rf"(?<![\w\]>.])({_ATOM})\s*(<=|>=|<|>)\s*(-?(?:0x[0-9a-fA-F]+|\d+))(?![\w.])")
    for i, line in enumerate(lines):
        if line.lstrip().startswith(("//", "#")) or "<<" in line or ">>" in line:
            continue
        for m in pat.finditer(line):
            a, op, n = m.groups()
            if not _bounded(line, m):
                continue
            v = int(n, 0)
            nv, nop = {"<": (v - 1, "<="), "<=": (v + 1, "<"),
                       ">": (v + 1, ">="), ">=": (v - 1, ">")}[op]
            text = hex(nv) if n.lower().startswith("0x") and nv >= 0 else str(nv)
            new = list(lines)
            new[i] = line[:m.start()] + f"{a} {nop} {text}" + line[m.end():]
            out.append(("obo", f"{i}:{m.start()}:{op}", new))
    return out


def unnests(lines):
    out = []
    for i, line in enumerate(lines):
        m = re.match(r"^(\s*)([A-Za-z_]\w*) = ([A-Za-z_]\w*)\(([A-Za-z_]\w*\([^()]*\))(.*)\);\s*$", line)
        if not m:
            continue
        ind, var, outer, inner, rest = m.groups()
        if var in _ID.findall(inner):
            continue
        new = list(lines)
        new[i] = f"{ind}{var} = {inner};\n{ind}{var} = {outer}({var}{rest});"
        out.append(("unnest", f"{i}", new))
    return out


_swaps = swaps


def swaps(lines):  # noqa: F811  (operand swaps and relational flips share the filter)
    return _swaps(lines) + flips(lines)


# ---------------------------------------------------------------- idioms
# Rewrites that keep the generated instructions but change how MSVC6 numbers
# its temporaries, which decides register allocation ties. Each candidate
# changes one occurrence, plus one candidate that changes all of them.

def _call_args(text, open_paren):
    """(end index after ')', [top-level argument strings]) for text[open_paren] == '('."""
    depth, start, args = 0, open_paren + 1, []
    for k in range(open_paren, len(text)):
        ch = text[k]
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                args.append(text[start:k])
                return k + 1, args
        elif ch == "," and depth == 1:
            args.append(text[start:k])
            start = k + 1
    return None, None


def _rewrite_calls(body, name, build):
    """Candidates rewriting calls of `name` with build(args) -> text or None."""
    spots = []
    for m in re.finditer(r"\b" + name + r"\s*\(", body):
        end, args = _call_args(body, m.end() - 1)
        if end is None:
            continue
        new = build([a.strip() for a in args])
        if new is not None:
            spots.append((m.start(), end, new))
    return spots


def _apply(body, spots):
    for a, b, new in sorted(spots, reverse=True):
        body = body[:a] + new + body[b:]
    return body


IDIOMS = {
    # (a * b) >> 32 through the asm helper, or FixMul and a C shift
    "fixmulshift32": lambda body: _rewrite_calls(
        body, "FixMulShift32",
        lambda a: f"(FixMul({a[0]}, {a[1]}) >> 16)" if len(a) == 2 else None),
}


def _shift_spots(body):
    spots = []
    for m in re.finditer(r"\bFixMul\s*\(", body):
        end, args = _call_args(body, m.end() - 1)
        if end is None or len(args) != 2:
            continue
        tail = re.match(r"\s*>>\s*16\b", body[end:])
        before = body[:m.start()].rstrip()
        # "a + FixMul(..) >> 16" shifts the sum: only rewrite a whole shift operand.
        if before.endswith(("+", "-", "*", "/", "%")) and not before.endswith(("++", "--")):
            continue
        if tail and not re.match(r"\s*[-+*/%]", body[end + tail.end():]):
            spots.append((m.start(), end + tail.end(),
                          f"FixMulShift32({args[0].strip()}, {args[1].strip()})"))
    return spots


IDIOMS["fixmul>>16"] = _shift_spots


def _in_directive(body, pos):
    """pos lies in a #define (or other directive), including continuation lines."""
    start = body.rfind("\n", 0, pos) + 1
    while True:
        if body[start:].lstrip(" \t").startswith("#"):
            return True
        if start == 0:
            return False
        prev = body.rfind("\n", 0, start - 1) + 1
        if not body[prev:start - 1].rstrip().endswith("\\"):
            return False
        start = prev


def idioms(lines):
    body = "\n".join(lines)
    out = []
    only = set(filter(None, os.environ.get("PERMUTE_IDIOMS", "").split(",")))
    for kind, finder in IDIOMS.items():
        if only and kind not in only:
            continue
        spots = [sp for sp in finder(body) if not _in_directive(body, sp[0])]
        # "all" rewrites only non-overlapping spots: nested ones would be
        # applied with stale offsets.
        disjoint = []
        for sp in sorted(spots):
            if not disjoint or sp[0] >= disjoint[-1][1]:
                disjoint.append(sp)
        variants = [[s] for s in spots] + ([disjoint] if len(disjoint) > 1 else [])
        for i, chosen in enumerate(variants):
            new = _apply(body, chosen)
            if new != body:
                out.append(("idiom", f"{kind}:{i}", new.split("\n")))
    return out


PURE_CALLS = {"FixMul", "FixMulShift32", "FixDiv", "FixVecDot", "FixVecLength", "FixSqrt", "sizeof"}


def _has_call(text):
    return any(m.group(1) not in PURE_CALLS for m in re.finditer(r"\b([A-Za-z_]\w*)\s*\(", text))


def _swap_args(name):
    # Swapping changes evaluation order, so only for arguments without calls.
    return lambda body: _rewrite_calls(
        body, name, lambda a: f"{name}({a[1]}, {a[0]})"
        if len(a) == 2 and a[0] != a[1] and not _has_call(a[0]) and not _has_call(a[1]) else None)


IDIOMS["fixmul-swap"] = _swap_args("FixMul")
IDIOMS["fixmulshift32-swap"] = _swap_args("FixMulShift32")

_OPERAND = r"[A-Za-z_][\w]*(?:(?:->|\.)[A-Za-z_]\w*|\[[^\[\]()]+\])*"


def _cond_operand(cond, start, end):
    """cond[start:end] is used only as a truth value: after stripping grouping
    parentheses around it, it is the whole condition or an operand of &&, ||
    or !. Anything else ((t) >= k, (x) + 1, a ? b : c, call arguments) is not."""
    while True:
        pre = cond[:start].rstrip()
        post = cond[end:].lstrip()
        if pre.endswith("(") and post.startswith(")") and not re.search(r"[\w\])]\s*\($", pre):
            start, end = len(pre) - 1, len(cond) - len(post) + 1
            continue
        break
    if pre == "" and post == "":
        return True
    left = pre.endswith(("&&", "||")) or (pre.endswith("!") and not pre.endswith("!="))
    right = post.startswith(("&&", "||"))
    if left and (post == "" or post.startswith((")", "&&", "||", "?"))):
        return not post.startswith("?") or pre.endswith(("&&", "||"))
    if right and (pre == "" or pre.endswith(("(", "&&", "||"))):
        return not re.search(r"[\w\])]\s*\($", pre)
    return False


def _condition_spots(body):
    """Inside if/while conditions: x != 0 <-> x, x == 0 <-> !x (whole operands only)."""
    spots = []
    for m in re.finditer(r"\b(?:if|while)\s*\(", body):
        end, args = _call_args(body, m.end() - 1)
        if end is None or len(args) != 1:
            continue
        start = m.end()
        cond = args[0]
        for c in re.finditer(rf"(?<![\w>.\]!])({_OPERAND})\s*(!=|==)\s*(?:0|NULL)\b(?!\s*[\w(.\[])", cond):
            if _cond_operand(cond, c.start(), c.end()):
                new = c.group(1) if c.group(2) == "!=" else "!" + c.group(1)
                spots.append((start + c.start(), start + c.end(), new))
        for c in re.finditer(rf"(?<![\w>.\]])(!?)({_OPERAND})(?![\w(.\[]|->|\s*[=!<>+\-*/%&|^?])", cond):
            if _cond_operand(cond, c.start(), c.end()):
                new = f"{c.group(2)} == 0" if c.group(1) else f"{c.group(2)} != 0"
                spots.append((start + c.start(), start + c.end(), new))
    return spots


IDIOMS["condition"] = _condition_spots


# Vector helpers spelled out per component. FixVecScale/FixVecDot compute the
# same bits as three FixMul calls (shld/shrd of the same product), and the
# compiler rejects the helper call when the operands are not FixVectors.
_VEC = r"[A-Za-z_]\w*(?:(?:->|\.)[A-Za-z_]\w*|\[[^\[\]]+\])*?"


def _vec_ref(base, sep):
    return base if sep == "->" else "&" + base


def _vecscale_spots(body):
    spots = []
    line = (r"(?P<ind>[ \t]*)(?P<d>{v})(?P<ds>->|\.)(?P<c>x|y|z) = FixMul\((?:(?P<s>{v})(?P<ss>->|\.)(?P=c), (?P<t>[^;\n]+?)|(?P<t2>[^;\n]+?), (?P<s2>{v})(?P<ss2>->|\.)(?P=c))\);[ \t]*\n").format(v=_VEC)
    pat = re.compile(line)
    for m1 in re.finditer(r"(?m)^", body):
        m1 = pat.match(body, m1.start())
        if not m1:
            continue
        if m1.group("c") != "x":
            continue
        m2 = pat.match(body, m1.end())
        m3 = m2 and pat.match(body, m2.end())
        if not (m2 and m3) or (m2.group("c"), m3.group("c")) != ("y", "z"):
            continue
        keys = []
        for m in (m1, m2, m3):
            src, ss, t = (m.group("s"), m.group("ss"), m.group("t")) if m.group("s") else \
                         (m.group("s2"), m.group("ss2"), m.group("t2"))
            keys.append((m.group("d"), m.group("ds"), src, ss, t.strip()))
        if len(set(keys)) != 1:
            continue
        d, ds, src, ss, t = keys[0]
        root = re.match(r"[A-Za-z_]\w*", d).group(0)
        if re.search(r"\b" + root + r"\b", t):
            continue
        new = f"{m1.group('ind')}FixVecScale({_vec_ref(d, ds)}, {_vec_ref(src, ss)}, {t});\n\n\n"
        spots.append((m1.start(), m3.end(), new))
    return spots


def _vecdot_spots(body):
    term = r"FixMul\(\s*(?P<a{n}>{v})(?P<as{n}>->|\.){c}\s*,\s*(?P<b{n}>{v})(?P<bs{n}>->|\.){c}\s*\)"
    pat = re.compile(r"\s*\+\s*".join(term.format(n=i, v=_VEC, c=c) for i, c in enumerate("xyz")))
    spots = []
    for m in pat.finditer(body):
        a = {(m.group(f"a{i}"), m.group(f"as{i}")) for i in range(3)}
        b = {(m.group(f"b{i}"), m.group(f"bs{i}")) for i in range(3)}
        if len(a) != 1 or len(b) != 1:
            continue
        (av, asep), (bv, bsep) = a.pop(), b.pop()
        before = body[:m.start()].rstrip()
        after = body[m.end():].lstrip()
        if before.endswith(("*", "/", "%", "-", "<<", ">>")) or after.startswith(("*", "/", "%", "[", "<<", ">>")):
            continue
        spots.append((m.start(), m.end(), f"FixVecDot({_vec_ref(av, asep)}, {_vec_ref(bv, bsep)})"))
    return spots


IDIOMS["vecscale"] = _vecscale_spots
IDIOMS["vecdot"] = _vecdot_spots

# a . b == b . a; the inline asm loads a first, so the order shows in the code.
IDIOMS["vecdot-swap"] = _swap_args("FixVecDot")
