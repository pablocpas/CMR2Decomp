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
