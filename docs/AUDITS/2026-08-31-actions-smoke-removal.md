# ASCENDENDO — Actions smoke diagnostic removal — 2026-08-31

## Finding

`.github/workflows/actions-smoke.yml` is a manual runner diagnostic only. It is not a production gate and no open issue remains for the startup incident that motivated it.

## Decision

Remove the obsolete workflow in this isolated CI-cleanup tranche. Keep Linux normal and ASan/UBSan in `tests.yml`, Windows independent, and deterministic capture independent.

## Evidence

The live workflow-governance study explicitly classifies `actions-smoke.yml` as temporary diagnostic and recommends its removal after the underlying incident is resolved.

## Acceptance

Linux normal, ASan/UBSan, Windows and deterministic capture must remain available after removal.
