# Differential tests

Each `differential_*.py` harness loads the original `cmr2bin/CMR2.exe` and the
rebuilt `build/CMR2.exe`, runs the same function(s) in both under
[Unicorn](https://www.unicorn-engine.org/) with identical inputs, and compares
the resulting memory, return values and calls to controlled providers. They
need `unicorn` and `pefile`; some also compile small probes with MSVC6 under Wine.

`logic-targets.json` registers every harness, the functions it covers and the
arguments it takes. Run the whole suite after building and measuring:

```bash
python3 tests/run_differential_suite.py --jobs 3
```

The runner refuses to start if the build or sources changed since the last
measurement, because the symbol map in `CMR2PROGRESS/entities.json` must belong
to the executable under test. A single harness can be run directly, e.g.:

```bash
python3 tests/differential_collision_checkpoint.py CMR2PROGRESS/summary.json CMR2PROGRESS/entities.json
```

A passing harness only establishes the tested cases; it does not replace
playing a full race, championship, replay, save/load or network session.

The `test_*.py` files are unit tests for the matching tools and need no Wine.
