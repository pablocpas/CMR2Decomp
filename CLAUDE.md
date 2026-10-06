# Working on CMR2Decomp

Matching decompilation: every `// FUNCTION: CMR2 0x...` must compile (MSVC6 SP3,
`/O2 /Gz /MD /GX`, `/QIfist` per TU) to the original bytes. The metric is the
relocated byte audit (`CMR2PROGRESS/bytes.json`), not the reccmp percentage.

## Setup (fresh Linux / cloud session)

    scripts/setup_linux.sh

Installs Wine, MSVC6 + SP3 (outside the repo, in `../msvc600`), the original EXE,
reccmp; builds, measures and prepares fastcmp. It should report the same
byte-exact count as `CMR2PROGRESS/provenance.json`. Never put the toolchain inside
the repo: reccmp scans the whole tree as source and the results change.

## Matching loop

    python3 scripts/match.py --list [--file X.cpp] [--max-size N]   # pick targets
    python3 scripts/match.py 0xADDR                                 # ~1 s
    python3 scripts/match.py --changed                              # gate before commit

`match.py` compiles the TU once, byte-compares every function in it against the
baseline and prints new exact matches, score changes, regressions (exit 1) and the
side-by-side diff (original | ours) of the requested functions.

## Before committing a batch

    python3 scripts/build.py && python3 scripts/measure.py && python3 scripts/prepare_fastcmp.py

The full measure must not lose any exact function. Commit the updated
`CMR2PROGRESS/` with the source. Commit messages state the function and the score
change, e.g. `Sprite_FillRect byte-exact (24.8% -> exact): ...`.

## Rules

- Keep annotations, struct offsets, declaration order; do not rename while matching.
- Behaviour must stay identical; no inline asm, no `#pragma` tricks to fake bytes.
- A change that makes one function exact but breaks another in the TU is not progress.
