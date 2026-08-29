# ASCENDENDO — Capture Validation Status — 2026-08-29

## Reconciled state

- `PROJECT-STUDIES/ASCENDENDO` currently provides the latest study-layer material dated 2026-08-28/29, but its current-state snapshots are historical relative to the latest upstream commits.
- `main` contains deterministic capture launcher PR #185, merged as `09176c5bb1c96eb7441bc4c4ea048afa8a0d4003`.
- PR #185 was validated on head `3bc8e588de0f140c94cfcfe91faa612ec3dbeb6c` with the three mandatory CI jobs successful.
- The current E2E evidence tranche is PR #186, branch `feat/deterministic-capture-e2e-validation-20260829`.

## Current claim boundary

The launcher and readback implementation now form an executable capture path, but visual correctness has not yet been claimed.

Current evidence distinguishes:

```text
launcher selects requested level
        ≠
PPM file exists
        ≠
PPM is structurally valid
        ≠
image is visually correct
        ≠
golden image equivalence
```

PR #186 is intended to establish only the first three transitions plus a minimal non-uniform-pixel sanity check.

## Next decision

After successful E2E capture evidence, inspect the actual PPM artifacts. Only reproduced geometric/visual discrepancies should justify a viewport/camera change.

The next candidate benchmark remains the deterministic Movement Feel Benchmark described by the study material; it must remain separate from PCG and player-conditioned generation work.
