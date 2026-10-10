#!/usr/bin/env python3
"""Bring CMR2Decomp changes to the game source into port/game/.

port/game/ is a copy of CMR2Decomp/ (in the same repository) that the port
edits (the ported platform functions). port/UPSTREAM holds the commit
port/game/ was last synced to. This applies the diff of CMR2Decomp/ since then
with a three-way merge, so decomp fixes land and conflicts show up as normal
merge conflicts.

    port/tools/sync_upstream.py [--ref HEAD] [--dry-run]
"""
import argparse
import subprocess
import sys
from pathlib import Path

PORT = Path(__file__).resolve().parent.parent
REPO = PORT.parent


def git(*args, **kw):
    return subprocess.run(["git", "-C", str(REPO), *args], check=True, text=True,
                          capture_output=True, **kw).stdout


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--ref", default="HEAD")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    old = (PORT / "UPSTREAM").read_text().strip()
    new = git("rev-parse", args.ref).strip()
    if new == old:
        print("game/ is up to date with", new[:10])
        return 0

    log = git("log", "--oneline", f"{old}..{new}", "--", "CMR2Decomp")
    diff = git("diff", "--binary", "--relative=CMR2Decomp", old, new, "--", "CMR2Decomp")
    print(f"{len(log.splitlines())} upstream commits touch the game source ({old[:10]}..{new[:10]})")
    if not diff:
        print("no changes under CMR2Decomp/")
    elif args.dry_run:
        print(git("diff", "--stat", "--relative=CMR2Decomp", old, new, "--", "CMR2Decomp"))
        return 0
    else:
        result = subprocess.run(["git", "-C", str(REPO), "apply", "--3way", "--directory=port/game", "-"],
                                input=diff, text=True)
        if result.returncode != 0:
            print("conflicts: resolve them in port/game/, then commit", file=sys.stderr)
    if not args.dry_run:
        (PORT / "UPSTREAM").write_text(new + "\n")
        print(f"UPSTREAM -> {new}; review and commit as 'Sync game source with CMR2Decomp {new[:7]}'")
    return 0


if __name__ == "__main__":
    sys.exit(main())
