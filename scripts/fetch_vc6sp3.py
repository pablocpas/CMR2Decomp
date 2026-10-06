#!/usr/bin/env python3
"""Install the Visual C++ 6.0 SP3 compiler passes into an MSVC6 tree.

The original CMR2.exe was built with VC6 SP3: its Rich header records C/C++
objects from build 8447, which is the comp.id written by SP3's C2.DLL. A plain
VC6 install (or a later service pack) generates slightly different code, so
matching builds replace the three compiler passes with the SP3 ones:

  C1.DLL / C1XX.DLL 12.00.8472   C and C++ front ends
  C2.DLL            12.00.8447   code generator (ships renamed as msvcep.dll)

CL.EXE and LINK.EXE stay as they are: the driver does not affect the
generated code, and every VC6 linker from SP3 on is 6.00.8447.

The files come from Microsoft's ten-part SP3 web distribution, preserved in the
August 1999 TechNet disc on archive.org. Extracting them needs 7-Zip (`7z`) or
`cabextract`, because the cabinets use LZX compression.
"""

import argparse
import hashlib
import shutil
import subprocess
import sys
import tempfile
import urllib.request
from pathlib import Path

ARCHIVE = "https://archive.org/download/ms-technet-tnsb9908/TNSB9908.iso/VSTUDIO%2FSP3%2F"
PARTS = {
    "VS6SP3_1.EXE": "00e25cbb652960a5dd6e9370248054ffcf0f275d1c929387442ea92265745b0a",
    "VS6SP3_5.EXE": "dfaad8de57fc29253c3c14269b4a33743df71578aa0066bda1267be9787d58d3",
}
PASSES = {
    "C1.DLL": "d1b25d84da1d3d0d460996ea298488cb183b857e1b90628147983114ac4ab55f",
    "C1XX.DLL": "28c355499c2f8090910624734faff31fe719a69a8aac7dd44b2ce64addf96ee2",
    "C2.DLL": "a0cc45f83fd0ed009aa4f43df5598105a1049ad9675de0b1089be7cce7d3c153",
}


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def extractor():
    """Return run(archive, out, *names): extract everything, or only the named files.

    The exit status is not checked: the fourth cabinet's last folder continues
    into cabinets we do not download, so tools report errors for files we never
    asked for. The SHA-256 checks below decide whether extraction worked."""
    for name in ["7z", "7za", r"C:\Program Files\7-Zip\7z.exe"]:
        found = shutil.which(name) or (Path(name).is_file() and name)
        if found:
            return lambda archive, out, *names: subprocess.run(
                [found, "x", "-y", "-o" + str(out), str(archive), *names] + (["-r"] if names else []),
                stdout=subprocess.DEVNULL)
    found = shutil.which("cabextract")
    if found:
        return lambda archive, out, *names: subprocess.run(
            [found, "-q", "-d", str(out), *sum((["-F", "*" + n] for n in names), []), str(archive)])
    sys.exit("fetch_vc6sp3: install 7-Zip (7z) or cabextract to unpack the SP3 cabinets")


def download(cache):
    cache.mkdir(parents=True, exist_ok=True)
    for name, digest in PARTS.items():
        path = cache / name
        if not path.is_file() or sha256(path) != digest:
            print("downloading", name)
            with urllib.request.urlopen(ARCHIVE + name) as response, open(path, "wb") as out:
                shutil.copyfileobj(response, out)
        if sha256(path) != digest:
            sys.exit(f"fetch_vc6sp3: {name} does not match its published SHA-256")


def find(root, name):
    matches = [p for p in Path(root).rglob("*") if p.name.lower() == name.lower()]
    if not matches:
        sys.exit(f"fetch_vc6sp3: {name} not found after extraction")
    return matches[0]


def unpack(cache):
    """Extract the three passes into cache/bin and return that directory."""
    out = cache / "bin"
    if out.is_dir() and all((out / n).is_file() and sha256(out / n) == d for n, d in PASSES.items()):
        return out
    download(cache)
    run = extractor()
    out.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        # Part 1 carries the code generator, renamed to msvcep.dll.
        run(cache / "VS6SP3_1.EXE", tmp / "p1")
        shutil.copyfile(find(tmp / "p1", "msvcep.dll"), out / "C2.DLL")
        # Part 5 wraps the fourth cabinet of the set, which holds the front ends.
        run(cache / "VS6SP3_5.EXE", tmp / "p5")
        run(find(tmp / "p5", "VS6sp3_4.cab"), tmp / "cab4", "c1.dll", "c1xx.dll")
        shutil.copyfile(find(tmp / "cab4", "c1.dll"), out / "C1.DLL")
        shutil.copyfile(find(tmp / "cab4", "c1xx.dll"), out / "C1XX.DLL")
    for name, digest in PASSES.items():
        if sha256(out / name) != digest:
            sys.exit(f"fetch_vc6sp3: extracted {name} has an unexpected SHA-256")
    return out


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--msvc-root", type=Path, required=True,
                        help="the VC98 directory whose Bin receives the SP3 passes")
    parser.add_argument("--cache", type=Path, default=Path("vc6sp3"),
                        help="where the downloads and extracted passes are kept")
    parser.add_argument("--backup", action="store_true",
                        help="keep the replaced passes in Bin/pre-sp3/")
    args = parser.parse_args()

    source = unpack(args.cache)
    bin_dir = args.msvc_root / "Bin"
    if not (bin_dir / "CL.EXE").is_file():
        sys.exit(f"fetch_vc6sp3: {bin_dir} is not an MSVC6 Bin directory")
    for name in PASSES:
        target = bin_dir / name
        if target.is_file() and sha256(target) == PASSES[name]:
            continue
        if args.backup and target.is_file():
            (bin_dir / "pre-sp3").mkdir(exist_ok=True)
            shutil.copy2(target, bin_dir / "pre-sp3" / name)
        shutil.copyfile(source / name, target)
        print("installed", name)
    print("VC6 SP3 compiler passes in", bin_dir)


if __name__ == "__main__":
    main()
