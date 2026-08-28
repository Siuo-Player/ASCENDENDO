# Process incident — direct documentation writes on main

## Data
2026-08-28

## Finding
During reconciliation around PR #133, documentation updates were accidentally committed directly to `main` before the dedicated documentation branch was established.

## Expected protocol
The repository requires:

```text
main → branch → document → implement → validate → PR → merge
```

## Classification
Process defect. No production-code behavior depended on the accidental documentation commits.

## Corrective action
Subsequent documentation reconciliation is being performed in `docs/reconcile-vulkan-upload-20260828` and submitted through a normal pull request. No history rewrite is being used.

## Follow-up
All future changes in this workstream must start from a dedicated branch before any repository write, including documentation-only changes.
