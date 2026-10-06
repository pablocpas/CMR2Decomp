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

    python3 scripts/match.py --list --shape [--file X.cpp] [--max-size N]  # pick targets (~5 s)
    python3 scripts/match.py 0xADDR                                        # compile + diff, ~1 s
    python3 scripts/match.py --changed                                     # gate before commit
    python3 scripts/helper_hints.py [--file X.cpp]                         # inline-helper mismatches

`match.py` compiles the TU once, byte-compares every function in it against the
baseline and prints new exact matches, score changes, regressions (exit 1) and the
side-by-side diff (original | ours) of the requested functions. The header also
gives the score with registers ignored: 100% there means only register
allocation differs. New or renamed globals resolve through their `// GLOBAL:`
annotation, so no full build is needed to try them.

Automated search (on a snapshot, writes a patch to review and `patch -p1`):

    python3 scripts/permute_batch.py --output /tmp/x --min-score 0 --max-bytes 100000 \
        --limit 1000 --mutation-kinds idiom          # PERMUTE_IDIOMS=a,b to restrict

## What has worked on the remaining functions

- Separate globals instead of one blob addressed through offset macros: MSVC6
  must assume a store into one view may change another and reloads it.
- The original's inline helper rather than equivalent C: `FixMul(a, b) >> 16`
  vs `FixMulShift32`, `FixVecLength`, `FixVecDot`, `FixVecScaleRecip`, and
  `FixMul` operand order. `helper_hints.py` lists where the counts differ.
- `c ? 0xff : 0` for the `setcc/dec/and` pattern, case order and shared tails
  in switches, statement order for stores.
- Editing one function can shift register allocation in later functions of the
  same TU; `match.py` reports every function of the TU for that reason. Any
  floating-point code earlier in a TU changes some later integer functions, and
  even declaring an unused inline function in a shared header moved code in
  other TUs. The original has ~193 C++ objects (Rich header); a function that
  only matches in a different neighbourhood can move to its own TU with
  `scripts/split_tu.py` (OptionMenuRows.cpp).

## Reading MSVC6 output

- Stack slots: ordered by reference count, most-referenced nearest the frame
  base (`ebp`, or `esp` without a frame), ties by first use. Declaration order
  is ignored; variables with disjoint lifetimes can share a slot. A slot
  mismatch means a variable is used a different number of times, or the
  original reuses one variable where we have two (or the reverse).
- Inline-asm helpers (`FixMul`, `FixVecScale`, ...) need their arguments in
  memory: a plain variable is used in place, any other expression gets a
  temporary home (often a dead parameter's slot). `FIX_ABS(FixMul(...))`
  expands the call three times. A helper call whose result is unused is still
  emitted.
- `c ? 0xff : 0` gives `setcc/dec/and`; `x -= k; f(x)` gives `sub` where
  `f(x - k)` gives `add x, -k`; a `return` inside an `if` duplicates the
  epilogue where `if/else` shares it.
- `shl r, 2` once, then `[r + table]` addressing: the index is a `char` or
  `BYTE` local (`BYTE i = (BYTE)player;`, `char car = g_partCar->index;`); an
  `int` index gives `[r*4 + table]` at each use instead. A value loaded once and used by two helper calls is
  a local (`int zoom = table[i];`), not two reads of the table.
- A caller that pushes a narrow local as a raw dword (no `movzx`/`and`) is
  calling a function whose parameter is `BYTE`/`char` too; fix the prototype
  (and the definition, so the mangled names agree).
- A load placed above a loop's own guard (`mov` before `test n; jle`) comes
  from a plain `for (i = 0; i < n; i++) p[i * k] = ...` loop; an explicit
  `if (n > 0)` around a pointer loop keeps the load inside.
- Stores through a pointer cannot move past loads of other globals; stores to
  a global by name can. When the original schedules a store early, write it
  to the global directly (`g_order[i] = a;`, not `p[0] = a;`), and keep
  globals that live in one struct/blob in the repo as separate globals.
- Splitting a blob into separate globals: uninitialised C++ globals are
  communal and the linker scatters them. If code reads across a neighbour
  (`g_netStageBest[stage - 1]`), initialise them (`= { 0 }`) so they stay in
  definition order in `.data`.

## Before committing a batch

    python3 scripts/build.py && python3 scripts/measure.py && python3 scripts/prepare_fastcmp.py

The full measure must not lose any exact function. Commit the updated
`CMR2PROGRESS/` with the source. Commit messages state the function and the score
change, e.g. `Sprite_FillRect byte-exact (24.8% -> exact): ...`.

## Rules

- Keep annotations, struct offsets, declaration order; do not rename while matching.
- Behaviour must stay identical; no inline asm, no `#pragma` tricks to fake bytes.
- A change that makes one function exact but breaks another in the TU is not progress.
