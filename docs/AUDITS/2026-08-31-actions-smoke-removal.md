# ASCENDENDO — Actions smoke diagnostic removal — 2026-08-31

## Finding

`.github/workflows/actions-smoke.yml` was a manual runner diagnostic. It only reported the GitHub-hosted runner environment and asserted trivial Linux execution.

The live workflow-governance study classifies this workflow as temporary diagnostic and recommends evaluating its removal after the earlier runner-startup incident is resolved.

## Current evidence

- No open ASCENDENDO issue remains associated with the runner-startup diagnostic incident.
- The production validation topology now has dedicated Linux test + ASan/UBSan jobs, an independent Windows gate, and an independent deterministic capture evidence workflow.
- The smoke diagnostic is not a required production check.

## Decision

Remove `actions-smoke.yml` in this isolated CI-cleanup tranche.

No runtime code, test semantics, required check names, Windows workflow, or deterministic capture workflow are changed.

## Acceptance

- `Actions smoke diagnostic` no longer exists.
- Linux normal check remains available.
- Linux ASan/UBSan check remains available under its existing job name.
- Windows remains independent.
- Deterministic capture remains independent.
