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
