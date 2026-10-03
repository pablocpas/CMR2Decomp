# CMR2Decomp

A matching decompilation of Colin McRae Rally 2.0 for PC, built with MSVC6 and
checked against the reference executable with [reccmp](https://github.com/isledecomp/reccmp).
The reference SHA-256 is recorded in `reccmp-project.yml`.

`main` contains the consolidated decompilation and matching reviews. See
[INTEGRATION.md](INTEGRATION.md) for the integration history and validation.

## Build

Initialize the SDK submodules and provide MSVC6 (`msvc600/VC98`).
The Visual Studio solution is useful for editing; matching builds use the
shared Python command below. The same command is used by Windows CI.

Windows:

```bat
python scripts/build.py --msvc-root msvc600/VC98
```

Linux with Wine and an existing compiler prefix:

```bash
export CMR2_MSVC_ROOT=/path/to/msvc600/VC98
export WINEPREFIX=/path/to/compiler-wineprefix
python3 scripts/build.py
```

The build applies `/QIfist` to the identified translation units and `/Ob2` to
zlib, with `/O2 /DNDEBUG /D_CRTIMP= /Zi /Gz /MD /GX`. It removes stale build
artifacts first and records source, compiler, EXE and PDB hashes in
`build/manifest.json`.

`--windowed` enables the recovered Wine/window rendering support. This build
is for execution; measure the default build for fidelity to the original.

## Measure

Provide the original EXE and the local `reccmp-user.yml` and
`reccmp-build.yml` configurations. Install the tested measurement tools:

```bash
python3 -m pip install -r scripts/requirements.txt
python3 scripts/measure.py
```

This verifies that sources and binaries still match the build manifest, checks
annotations and global data, generates `index.html`, and saves reports in
`CMR2PROGRESS/`:

- `summary.json`: reccmp function scores.
- `bytes.json`: all annotated source functions, compared after relocation.
- `entities.json`: the symbol map belonging to this build.
- `nonmatching.tsv`: remaining non-exact functions, lowest matching score first,
  then largest first for equal scores; both comparison scores are recorded.
- `provenance.json`: counts, build inputs, hashes and unresolved symbols.

A 100% reccmp *implemented* score is coverage of its measured functions, not
100% exact code or proof that the complete game works. The original inventory
in `scripts/functions.tsv` also includes library code and analysis artifacts.
Unresolved operands remain non-exact in the byte audit.
Exactness includes embedded switch tables. Trailing COFF alignment is removed
before determining how many original bytes to read, so neighbouring functions
are not mistaken for part of a small rebuilt body.

## Batch source search

`scripts/permute_batch.py` searches small non-exact functions, highest score
first, using several functions and compiler workers concurrently. It requires
the external `../tools/fastcmp/permute.py` mutator, or `--mutator PATH`, and the
compiler environment used above. Build and measure first, then run:

```bash
python3 scripts/permute_batch.py --output /tmp/cmr2-search --limit 140
python3 scripts/permute_batch.py --output /tmp/cmr2-search --limit 140 --resume
python3 scripts/permute_batch.py --output /tmp/cmr2-packed --packed --min-score 0 --max-score .6 --max-bytes 20000 --limit 900 --rounds 5
```

The output contains an isolated source/build snapshot, cached candidates,
`search-results.json` and `changes.patch`. The command keeps strictly improving
variants in the snapshot; review the patch before applying it, then rebuild
and measure. The main sources are never modified by the search. Resume checks
the build, reports, compiler options and driver identity and reuses successful
compilations. Failed compilations are retried.

Defaults search functions at 88% or better and up to 1000 original bytes, with
four functions sharing twelve compiler workers. After that band is exhausted,
use a new output directory and `--min-score .6 --max-score .88 --max-bytes 800`
to examine the next band. Use `--min-bytes 1001 --max-bytes 10000` to examine
larger functions without repeating that size band. `--jobs`, `--functions` and `--rounds` control the
search cost. Boundary and mutation regression checks run without Wine:

```bash
python3 tests/test_permute_batch.py
```

Wrapped, unbraced `if/else` calls are treated as whole statements. To search
only that family across the pending functions, use a fresh output directory
with `--mutation-kinds ifswap-multiline --min-score 0 --max-bytes 20000 --limit 900`.
The negated condition preserves its evaluation and floating-point comparisons.
For a small stalled selection, `--neutral-rounds 1` also explores alternative
representations that initially keep the same score; the default stops when
there is no strict improvement. This increases the search cost.

`--packed` tests one candidate for each selected function in a translation unit
together, so one compiler invocation can evaluate several function variants.
The object is parsed once and each function is compared separately, including
relocations and switch tables. Winning bodies are checked again together before
the patch is written. Integrate a whole reviewed batch, then run build, measure
and the differential suite once for that batch; check all previously exact
functions and global data, since neighbouring inlining can change other bodies.
Packed searches require a new output directory and do not support `--resume`
or neutral rounds. `--mutation-kinds local-layout` additionally searches plain
uninitialized scalar, pointer and `FixVector` declarations at function entry;
it leaves initializers, constructors and nested scopes in place.

## Differential validation

The native harnesses execute original and recompiled machine code with the
same inputs and compare memory, guards and relevant calls. On Linux:

```bash
export CMR2_TOOLS=/path/to/directory-containing-msvc600
python3 tests/differential_collision_checkpoint.py CMR2PROGRESS/summary.json CMR2PROGRESS/entities.json
```

The harnesses honor `WINEPREFIX`. Keep the EXE, PDB, report and symbol map from
the same build. A successful function harness does not replace testing a full
race, championship, replay, save/load or network session.

Run the complete registered suite and regenerate the logic inventory after
building and measuring, using the same compiler/Wine environment:

```bash
python3 tests/run_differential_suite.py --jobs 3
python3 scripts/audit_logic.py
```

The runner verifies build identity and records each harness's output and tested
entry points in `CMR2PROGRESS/logic-tests.json`. Controlled providers do not count
as tested functions. The auditor checks current evidence, inventories every
annotated source function, and compares reachable calls, branches and return
cleanup for non-exact bodies. Indirect dispatch is marked as partial. Static
differences guide review and do not establish behavioral defects.

See [the logic review](CMR2PROGRESS/logic-review.md),
[all source functions](CMR2PROGRESS/logic-all.tsv),
[remaining matching functions](CMR2PROGRESS/logic-pending.tsv), and
[original entries outside annotated source](CMR2PROGRESS/logic-outside-source.tsv).
Local byte equivalence does not validate non-exact callees; passing fixtures
only establish the tested cases. The original inventory also contains linked
libraries, jump entries and analysis artifacts requiring separate classification.
