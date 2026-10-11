# Evidence-backed migration batches

scripts/migrate_64bit.py automates reference collection, repeated source
transformations, matching gates and isolation of failed families. A reviewed
recipe supplies the data classification and evidence. The tool cannot infer
whether an arbitrary integer is an address, numeric value, mixed slot or file
offset, nor assign a semantic name without evidence.

## Prepare and review

Collect the declarations, producers and consumers of candidate spellings:

```sh
python3 scripts/migrate_64bit.py --discover CallbackStateMachine StageUI_GetRaceResultTable g_unk0x00537f0c --output /tmp/cmr2-references
```

references.json contains lexical rg matches. Related fields/types must be
grouped using those references and the producer/consumer evidence. Supply
each family as one coherent migration: pointer declarations, prototypes,
members, arrays/sizeof/offsetof and original layout checks together.

```json
{
  "families": [
    {
      "id": "example-pointer-flow",
      "classification": "pointer",
      "evidence": "Describe the producer, actual pointed-to record and consumers.",
      "symbols": ["example"],
      "rules": [
        {
          "paths": ["CMR2Decomp/Example*.cpp"],
          "find": "old reviewed spelling",
          "replace": "new reviewed spelling",
          "count": 3
        }
      ]
    }
  ]
}
```

Accepted classifications are pointer, number, mixed, file_offset and layout.
Rules use literal substitution by default; regex:true enables multiline
regular expressions and replacement backreferences. count is the exact total
number of matches across the selected files. A new header uses a rule with
create, content and count:1. A family may declare requires:["earlier-family"];
dependencies must appear before consumers.

```sh
python3 scripts/migrate_64bit.py --recipe /tmp/reviewed-recipe.json --output /tmp/cmr2-migration
```

This generates migration.patch and plan.json without changing sources. The
plan freezes selected files, SHA-256 input hashes, per-file match counts and
the reviewed transformations. Empty/ambiguous selections fail immediately.
Paths outside the workspace and .git are rejected. Every changed declaration
and use must still be accounted for in review; lexical discovery does not
prove that a recipe contains every semantic consumer.

## Apply large batches and isolate failures

```sh
WINEPREFIX=/home/pablo/colin_mcrae_linux/tools/wineprefix python3 scripts/migrate_64bit.py --plan /tmp/cmr2-migration/plan.json --apply --output /tmp/cmr2-migration-run
```

The default gate is scripts/match.py --changed --no-diff. It compares every
function of the affected translation units against the existing scores and
returns failure for lost exactness, lower raw/fuzzy scores or compile errors.
The tool first gates the unchanged baseline and checks frozen input hashes,
then tries all families together. On failure it recursively divides independent
groups, trying each against the already accepted changes. It retains passing
groups and parks failing atomic groups. A requires connection makes its entire
dependency group indivisible; a record and its consumers cannot be split into
invalid intermediate layouts merely to locate a failure.

Independent families may edit the same file: isolation reapplies their frozen
rules rather than substituting whole-file snapshots. Overlapping rules that
need each other must declare that dependency. report.json records accepted
families, parked groups, every probe's exit status and log path. Rejected probe
changes are removed, including new headers. Unexpected failures roll back
owned changes; detected concurrent edits are preserved and stop the run.

--gate followed by an argument vector selects a custom gate; no shell is used.
It must be read-only with respect to CMR2PROGRESS/bytes.json. Rewriting the
baseline during a probe is rejected and the saved metrics are restored. The
gate's comparison and exit status decide acceptance; unchanged metadata alone
cannot establish that the newly compiled functions match.

## Final validation and stable commit

Add --finalize to --apply for the full build, measurement, metadata preparation,
matching and differential suite with three workers. Measurements are compared
against the saved s/fz/x baseline for every previously tracked function,
including missing functions. A failure leaves accepted source changes available
for review, with stable:false and the failing step recorded. Successful final
validation marks stable:true and updates CMR2PROGRESS.

Before the local stable commit, update the family documentation and inventories,
inspect the final diff and include the refreshed progress files. The generator
does not commit or push. Focused native validation and any new behavioral
harness needed for a recovered access must also be supplied by the recipe's
author; the generic matching gate does not generate those semantic models.

## Reproducible example and tests

scripts/migrations/race-state-pointers.json records the complete reviewed
callback-state migration from 2e37a49. Its 57 transformations reproduce fifteen
production source/header files byte for byte, including repeated declarations,
the owned storage, mixed pointer-width table and layout checks. This is one
atomic family because its member/prototype changes depend on one another.
Replay it in an isolated checkout of that baseline; applying it to the already
migrated source correctly fails its input/new-file checks.

Ten tests in tests/test_migrate_64bit.py exercise large-batch failure isolation,
independent edits sharing a file, dependency groups, stale plans, invalid loaded
dependencies, baseline failure, preservation of concurrent edits, every score
dimension/missing functions, baseline-metadata restoration, exact match counts/path boundaries and removal of
a parked new header. Their gates are synthetic and verify orchestration; the
actual callback-state batch was separately built, measured and behavior-tested.
The real recipe replay verifies source reproduction, not an additional automatic
MSVC6 bisection experiment. The test log accompanies this document.
