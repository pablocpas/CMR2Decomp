# Working on CMR2Decomp

Matching decompilation: every `// FUNCTION: CMR2 0x...` must compile (MSVC6 SP3,
`/O2 /Gz /MD /GX`, `/QIfist` per TU) to the original bytes. Progress is measured
by the relocated byte audit (`CMR2PROGRESS/bytes.json`), not the reccmp percentage,
with two size-weighted metrics (`measure.py` prints both, `provenance.json` keeps them):

- **Perfect match**: % of code bytes in byte-exact functions (the headline).
- **Fuzzy match**: size-weighted similarity with registers and branch targets
  ignored (`fz` per function), the one that steers the work: pick functions by
  missing bytes, fix structure, verify behaviour. A function at 100% fuzzy that
  is not exact differs only in register allocation; leave it parked.

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

## Slot tools

- `match.py` prints a third score that also ignores stack offsets: near 100%
  means the code is right and only the frame layout differs.
- `scripts/slots.py 0xADDR` maps each of our frame slots to the original's.
- `scripts/rename_search.py 0xADDR [--merge] [--apply]` tries local/parameter
  renames (slot ties) and, with --merge, merging two locals (check lifetimes).

## Reading MSVC6 output

- Stack slots: ordered by reference count, most-referenced nearest the frame
  base (`ebp`, or `esp` without a frame). Declaration order is ignored, but
  ties depend on the variables' names (renaming `ab, cb, cd, ad` to
  `edge1, edge3, edge4, edge2` fixed Car_UpdateGroundNormal), so when only
  equal-use slots are swapped, try other names; variables with disjoint lifetimes can share a slot. A slot
  mismatch means a variable is used a different number of times, or the
  original reuses one variable where we have two (or the reverse).
- Inline-asm helpers (`FixMul`, `FixVecScale`, ...) need their arguments in
  memory: a plain variable is used in place, any other expression gets a
  temporary home (often a dead parameter's slot). `FIX_ABS(FixMul(...))`
  expands the call three times. A helper call whose result is unused is still
  emitted.
- `&global` as an inline-asm argument is used in place (`mov esi, imm`), but
  `&global[1]` / a member at a non-zero offset gets a home first
  (`mov [ebp-x], imm`). A homed constant address means the vector is part of
  a larger object (Graphics_DrawLayerQuad: g_glowBasis[3]).
- A local that MSVC6 can keep in a dead parameter's slot (depth in `[ebp+0xc]`)
  is how `x = e; x = FixMul(x, k)` shows up: the asm operand is the variable's
  own slot (`mov [ebp-4], eax; mov eax, [ebp-4]`), where `FixMul(e, k)` homes
  `e` in a temp slot. Likewise a value stored in a variable before being
  passed (`d = FixVecDot(..); FixVecScale(.., d)`) is homed in that variable's
  slot. The order of the two homes of `FixVecDot(a, b)` shows which argument
  is which, so swap them when only those two stores differ.
- Corner/vertex blocks: `q2 = q0 - A; q1 = q0 + B; q0 -= B; q3 = q2 - B;
  q2 += B` (the first q2 store is dead and vanishes, its value stays in
  registers) rather than each corner written out from q0
  (Graphics_DrawProjectedQuad, Graphics_DrawLayerQuad).
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
- MSVC6 duplicates a short common tail (`last = now; return x;`) into each
  branch. Decompiled code with the same statements repeated before several
  returns usually matches as one if/else followed by a single tail.
  The reverse also happens: MSVC6 cross-jumps identical ends of two branches,
  so the original may have a call in each branch (different registers before
  the shared tail mean two calls in the source). Two calls of one function
  with different arguments in an if/else are often one call with the
  differing arguments computed into locals first.
- `push ebx/edi` inside a branch instead of in the prologue (or the reverse)
  is MSVC6 placing callee-saved pushes: decompiled `if (!flag) return;`
  before a shared tail often matches as `if (flag) tail;` with no return.
- A `volatile` that forces a reload is usually covering for tail duplication:
  `if (c) { a = f(); } else { a = 0; } b = x + a;` gives the reload of `a` in
  the copied tail (StageObject_UpdateThirdRouteRamp).
- `add r, -k` where `sub r, k` was expected: the subtraction is part of a
  larger expression (`v = (s < 0 ? -s : s) - k;`).
- Decompiled block-scoped temporaries (`int d = ...; int s = ...;`) are
  often one reused variable in the original (`step = FixVecDot(...)`): the
  inline-asm homes then land in that variable's slot. A pointer to a member
  (`pUp = &car->right`) taken just before its first use, with the arithmetic
  before it spelled `car->right.x`, matches where the decompiler hoisted it.
- A decompiled `do { p = pRows; ...; pRows++; } while (--i);` whose
  original keeps the pointer biased (`[ebp+8]` = row + 8, fields at `-4`,
  `-8`) is an indexed loop: `for (i = 0; i < 3; i++) f(&pRows[i]);
  pRows[i].x += ...` (FixBasis_Integrate). Likewise `rec[player * 5 + i]`
  rather than a pointer walk when the induction pointer is at a field.
- A value the original keeps in a register across stores to globals (and
  writes to its global only after the branches) is a local:
  `type = g_a[i]; ... if (type == 0) type = other; g_type = type;`. A load
  reused after stores (`movsx edi, [esi+6]` then `sub eax, edi`) means the
  expression using it came before those stores in the source.
- Calls with constant arguments pushed per branch (`push 4; push edi; jmp
  call` / `push 3; ...`) are separate calls in an if/else; a ternary argument
  becomes `setcc` arithmetic instead. A condition the original tests twice
  (`cmp eax, 1` again after a branch on it) is written twice in the source,
  on a value the compiler cannot prove (`flags[i] == 1 && ...` then
  `else if (flags[i] == 1 && ...)`).
- A tail jump past alignment padding (`jmp X` then `nop`s, X a 16-aligned
  address inside the inventory size) means the original has two functions;
  split it and fix `scripts/functions.tsv` (CGame::UpdateFrontendCallbackMachine).
- A parameter the function keeps updating (a running x position) that the
  original loads into a register at entry, before any other work, is copied
  into a local first (`px = x;`, then only `px` is used): the copy also frees
  the parameter's stack slot, so a small local (`BYTE colour[4]`) lands there
  (Game_DrawFadingBootLabel, FrontendDraw_BreadcrumbItem, RallyData_DrawListItem).
- `T s = {0};` stores the first member and zero-fills the rest from its end;
  dword stores that cover the whole struct from offset 0 are `memset(&s, 0,
  sizeof(s))` (Sound_CreatePcmSampleBuffer).
- A frame 4+ bytes larger than the original, where an array of the original
  shares its slot with a spill temp of an earlier loop, means the array is
  declared in the block that uses it (`if (drawScene) { short rect[4]; ... }`,
  RallyData_DrawLoadingProgress).
- Bytes of a .data string copied into a stack array and the rest zeroed is a
  literal initialiser: `char names[8][10] = { "Finland", ... }`, `char
  gears[] = "RN123456"` (declaration order picks the copy order); not
  `memcpy` from a global or a struct copy.
- An array's size changes where it lands in the frame: `char number[6]`
  matched where `[8]` did not (StageUI_DrawHudRouteMap). Struct copies that
  keep three separate 12-byte slots are an array `FixVector c[3]`; separate
  locals get packed with other variables.
- A value computed before a call's left arguments (Font_DrawText(x, y)
  pushes y first) is a local: `x = MAP_X; y = MAP_Y; f(x, y)`.
- `dec r; je; dec r; jne` dispatch is a `switch` (cases emitted in reverse
  source order); jump-table `switch`es emit their cases in source order, so
  the table gives the original case order (Race_AssignSlotsFromCallRecord).
- `sub cx, si` on a short field is `dst.y = ...; dst.y -= h;`, not `dst.y =
  ... - h` (FrontendMenu_DrawCarSetup).
- A constant kept in memory and reloaded on the else paths (`mov edi,
  [ebp-0x14]`) is one variable that is only lowered inside nested ifs ending in
  a single return (Collision_RayQuad).
- An exit test on a derived value while the index still increments is
  `for (i = 0; i < n; i++)` with `k0 - i * step` in the body (MSVC replaces
  the test).
- A narrow argument computed in 8 bits (`add dl, bl`) where the original
  pushes a full register means the callee takes an int (Race_TeardownStage:
  Replay_SwapPendingSlotValue's flag).
- A choice between two values is usually an `if/else` in the original, not a
  default followed by an overriding `if`: `if (enabled) c = text; else c =
  dim;` closed two "registers only" menu screens. Also try the ternary.
- `cdq; xor; sub` is `abs()`; a hand-written `d < 0 ? b - a : d` where the
  original reuses one register is `FIX_ABS(a - b)`.
- Sibling functions share idioms: Car_UpdateRideHeight matched once its axle
  loads were accumulated like Car_UpdateEngineSpeed's (`x = FixMul(..); x +=
  FixMul(..); x /= 2;`).
- A global array re-read (as a word/dword) after unrelated byte stores is
  read through a pointer in the original (`BYTE *pText = g_colourText;`),
  so MSVC6 assumes the stores may alias it (FrontendDraw_HelpText).
- Runtime-library code (type_info's deleting destructor) is compiled /O1 in
  its own TU (`CRT_O1` in build.py): `pop ecx` instead of `add esp, 4`.

## Missing code

A function far from the original with more original instructions than ours
(`scripts/helper_hints.py`, or comparing instruction counts) may have lost
statements in decompilation: StageObject_TestHeadlightGlowsAgainstCarBox had
dropped `dir.y += 0x6666; FixVecScale(&dir, &dir, 0x60000);`. Read the
original disassembly and restore them; that fixes behaviour as well.

zlib (Zlib*.cpp) is compiled as C (`/TC`, as in the original's Rich header),
except ZlibZutil.cpp, which needs a C++ static member and uses extern "C".

## Before committing a batch

    python3 scripts/build.py && python3 scripts/measure.py && python3 scripts/prepare_fastcmp.py

The full measure must not lose any exact function. Commit the updated
`CMR2PROGRESS/` with the source. Commit messages state the function and the score
change, e.g. `Sprite_FillRect byte-exact (24.8% -> exact): ...`.

## Rules

- Keep annotations, struct offsets, declaration order; do not rename while matching.
- Behaviour must stay identical; no inline asm, no `#pragma` tricks to fake bytes.
- A change that makes one function exact but breaks another in the TU is not progress.
- `port/` is OpenCMR2, the SDL3 port, with its own CMake build; matching work
  does not touch it. `port/game/` is a patched copy of `CMR2Decomp/` synced with
  `port/tools/sync_upstream.py`.
