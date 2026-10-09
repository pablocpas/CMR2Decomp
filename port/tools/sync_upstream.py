#!/usr/bin/env python3
"""Bring upstream CMR2Decomp changes to the game source into game/.

game/ is a copy of CMR2Decomp/CMR2Decomp that OpenCMR2 edits (the ported
platform functions). UPSTREAM holds the CMR2Decomp commit game/ was last
synced to. This applies the upstream diff since then with a three-way merge,
so upstream fixes land and conflicts show up as normal merge conflicts.

    tools/sync_upstream.py [--remote upstream] [--ref main] [--dry-run]

The remote defaults to "upstream" (add it with
`git remote add upstream <path or URL of CMR2Decomp>`).
"""
import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def git(*args, **kw):
    return subprocess.run(["git", "-C", str(ROOT), *args], check=True, text=True,
                          capture_output=True, **kw).stdout


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--remote", default="upstream")
    ap.add_argument("--ref", default="main")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    old = (ROOT / "UPSTREAM").read_text().strip()
    git("fetch", "--quiet", args.remote, args.ref)
    new = git("rev-parse", "FETCH_HEAD").strip()
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
        result = subprocess.run(["git", "-C", str(ROOT), "apply", "--3way", "--directory=game", "-"],
                                input=diff, text=True)
        if result.returncode != 0:
            print("conflicts: resolve them in game/, then commit", file=sys.stderr)
    if not args.dry_run:
        (ROOT / "UPSTREAM").write_text(new + "\n")
        print(f"UPSTREAM -> {new}; review and commit as 'Sync game source with CMR2Decomp {new[:7]}'")
    return 0


if __name__ == "__main__":
    sys.exit(main())
