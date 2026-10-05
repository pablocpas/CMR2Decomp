#!/usr/bin/env python3
"""Plan and apply batches of descriptive names to byte-exact functions.

export writes an editable TSV; fill new_name and evidence, leaving other rows
blank. show displays a body and its references. preview emits a unified diff.
apply rebuilds, measures, checks object code and runs the differential suite;
on failure it restores edited files, build artifacts and measurement reports.
No names are inferred automatically. Only the relocated byte audit is used.
"""
import argparse
from collections import Counter
import csv
from dataclasses import dataclass
import difflib
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
MARKER = re.compile(r"//\s*FUNCTION:\s*CMR2\s+(0x[\da-fA-F]+)")
QUALIFIED = r"[A-Za-z_]\w*(?:::[A-Za-z_~]\w*)*"
NAME = re.compile(r"[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*\Z", re.ASCII)
WORDS = re.compile(r"[A-Za-z_]\w*", re.ASCII)
NONCODE = re.compile(
    r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\''
)
KEYWORDS = set("""asm auto bool break case catch char class const const_cast
continue default delete do double dynamic_cast else enum explicit export extern
false float for friend goto if inline int long mutable namespace new operator
private protected public register reinterpret_cast return short signed sizeof
static static_cast struct switch template this throw true try typedef typeid
typename union unsigned using virtual void volatile wchar_t while and and_eq
bitand bitor compl not not_eq or or_eq xor xor_eq""".split())
COLUMNS = ["address", "old_name", "new_name", "evidence", "file", "line",
           "signature", "comment"]


def read_text(path):
    # Preserve line endings and any legacy non-UTF-8 bytes exactly.
    return path.read_bytes().decode("utf-8", errors="surrogateescape")


def encode(text):
    return text.encode("utf-8", errors="surrogateescape")


def code_only(text):
    return NONCODE.sub(lambda m: re.sub(r"[^\r\n]", " ", m[0]), text)


def source_paths(root):
    return sorted(p for p in (root / "CMR2Decomp").rglob("*")
                  if p.suffix in (".cpp", ".h"))


def body_end(code, begin, limit):
    depth = 0
    branches = []
    tokens = re.compile(r"[{}]|^[ \t]*#[ \t]*(if|ifdef|ifndef|elif|else|endif)\b[^\n]*", re.MULTILINE)
    for token in tokens.finditer(code, begin, limit):
        directive = token[1]
        if directive in ("if", "ifdef", "ifndef"):
            branches.append(depth)
        elif directive in ("else", "elif"):
            if not branches:
                raise ValueError("Unbalanced preprocessor branches in function")
            # Alternative branches can each open the same shared block (zlib).
            depth = branches[-1]
        elif directive == "endif":
            if not branches:
                raise ValueError("Unbalanced preprocessor branches in function")
            branches.pop()
        elif token[0] == "{":
            depth += 1
        elif token[0] == "}":
            depth -= 1
            if depth == 0:
                return token.end()
    raise ValueError("Unterminated function body")


@dataclass
class Function:
    address: int
    name: str
    path: Path
    line: int
    signature: str
    comment: str
    begin: int
    end: int


def functions(root):
    result = {}
    for path in source_paths(root):
        if path.suffix != ".cpp":
            continue
        text = read_text(path)
        code = code_only(text)
        markers = list(MARKER.finditer(text))
        for i, marker in enumerate(markers):
            address = int(marker[1], 16)
            if address in result:
                raise ValueError(f"Duplicate FUNCTION annotation: {address:#x}")
            limit = markers[i + 1].start() if i + 1 < len(markers) else len(code)
            begin = code.find("{", marker.end(), limit)
            if begin < 0:
                raise ValueError(f"Missing function body: {address:#x}")
            names = re.findall(r"(" + QUALIFIED + r")\s*\(", code[marker.end():begin])
            if not names:
                raise ValueError(f"Missing function name: {address:#x}")
            # Parameters can contain function-pointer types; their names are
            # not the annotated function. Ignore only declaration attributes.
            name = next(n for n in names if n not in ("__declspec", "__attribute__"))
            end = body_end(code, begin, limit)
            comments = []
            for line in reversed(text[:marker.start()].splitlines()):
                if not line.strip().startswith("//"):
                    break
                if not re.match(r"//\s*(match|FUNCTION|GLOBAL)\b", line.strip()):
                    comments.insert(0, line.strip()[2:].strip())
            result[address] = Function(
                address, name, path, text.count("\n", 0, marker.start()) + 1,
                " ".join(text[marker.end():begin].split()), " ".join(comments),
                marker.start(), end,
            )
    return result


def load_audit(root):
    return {int(a, 16): v for a, v in
            json.loads((root / "CMR2PROGRESS/bytes.json").read_text()).items()}


def require_fresh(root):
    manifest = json.loads((root / "build/manifest.json").read_text())
    provenance = json.loads((root / "CMR2PROGRESS/provenance.json").read_text())
    current = {str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest()
               for p in source_paths(root)}
    if manifest["windowed"] or provenance["build"] != manifest:
        raise ValueError("Build and measure the default matching executable first.")
    if current != manifest["source_sha256"]:
        raise ValueError("Sources changed since measurement; rebuild and measure first.")
    for suffix in ("exe", "pdb"):
        path = root / ("build/CMR2." + suffix)
        if hashlib.sha256(path.read_bytes()).hexdigest() != manifest[suffix + "_sha256"]:
            raise ValueError(f"Build artifact changed since measurement: {path}")
    original = root / "cmr2bin/CMR2.exe"
    if hashlib.sha256(original.read_bytes()).hexdigest() != provenance["original_sha256"]:
        raise ValueError("Original executable changed since measurement.")


def export_map(root, output, file_filter=None, include_named=False):
    require_fresh(root)
    audit = load_audit(root)
    stream = io.StringIO(newline="")
    writer = csv.DictWriter(stream, fieldnames=COLUMNS, delimiter="\t", lineterminator="\n")
    writer.writeheader()
    count = 0
    for address, fn in sorted(functions(root).items()):
        if not audit.get(address, {}).get("x"):
            continue
        if file_filter and fn.path.name not in file_filter:
            continue
        if not include_named and not re.search(r"FUN_[\da-fA-F]{8}", fn.name):
            continue
        writer.writerow(dict(address=f"0x{address:08x}", old_name=fn.name,
                             new_name="", evidence="", file=str(fn.path.relative_to(root)),
                             line=fn.line, signature=fn.signature, comment=fn.comment))
        count += 1
    if output is None:
        sys.stdout.write(stream.getvalue())
    else:
        # Never overwrite a map that may already contain reviewed names.
        with output.open("xb") as handle:
            handle.write(encode(stream.getvalue()))
        print(f"{count} byte-exact functions exported to {output}")


def read_map(path):
    with path.open(encoding="utf-8", errors="surrogateescape", newline="") as handle:
        reader = csv.DictReader(handle, delimiter="\t")
        if not {"address", "old_name", "new_name"}.issubset(reader.fieldnames or []):
            raise ValueError("Map requires address, old_name and new_name TSV columns.")
        rows = []
        for line, row in enumerate(reader, 2):
            if None in row:
                raise ValueError(f"Too many columns on map line {line}")
            if not (row.get("new_name") or "").strip():
                continue
            row = {k: (v or "").strip() for k, v in row.items()}
            try:
                row["address"] = int(row["address"], 16)
            except ValueError as error:
                raise ValueError(f"Invalid address on map line {line}") from error
            rows.append(row)
    if not rows:
        raise ValueError("No new_name entries filled in; nothing to rename.")
    return rows


def replace_cpp(text, renames):
    def replace(segment):
        return WORDS.sub(lambda m: renames.get(m[0], m[0]), segment)
    output = []
    start = 0
    for token in NONCODE.finditer(text):
        output.append(replace(text[start:token.start()]))
        # Update explanatory comments but preserve runtime string/char literals.
        output.append(replace(token[0]) if token[0].startswith("/") else token[0])
        start = token.end()
    output.append(replace(text[start:]))
    return "".join(output)


def make_plan(root, rows):
    known = functions(root)
    audit = load_audit(root)
    basenames = Counter(fn.name.rsplit("::", 1)[-1] for fn in known.values())
    identifiers = set()
    for path in source_paths(root):
        identifiers.update(WORDS.findall(code_only(read_text(path))))
    changes = {}
    renames = {}
    addresses = set()
    used_names = set()
    for row in rows:
        address, old, new = row["address"], row["old_name"], row["new_name"]
        fn = known.get(address)
        if fn is None or old != fn.name:
            raise ValueError(f"Stale map at {address:#x}: current name is {fn.name if fn else 'missing'}")
        if not audit.get(address, {}).get("x"):
            raise ValueError(f"Function is not byte-exact: {address:#x}")
        if address in addresses:
            raise ValueError(f"Duplicate map address: {address:#x}")
        addresses.add(address)
        if not NAME.fullmatch(new):
            raise ValueError(f"Invalid C++ name: {new}")
        old_scope, _, leaf = old.rpartition("::")
        new_scope, _, target = new.rpartition("::")
        if new_scope and new_scope != old_scope:
            raise ValueError(f"Cannot move a function between scopes: {old} -> {new}")
        if target in KEYWORDS or target.startswith("_") or "__" in target:
            raise ValueError(f"Reserved C++ identifier: {target}")
        if old_scope and (leaf.startswith("~") or leaf == old_scope.rsplit("::", 1)[-1]):
            raise ValueError(f"Cannot rename a constructor or destructor: {old}")
        if basenames[leaf] != 1:
            raise ValueError(f"Ambiguous identifier {leaf}; scope-aware manual renaming required")
        if target in identifiers or target in used_names:
            raise ValueError(f"Name already in use: {target}")
        used_names.add(target)
        renames[leaf] = target
    for path in source_paths(root):
        old = read_text(path)
        new = replace_cpp(old, renames)
        if new != old:
            changes[path] = (encode(old), encode(new))
    # Match complete tokens in test symbol-name strings (including class names
    # and legacy Module_FUN_... free functions); never change address constants.
    pattern = re.compile(r"(?<!\w)(?:" + "|".join(map(re.escape, renames)) + r")(?!\w)")
    for path in sorted((root / "tests").rglob("*")):
        if path.suffix not in (".py", ".json"):
            continue
        # These are synthetic mini-repositories, not references to game symbols.
        if path == root / "tests/test_rename_functions.py":
            continue
        old = read_text(path)
        new = pattern.sub(lambda m: renames[m[0]], old)
        if new != old:
            changes[path] = (encode(old), encode(new))
    path = root / "scripts/functions.tsv"
    old = read_text(path)
    entries = {r["address"]: r for r in rows}
    lines = old.splitlines(keepends=True)
    for i in range(1, len(lines)):
        columns = lines[i].split("\t")
        row = entries.get(int(columns[0], 16))
        if row:
            # Keep the inventory's existing qualified/unqualified name style.
            columns[2] = pattern.sub(lambda m: renames[m[0]], columns[2])
            lines[i] = "\t".join(columns)
    new = "".join(lines)
    if new != old:
        changes[path] = (encode(old), encode(new))
    return changes, renames


def show_function(root, address):
    fn = functions(root).get(address)
    if fn is None:
        raise ValueError(f"No source function at {address:#x}")
    exact = load_audit(root).get(address, {}).get("x", False)
    print(f"{fn.path.relative_to(root)}:{fn.line} — {fn.name} — byte-exact: {exact}")
    if fn.comment:
        print("// " + fn.comment)
    print(read_text(fn.path)[fn.begin:fn.end])
    print("\nReferences:")
    pattern = re.compile(r"(?<!\w)" + re.escape(fn.name.rsplit("::", 1)[-1]) + r"(?!\w)")
    for path in source_paths(root):
        for line, text in enumerate(read_text(path).splitlines(), 1):
            if pattern.search(text):
                print(f"{path.relative_to(root)}:{line}: {text.strip()}")


def object_code(root, reverse=None):
    # Use the existing COFF reader. Symbol table indices and debug data may
    # change with names; normalize relocation targets, preserving their meaning.
    from fastcmp import COFF
    def symbol(sym):
        if sym is None:
            raise ValueError("Invalid COFF relocation target")
        name = sym["name"]
        for new, old in (reverse or {}).items():
            name = name.replace("?" + new + "@", "?" + old + "@")
            name = name.replace("_" + new + "@", "_" + old + "@")
            if name == "_" + new:
                name = "_" + old
        return (name, sym["val"], sym["sec"], sym["typ"], sym["cls"])
    result = {}
    for path in sorted((root / "build").glob("*.obj")):
        obj = COFF(str(path))
        result[path.name] = [
            (section["name"], section["data"],
             [(offset, kind, symbol(obj.syms[index]))
              for offset, index, kind in section["rels"]])
            for section in obj.secs if not section["name"].startswith(".debug")
        ]
    if not result:
        raise ValueError("No baseline build/*.obj files; rebuild before applying names.")
    return result


def check_audit(before, after):
    if set(before) != set(after):
        raise ValueError("The set of audited functions changed")
    lost = [hex(a) for a, v in before.items() if v["x"] and not after[a]["x"]]
    errors = [hex(a) for a, v in after.items() if v.get("err")]
    scores = [hex(a) for a, v in before.items() if after[a]["s"] + 1e-12 < v["s"]]
    if lost or errors or scores:
        raise ValueError(f"Matching regression: lost exact={lost}, errors={errors}, lower byte scores={scores}")


def generated_paths(root):
    return set((root / "build").glob("*.obj")) | {
        root / "build" / name for name in ("CMR2.exe", "CMR2.pdb", "manifest.json")
    } | {root / "vc60.pdb", root / "index.html",
         root / "scripts/work/cur.json", root / "scripts/work/ent.json"} | {
        root / "CMR2PROGRESS" / name for name in
        ("bytes.json", "summary.json", "entities.json", "provenance.json", "datacmp.log", "nonmatching.tsv")
    }


def atomic_write(path, data):
    if path.is_symlink():
        raise ValueError(f"Refusing to replace a symlink: {path}")
    mode = path.stat().st_mode & 0o777 if path.exists() else 0o644
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as handle:
        tmp = Path(handle.name)
        try:
            handle.write(data)
            handle.close()
            tmp.chmod(mode)
            tmp.replace(path)
        finally:
            tmp.unlink(missing_ok=True)


def run(root, script, *arguments):
    print(f"Running {script}...", flush=True)
    subprocess.run([sys.executable, str(root / script), *arguments], cwd=root, check=True)


def apply_plan(root, changes, renames, jobs):
    require_fresh(root)
    before_audit = load_audit(root)
    before_code = object_code(root)
    paths = set(changes) | generated_paths(root)
    # Snapshot only outputs the build/measurement commands own, not the large
    # unrelated experiments in build/ or scripts/work/.
    with tempfile.TemporaryDirectory(prefix="cmr2-rename-") as directory:
        backup = Path(directory)
        saved = set()
        for path in paths:
            if path.is_symlink():
                raise ValueError(f"Refusing to mutate a symlink: {path}")
            if path.exists():
                target = backup / path.relative_to(root)
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(path, target)
                saved.add(path)
        try:
            for path, (old, new) in changes.items():
                if path.read_bytes() != old:
                    raise ValueError(f"File changed after planning: {path}")
            for path, (_, new) in changes.items():
                atomic_write(path, new)
            run(root, "scripts/build.py")
            after_code = object_code(root, {new: old for old, new in renames.items()})
            if before_code != after_code:
                changed = sorted(k for k in before_code.keys() | after_code.keys()
                                 if before_code.get(k) != after_code.get(k))
                raise ValueError("Object code or relocation targets changed: " + ", ".join(changed))
            run(root, "scripts/measure.py")
            check_audit(before_audit, load_audit(root))
            run(root, "tests/run_differential_suite.py", "--jobs", str(jobs))
            run(root, "scripts/prepare_fastcmp.py")
        except BaseException:
            # Also remove newly created build objects from a failed compilation.
            for path in paths | generated_paths(root):
                if path in saved:
                    shutil.copy2(backup / path.relative_to(root), path)
                elif path.exists():
                    path.unlink()
            print("Rename failed; restored sources, tests, inventory, build and reports.", file=sys.stderr)
            raise
    print(f"Applied {len(renames)} names across {len(changes)} files; object code identical, matching preserved.")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    export = sub.add_parser("export", help="Export unnamed byte-exact functions as editable TSV")
    export.add_argument("--output", type=Path, help="New file to create; defaults to stdout")
    export.add_argument("--file", action="append", help="Filter by source basename, e.g. Car.cpp")
    export.add_argument("--include-named", action="store_true")
    show = sub.add_parser("show", help="Read a function body and all source references")
    show.add_argument("address", type=lambda a: int(a, 16))
    preview = sub.add_parser("preview", help="Validate the map and print a diff without editing")
    preview.add_argument("map", type=Path)
    apply = sub.add_parser("apply", help="Apply a map, validate and restore on failure")
    apply.add_argument("map", type=Path)
    apply.add_argument("--jobs", type=int, default=3, help="Differential suite workers")
    args = parser.parse_args(argv)
    try:
        if args.command == "export":
            export_map(ROOT, args.output, args.file, args.include_named)
        elif args.command == "show":
            show_function(ROOT, args.address)
        else:
            require_fresh(ROOT)
            changes, renames = make_plan(ROOT, read_map(args.map))
            if args.command == "preview":
                for path, (old, new) in sorted(changes.items()):
                    relative = str(path.relative_to(ROOT))
                    sys.stdout.writelines(difflib.unified_diff(
                        old.decode("utf-8", "surrogateescape").splitlines(keepends=True),
                        new.decode("utf-8", "surrogateescape").splitlines(keepends=True),
                        fromfile="a/" + relative, tofile="b/" + relative))
                print(f"\n{len(renames)} renames, {len(changes)} files; no files changed.")
            else:
                if args.jobs < 1:
                    raise ValueError("--jobs must be positive")
                apply_plan(ROOT, changes, renames, args.jobs)
    except (ValueError, OSError, subprocess.CalledProcessError, KeyError) as error:
        parser.exit(1, f"rename_functions: {error}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
